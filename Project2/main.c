#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#include "extrafunctions.h"

int sigflag=0;
void SigHandler(int s){
    if(s == SIGCHLD) sigflag = 1;

    else if(s == SIGINT || s == SIGQUIT) sigflag = 3;
}

void SignalSend(int s, pid_t* table, int num){
    for(int i=0; i<num ;i++){
        kill(table[i],s);
    }
}


int main(int argc, char *argv[]){
    int i,j,numWorkers,buffer,fd;
    char dir[20];

    if(argc == 7){
        for(i=1; i<argc ;i++){
    		if(strcmp(argv[i],"-w") == 0) numWorkers = atoi(argv[i+1]);
            if(strcmp(argv[i],"-b") == 0) buffer = atoi(argv[i+1]);

    		if(strcmp(argv[i],"-i") == 0) strcpy(dir, argv[i+1]);
        }
    }
    else{
        printf("Wrong input\n");
        exit(EXIT_FAILURE);
    }

    int readfd[numWorkers];
    int writefd[numWorkers];
    int status;
    char fifo1[50],fifo2[50];
    pid_t pidArr[numWorkers];

    for(i=0; i<numWorkers ;i++){
        snprintf(fifo1 , sizeof(fifo1),"fifo%d.1",i+1);
        snprintf(fifo2 , sizeof(fifo2),"fifo%d.2",i+1);

        if(mkfifo(fifo1, 0666) < 0) perror("can't create fifo");
        if(mkfifo(fifo2, 0666) < 0){
            unlink(fifo1);
            perror("can't create fifo");
        }

        pidArr[i] = fork();  //create child process and save pid
        if(pidArr[i] == 0){

            char str[100];
            sprintf(str, "%d %d %d %s", i, numWorkers, buffer, dir);
            char *args[]={"./workers",str,NULL};
            execv(args[0],args);

            exit(0);
        }
        else{
            if((writefd[i] = open(fifo2, O_WRONLY)) < 0){
                perror("server: can't open write fifo");
            }

            if((readfd[i] = open(fifo1, O_RDONLY)) < 0){
                perror("server: can't open read fifo");
            }
        }
    }

    DIR* database = opendir(argv[6]);   //open database
    if(database == NULL){
        perror("Opening directory\n");
        exit(1);
    }

    struct dirent* entry;
    int numdirs=0,pos=0;
    char* filesperworker[numWorkers];
    for(i=0; i<numWorkers ;i++){
        filesperworker[i] = malloc(200*sizeof(char));
        strcpy(filesperworker[i]," ");  //initialize
    }

    char temp[20];
    while((entry = readdir(database)) != NULL){

        if(strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0){
            strcpy(temp, entry->d_name);

            if(strcmp(filesperworker[pos]," ") == 0){
                strcpy(filesperworker[pos], entry->d_name);
            }
            else{
                strcat(filesperworker[pos], " ");
                strcat(filesperworker[pos], entry->d_name);
            }

            pos++;
            if(pos==numWorkers) pos=0;

            numdirs++;  //count dirs
        }
    }

    for(i=0; i<numWorkers ;i++){
        WriteMess(filesperworker[i], writefd[i], buffer);   //send the jobs of every worker through the pipes
    }

    bool ex=0;
    int times=0;

    int end[numWorkers];
    memset(end, 0, sizeof(end));
    while(!ex){
        fd_set fds;
        int maxfd;
        char* buf = NULL;

        FD_ZERO(&fds);  // clear fd_set

        for(i=0; i<numWorkers ;i++){
            FD_SET(readfd[i], &fds);
        }

        maxfd = readfd[0];
        for(i=1; i<numWorkers ;i++){
            if(end[i] == 1) continue;

            if(readfd[i] > maxfd){
                maxfd = readfd[i];
            }
        }

        select(maxfd + 1, &fds, NULL, NULL, NULL);

        for(i=0; i<numWorkers ;i++){
            if(end[i] == 1) continue;

            if(FD_ISSET(readfd[i], &fds)){

                buf = ReadMess(readfd[i],buffer);
                while(strcmp(buf,"END") != 0){
                    free(buf);
                    buf = ReadMess(readfd[i],buffer);

                    if(strcmp(buf,"END") != 0) printf("%s\n",buf );
                }
                free(buf);

                end[i] = 1;

                times++;
                if(times == numWorkers) ex=1;

            }
        }
    }

    // sleep(10);
    // SignalSend(SIGUSR1, pidArr, numWorkers);

    SignalSend(SIGUSR2, pidArr, numWorkers);    //ready for queries

    signal(SIGINT,SigHandler);  //initialize
    signal(SIGQUIT,SigHandler);

    int error_count=0,success_count=0;

    char* command = NULL;
    size_t characters, bufsize=100;
    char* tok;
    char input[50],send[516];
    int worker = 0;
    bool quit=0;
    bool kill=0;

    printf("\nEnter commands: ");
    characters = getline(&command,&bufsize,stdin);
    if(strcmp(command,"\n")!=0)command[strlen(command)-1] = '\0';

    while(strcmp(command,"/exit") != 0){
        if(sigflag == 3){
            kill=1;
            printf("Sending SIGKILL to children\n" );
            break;
        }
        if(strcmp(command,"\n")==0) break;

        tok = strtok(command, " ");
        strcpy(input,tok);

        if(strcmp(tok, "/listCountries") == 0){
            for(i=0; i<numWorkers ;i++){
                memset(send, 0, sizeof(send));
                snprintf(send,sizeof(send),"%s",input);
                WriteMess(send, writefd[i], buffer);
            }
            success_count++;
        }
        else if(strcmp(tok, "/topk-AgeRanges") == 0){
            char country[15],virus[30],date1[15],date2[15];
            int top;

            memset(country, 0, sizeof(country));
            memset(virus, 0, sizeof(virus));
            memset(date1, 0, sizeof(date1));
            memset(date2, 0, sizeof(date2));

            tok = strtok(NULL, " ");
            top = atoi(tok);
            tok = strtok(NULL, " ");
            strcpy(country,tok);
            tok = strtok(NULL, " ");
            strcpy(virus,tok);
            tok = strtok(NULL, " ");
            strcpy(date1,tok);
            tok = strtok(NULL, " ");
            strcpy(date2,tok);

            if(top <= 4){

                bool success=0;
                for(i=0; i<numWorkers ;i++){
                    memset(send, 0, sizeof(send));
                    snprintf(send,sizeof(send),"%s %d %s %s %s %s",input,top,country,virus,date1,date2);
                    WriteMess(send, writefd[i], buffer);

                    char* rd = NULL;
                    rd = ReadMess(readfd[i],buffer);

                    if(strcmp(rd,"1")==0) success = 1;

                    free(rd);
                }
                if(!success) printf("No cases of %s found in %s\n",virus,country);

                success_count++;
            }
            else{
                error_count++;
                printf("Not valid k: %d is too big\n",top );
            }

        }
        else if(strcmp(tok, "/diseaseFrequency") == 0){
            int stats=0;
            char virus[30],date1[15],date2[15];

            memset(virus, 0, sizeof(virus));
            memset(date1, 0, sizeof(date1));
            memset(date2, 0, sizeof(date2));

            tok = strtok(NULL, " ");
            strcpy(virus,tok);
            tok = strtok(NULL, " ");
            strcpy(date1,tok);
            tok = strtok(NULL, " ");
            strcpy(date2,tok);

            tok = strtok(NULL, " ");
            if(tok == NULL){
                for(i=0; i<numWorkers ;i++){
                    memset(send, 0, sizeof(send));
                    snprintf(send,sizeof(send),"%s %s %s %s -",input,virus,date1,date2);
                    WriteMess(send, writefd[i], buffer);

                    char* rd = NULL;
                    rd = ReadMess(readfd[i],buffer);

                    if(atoi(rd) > 0) stats += atoi(rd);

                    free(rd);
                }
            }
            else{
                for(i=0; i<numWorkers ;i++){
                    memset(send, 0, sizeof(send));
                    snprintf(send,sizeof(send),"%s %s %s %s %s",input,virus,date1,date2,tok);
                    WriteMess(send, writefd[i], buffer);

                    char* rd = NULL;
                    rd = ReadMess(readfd[i],buffer);

                    if(atoi(rd) > 0) stats += atoi(rd);

                    free(rd);
                }
            }
            if(stats == 0) printf("Not found\n");
            else printf("%d\n",stats );

            success_count++;
        }
        else if(strcmp(tok, "/searchPatientRecord") == 0){
            char id[15];
            memset(id, 0, sizeof(id));

            tok = strtok(NULL, " ");
            strcpy(id,tok);

            bool in = 0;
            for(i=0; i<numWorkers ;i++){
                memset(send, 0, sizeof(send));
                snprintf(send,sizeof(send),"%s %s",input,id);

                WriteMess(send, writefd[i], buffer);

                char* res = NULL;
                res = ReadMess(readfd[i],buffer);

                if(strcmp(res,"-") != 0){
                    in = 1;
                    printf("%s\n",res );
                }
                free(res);
            }
            if(!in) printf("Record with id %s doesn't exist\n",id);

            success_count++;
        }
        else if(strcmp(tok, "/numPatientAdmissions") == 0){
            char country[15],virus[30],date1[15],date2[15];

            memset(country, 0, sizeof(country));
            memset(virus, 0, sizeof(virus));
            memset(date1, 0, sizeof(date1));
            memset(date2, 0, sizeof(date2));

            tok = strtok(NULL, " ");
            strcpy(virus,tok);
            tok = strtok(NULL, " ");
            strcpy(date1,tok);
            tok = strtok(NULL, " ");
            strcpy(date2,tok);
            tok = strtok(NULL, " ");
            if(tok != NULL){
                strcpy(country,tok);
                for(i=0; i<numWorkers ;i++){
                    memset(send, 0, sizeof(send));
                    snprintf(send,sizeof(send),"%s %s %s %s %s",input,virus,date1,date2,country);

                    WriteMess(send, writefd[i], buffer);
                }
            }
            else{
                for(i=0; i<numWorkers ;i++){
                    memset(send, 0, sizeof(send));
                    snprintf(send,sizeof(send),"%s %s %s %s -",input,virus,date1,date2);

                    WriteMess(send, writefd[i], buffer);
                }
            }
            success_count++;
        }
        else if(strcmp(tok, "/numPatientDischarges") == 0){
            char country[15],virus[30],date1[15],date2[15];

            memset(country, 0, sizeof(country));
            memset(virus, 0, sizeof(virus));
            memset(date1, 0, sizeof(date1));
            memset(date2, 0, sizeof(date2));

            tok = strtok(NULL, " ");
            strcpy(virus,tok);
            tok = strtok(NULL, " ");
            strcpy(date1,tok);
            tok = strtok(NULL, " ");
            strcpy(date2,tok);
            tok = strtok(NULL, " ");
            if(tok != NULL){
                strcpy(country,tok);
                for(i=0; i<numWorkers ;i++){
                    memset(send, 0, sizeof(send));
                    snprintf(send,sizeof(send),"%s %s %s %s %s",input,virus,date1,date2,country);

                    WriteMess(send, writefd[i], buffer);
                }
            }
            else{
                for(i=0; i<numWorkers ;i++){
                    memset(send, 0, sizeof(send));
                    snprintf(send,sizeof(send),"%s %s %s %s -",input,virus,date1,date2);

                    WriteMess(send, writefd[i], buffer);
                }
            }
            success_count++;

        }
        else if(strcmp(tok, "/intquitWorker") == 0){
            int numw;

            tok = strtok(NULL, " ");
            if(tok!=NULL){
                numw = atoi(tok);
                if(numw <= numWorkers){
                    signal(SIGCHLD,SigHandler);

                    if(numw == numWorkers){  // quit all if if input=number of workers
                        for(i=0; i<numWorkers ;i++){
                            memset(send, 0, sizeof(send));
                            snprintf(send,sizeof(send),"%s",input);

                            WriteMess(send, writefd[i], buffer);
                        }
                        printf("All children exited\n" );
                        quit=1;
                        break;
                    }
                    else{
                        for(i=0; i<numWorkers ;i++){
                            if(numw == i){
                                memset(send, 0, sizeof(send));
                                snprintf(send,sizeof(send),"%s",input);

                                WriteMess(send, writefd[i], buffer);
                            }
                        }
                        printf("Worker %d exited. Creating new\n",numw );

                        pid_t pros;
                        pros = wait(NULL);
                        if(sigflag == 1){
                            snprintf(fifo1 , sizeof(fifo1),"fifo%d.1",numw+1);
                            snprintf(fifo2 , sizeof(fifo2),"fifo%d.2",numw+1);

                            unlink(fifo1);
                            unlink(fifo2);

                            if(mkfifo(fifo1, 0666) < 0) perror("can't create fifo");
                            if(mkfifo(fifo2, 0666) < 0){
                                unlink(fifo1);
                                perror("can't create fifo");
                            }

                            pidArr[numw] = fork();  //create child process and save pid
                            if(pidArr[numw] == 0){
                                char str[100];
                                sprintf(str, "%d %d %d %s", numw, numWorkers, buffer, dir);
                                char *args[]={"./workers",str,NULL};
                                execv(args[0],args);

                                exit(0);
                            }
                            else{
                                if((writefd[numw] = open(fifo2, O_WRONLY)) < 0){
                                    perror("server: can't open write fifo");
                                }

                                if((readfd[numw] = open(fifo1, O_RDONLY)) < 0){
                                    perror("server: can't open read fifo");
                                }
                            }
                            WriteMess(filesperworker[numw], writefd[numw], buffer);

                            char* buff;
                            buff = ReadMess(readfd[numw],buffer);
                            while(strcmp(buff,"END") != 0){ // print new child statistics
                                if(strcmp(buff,"END") != 0) printf("%s\n",buff );

                                free(buff);
                                buff = ReadMess(readfd[numw],buffer);
                            }
                            free(buff);

                        }

                    }

                }
                else{
                    error_count++;
                    printf("There are not so many workers\n" );
                }
            }
            else{
                error_count++;
                printf("Give worker or number of workers \n" );
            }
        }
        else{
            printf("Wrong command\n" );
            error_count++;
        }

        // printf("Enter command: ");

        printf("\n" );
        characters = getline(&command,&bufsize,stdin);
        if(strcmp(command,"\n")!=0)command[strlen(command)-1] = '\0';

    }

    for(i=0; i<numWorkers ;i++){
        memset(send, 0, sizeof(send));
        strcpy(send,"/exit");
        WriteMess(send, writefd[i], buffer);
    }


    // SignalSend(SIGQUIT, pidArr, numWorkers);
    if(!quit) SignalSend(SIGKILL, pidArr, numWorkers);

    int pid = getpid();

    char log[100];
    snprintf(log,sizeof(log),"log_file.%d",pid);

    FILE* logfile = fopen(log, "w");

    char tmpstr[100];
    for(i=0; i<numWorkers ;i++){    //countries that participated

        tok = strtok(filesperworker[i], " ");
        memset(tmpstr, 0, sizeof(tmpstr));
        strcpy(tmpstr,tok);
        while(tok != NULL){
            fprintf(logfile, "%s\n", tmpstr);

            tok = strtok(NULL, " ");
            if(tok != NULL){
                strcpy(tmpstr,tok);
            }
        }

    }
    char total[200];
    snprintf(total,sizeof(total),"TOTAL %d\nSUCCESS %d\nFAIL %d",success_count+error_count,success_count,error_count);
    fprintf(logfile,"%s\n",total);

    fclose(logfile);
    free(command);
    closedir(database);

    for(i=0; i<numWorkers ;i++){
        free(filesperworker[i]);

        snprintf(fifo1 , sizeof(fifo1),"fifo%d.1",i+1);
        snprintf(fifo2 , sizeof(fifo2),"fifo%d.2",i+1);
        unlink(fifo1);
        unlink(fifo2);

        close(readfd[i]);
        close(writefd[i]);
    }

    while(waitpid(-1, &status, 0) > 0);

    printf("Aggregator exited \n");
    return 0;
}
