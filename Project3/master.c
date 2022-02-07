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

int main(int argc, char *argv[]){
    int i,j,numWorkers,buffer,fd,port;
    char dir[20];
    char ip[100];

    if(argc == 11){
        for(i=1; i<argc ;i++){
    		if(strcmp(argv[i],"-w") == 0) numWorkers = atoi(argv[i+1]);
            if(strcmp(argv[i],"-b") == 0) buffer = atoi(argv[i+1]);
            if(strcmp(argv[i],"-p") == 0) port = atoi(argv[i+1]);

    		if(strcmp(argv[i],"-s") == 0) strcpy(ip, argv[i+1]);
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

    DIR* database = opendir(dir);   //open database
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

    char iport[200];
    snprintf(iport, sizeof(iport),"%s %d",ip,port);

    for(i=0; i<numWorkers ;i++){
        WriteMess(filesperworker[i], writefd[i], buffer);   //send the jobs of every worker through the pipes
        WriteMess(iport, writefd[i], buffer);
    }

    signal(SIGINT,SigHandler);  //initialize
    signal(SIGQUIT,SigHandler);

    while(1){
        if(sigflag == 3) break;
    }

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

    printf("\nMaster exited \n");
    return 0;
}
