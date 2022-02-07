#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>

#include "struct.h"
#include "list.h"
#include "avltree.h"
#include "hashtable.h"
#include "queries.h"
#include "extrafunctions.h"

#define NumOfEntries 17

int main(int argc, char *argv[]){
    int worker,numWorkers,buffer,i,j,l;
    char dir[20];

    sscanf(argv[1],"%d %d %d %s",&worker, &numWorkers, &buffer, dir);

    int writefd,readfd;
    char fifo1[50],fifo2[50];

    snprintf(fifo1 , sizeof(fifo1),"fifo%d.1",worker+1);
    snprintf(fifo2 , sizeof(fifo2),"fifo%d.2",worker+1);

    if((readfd = open(fifo2, O_RDONLY)) < 0) perror("worker: can't open read fifo");

    if((writefd = open(fifo1, O_WRONLY)) < 0) perror("worker: can't open write fifo");

    char* jobs;
    jobs = ReadMess(readfd,buffer);   //read the workers jobs

    // printf("Worker %d has this jobs: %s|\n",worker,jobs );

    int totaljobs = 1;
    for(i=0; jobs[i]!='\0' ;i++){
		if(jobs[i] == ' ' || jobs[i] == '\n'){
			totaljobs++;
		}
	}

    char* tok;
    char* jobsArray[totaljobs];

    tok = strtok(jobs, " ");
    jobsArray[0] = strdup(tok);
    for(i=1; i<totaljobs ;i++){
        tok = strtok(NULL, " ");
        jobsArray[i] = strdup(tok);
    }

    char* iport;
    iport = ReadMess(readfd,buffer);    // read ip and port

    char ip[100];
    int port;
    sscanf(iport,"%s %d",ip,&port);

    free(iport);

    struct dirent* entry;
    char buf[40];
    char cntry[20];
    int count,suc=0,fail=0;
    bool flag;
    list patientList = NULL;

    int num_of_files[totaljobs];
    int n=0;

    DIR* database = opendir(dir);
    while((entry = readdir(database)) != NULL){
        flag = 0;   // to adjust the jobs

        if(strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0){
            strcpy(cntry,entry->d_name);
            snprintf(buf,sizeof(buf),"%s/%s",dir,cntry);

            for(i=0; i<totaljobs ;i++){
                if(strcmp(jobsArray[i],cntry) == 0){
                    flag = 1;
                    break;
                }
            }
            if(!flag) continue;  //job for other worker

            DIR* countryFiles = opendir(buf);
            struct dirent* dr;

            count = 0;
            while((dr = readdir(countryFiles)) != NULL) count++; //count files
            count -= 2; // . .. dirs

            num_of_files[n] = count;
            n++;

            rewinddir(countryFiles);

            char datesArray[count][50];
            i=0;
            while((dr = readdir(countryFiles)) != NULL){
                if(strcmp(dr->d_name,".") != 0 && strcmp(dr->d_name,"..") != 0){
                    strcpy(datesArray[i],dr->d_name);
                    i++;
                }
            }
            SelectionSort(datesArray,count);    //sort the dates

            char str[200];
            char file[100];

            for(i=0; i<count ;i++){

                snprintf(file,sizeof(file),"%s/%s/%s",dir,cntry,datesArray[i]);
                FILE* record = fopen(file,"r");
                if(!record){
                    fprintf(stderr, "Error opening file '%s'\n", datesArray[i]);
                    return EXIT_FAILURE;
                }

                char id[20],enxit[20],firstname[20],lastname[20],disease[20],date[20];
                int age;
                while(fgets(str, 100, record) != NULL){
                    tok = strtok(str, " ");
            	    strcpy(id,tok);
            	    tok = strtok(NULL, " ");
            	    strcpy(enxit,tok);
            	    tok = strtok(NULL, " ");
            	    strcpy(firstname,tok);
            	    tok = strtok(NULL, " ");
            	    strcpy(lastname,tok);
            	    tok = strtok(NULL, " ");
            	    strcpy(disease,tok);
            	    tok = strtok(NULL, " ");
            	    age = atoi(tok);

                    if(strcmp(enxit, "ENTER") == 0){
                        if(!CheckIdExists(patientList,id)){
                            patientList = AddListnode(patientList,id,firstname,lastname,disease,cntry,age,datesArray[i],"-");
                            suc++;
                        }
                        else fail++;
                    }
                    else{
                        if(CheckValidDate(patientList,id,datesArray[i])){
                            fail++;
                            // printf("ERROR\n" );
                        }
                        else{
                            list keep = patientList;

                            list trav = patientList;
                            while(trav != NULL){
                        		if(strcmp(trav->patient->recordID, id) == 0){
                                    trav->patient->exitDate = SetDate(datesArray[i]);

                                    break;
                                }
                        		trav = trav->next;
                        	}

                            patientList = keep;
                        }
                    }

                }
                fclose(record);
            }
            closedir(countryFiles);
        }
    }
    closedir(database);

    int bucketSize = 80;
    hashtable* diseaseHashTable = CreateHashtable(NumOfEntries,bucketSize);
    hashtable* countryHashTable = CreateHashtable(NumOfEntries,bucketSize);
    hashtable* recordHashTable = CreateHashtable(NumOfEntries,bucketSize);

    list temp = patientList;
    while(temp != NULL){
        InsertinHash(diseaseHashTable,temp->patient,temp->patient->diseaseID,NumOfEntries);
        InsertinHash(countryHashTable,temp->patient,temp->patient->country,NumOfEntries);
        InsertinHash(recordHashTable,temp->patient,temp->patient->recordID,NumOfEntries);

        temp = temp->next;    //traverse patient list
    }
    // PrintHashtable(diseaseHashTable, NumOfEntries);


    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;

    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket");

    struct hostent *rem;
    if((rem = gethostbyname(ip)) == NULL) {
	   herror("gethostbyname"); exit(1);
    }
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if(connect(sock, serverptr, sizeof(server)) < 0) perror_exit("connect");
    printf("Connecting to port %d\n", port);

    WriteMess("stats",sock,buffer);

    char diseaseArray[count*10][50];
    strcpy(diseaseArray[0],"-");

    int ageArr[totaljobs][4];   //age statistics
    for(i=0; i<NumOfEntries ;i++){
        int size = bucketSize/sizeof(bucketnode);

        bucket* temp1;
        for(i=0; i<NumOfEntries ;i++){
            temp1 = countryHashTable[i];
            do{
                for(j=0; j<size ;j++){
                    if(strcmp(countryHashTable[i]->bucketnodeArray[j]->name,"-") != 0){

                        flag = 0;
                        for(int k=0; k<totaljobs ;k++){
                            if(strcmp(countryHashTable[i]->bucketnodeArray[j]->Btree->patient->country, jobsArray[k]) == 0){

                                strcpy(diseaseArray[0],"-");
                                MakeDiseaseArray(countryHashTable[i]->bucketnodeArray[j]->Btree,count,diseaseArray);

                                SummaryStatistics(countryHashTable[i]->bucketnodeArray[j]->Btree, count, diseaseArray, sock, buffer);
                            }
                        }
                    }
                }
                countryHashTable[i] = countryHashTable[i]->next;

            }while(countryHashTable[i] != NULL);

            countryHashTable[i] = temp1;
        }
    }


    if(write(sock ,"3#END",6) == -1){    // end of SummaryStatistics
        printf("Problem writing in END\n");
    }


    int worksock,cursock;
    socklen_t socklen;
    struct sockaddr_in worker_server, client;

    struct sockaddr* server_ptr = (struct sockaddr*)&worker_server;

    if((worksock = socket(PF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket");
    worker_server.sin_family = AF_INET;
    worker_server.sin_addr.s_addr = htonl(INADDR_ANY);
    worker_server.sin_port = htons(0);

    socklen = sizeof(server_ptr);


    int opt = 1;
    if(setsockopt(worksock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if(bind(worksock, server_ptr, sizeof(worker_server)) < 0) perror_exit("bind");

    if(listen(worksock, 5) < 0) perror_exit("listen");


    if(getsockname(worksock, (struct sockaddr *)&worker_server, &socklen) == -1) perror_exit("getsockname");
    printf("Ready for queries to port %d\n", ntohs(worker_server.sin_port)  );

    char portmess[100];
    memset(portmess, 0, sizeof(portmess));
    snprintf(portmess,sizeof(portmess),"%d",ntohs(worker_server.sin_port));
    WriteMess(portmess, sock, buffer);


    if((cursock = accept(worksock, server_ptr, &socklen)) < 0) perror_exit("accept");
    // printf("Accepted connection for queries\n");


    char command[50],mess[100];
    char* input = NULL;
    char* send;
    while(1){

        input = ReadMess(cursock,buffer);

        tok = strtok(input, " ");
        strcpy(command,tok);

        if(strcmp(command,"/diseaseFrequency") == 0){
            char virus[30],date1[15],date2[15];
            int stats;

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
            if(tok == NULL){   //no country
                stats = FrequencyStats(diseaseHashTable, date1, date2, NumOfEntries, virus, "-");
            }
            else stats = FrequencyStats(diseaseHashTable, date1, date2, NumOfEntries, virus, tok);

            // printf("stats %d\n",stats );
            memset(mess, 0, sizeof(mess));
            snprintf(mess,sizeof(mess),"%d",stats);
            WriteMess(mess, cursock, buffer);
        }
        else if(strcmp(command,"/topk-AgeRanges") == 0){
            char country[15],virus[30],date1[15],date2[15];
            int top;

            memset(country, 0, sizeof(country));
            memset(virus, 0, sizeof(virus));
            memset(date1, 0, sizeof(date1));
            memset(date2, 0, sizeof(date2));

            tok = strtok(NULL, " ");
            top = atoi(tok);

            if(top <= 4){
                tok = strtok(NULL, " ");
                strcpy(country,tok);
                tok = strtok(NULL, " ");
                strcpy(virus,tok);
                tok = strtok(NULL, " ");
                strcpy(date1,tok);
                tok = strtok(NULL, " ");
                strcpy(date2,tok);

                send = TopkAgeRanges(diseaseHashTable,date1,date2,NumOfEntries,virus,country,top);

                WriteMess(send, cursock, buffer);
                free(send);
            }
            else{
                WriteMess("Not valid k", cursock, buffer);
            }

        }
        else if(strcmp(command,"/searchPatientRecord") == 0){
            char id[15];
            memset(id, 0, sizeof(id));

            tok = strtok(NULL, " ");
            strcpy(id,tok);

            char* find = NULL;
            find = FindinHashtable(recordHashTable,NumOfEntries,id);

            WriteMess(find, cursock, buffer);
            free(find);
        }
        else if(strcmp(command,"/numPatientAdmissions") == 0){
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
            if(tok == NULL){   //no country
                send = PatientAdmissionsCountry(diseaseHashTable,date1,date2,NumOfEntries,virus,"-",totaljobs,jobsArray);
            }
            else send = PatientAdmissionsCountry(diseaseHashTable,date1,date2,NumOfEntries,virus,tok,totaljobs,jobsArray);

            WriteMess(send, cursock, buffer);

            free(send);
        }
        else if(strcmp(command,"/numPatientDischarges") == 0){
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
            if(tok == NULL){   //no country
                send = PatientDischarges(diseaseHashTable,date1,date2,NumOfEntries,virus,"-",totaljobs,jobsArray);
            }
            else send = PatientDischarges(diseaseHashTable,date1,date2,NumOfEntries,virus,tok,totaljobs,jobsArray);

            WriteMess(send, cursock, buffer);

            free(send);
        }
        else if(strcmp(command,"/exit") == 0){
            int pid = getpid();

            char log[100];
            snprintf(log,sizeof(log),"log_file.%d",pid);

            FILE* logfile = fopen(log, "w");

            for(i=0; i<totaljobs ;i++){
                fprintf(logfile, "%s\n", jobsArray[i]);

                free(jobsArray[i]);
            }

            char total[200];
            snprintf(total,sizeof(total),"TOTAL %d\nSUCCESS %d\nFAIL %d",suc+fail,suc,fail);
            fprintf(logfile,"%s\n",total);

            free(jobs);
            fclose(logfile);
            FreeHashTable(diseaseHashTable,NumOfEntries);
            FreeHashTable(countryHashTable,NumOfEntries);
            FreeHashTable(recordHashTable,NumOfEntries);
            FreeList(patientList);
            free(input);
            close(cursock);
            printf("Worker %d exited\n",worker+1 );

            exit(0);
        }

        free(input);


    }
    close(cursock);

    return 0;
}
