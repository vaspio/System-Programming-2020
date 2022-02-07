#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include "struct.h"
#include "list.h"
#include "avltree.h"
#include "hashtable.h"
#include "queries.h"
#include "extrafunctions.h"

#define NumOfEntries 17

int sigflag=0;

void SigHandler(int s){
    if(s == SIGINT || s == SIGQUIT) sigflag = 3;

    else if(s == SIGUSR2) sigflag = 1;  //for queries

    else if(s == SIGUSR1) sigflag = 2;
}


int main(int argc, char *argv[]){
    int worker,numWorkers,buffer,i,j,l;
    char dir[20];

    sscanf(argv[1],"%d %d %d %s",&worker, &numWorkers, &buffer, dir);

    signal(SIGUSR2,SigHandler);
    signal(SIGINT,SigHandler);
    signal(SIGQUIT,SigHandler);
    signal(SIGUSR1,SigHandler);
    sigset_t temps;
    (void) sigemptyset(&temps);

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
    // printf("Worker %d there are:%d\n",worker,totaljobs );

    char* tok;
    char* jobsArray[totaljobs];

    tok = strtok(jobs, " ");
    jobsArray[0] = strdup(tok);
    for(i=1; i<totaljobs ;i++){
        tok = strtok(NULL, " ");
        jobsArray[i] = strdup(tok);
    }

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

                                SummaryStatistics(countryHashTable[i]->bucketnodeArray[j]->Btree, count, diseaseArray, writefd, buffer);
                            }
                        }
                    }
                }
                countryHashTable[i] = countryHashTable[i]->next;

            }while(countryHashTable[i] != NULL);

            countryHashTable[i] = temp1;
        }
    }


    if(write(writefd ,"6#END",6) == -1){    // end of SummaryStatistics
        printf("Problem writing in END\n");
    }


    while(1){
        sigsuspend(&temps);

        if(sigflag == 1){   //queries
            sigflag = 0;

            char command[50],mess[50];
            char* input = NULL;
            input = ReadMess(readfd,buffer);

            while(strcmp(input,"/exit") != 0){
                tok = strtok(input, " ");
                strcpy(command,tok);

                if(strcmp(command,"/listCountries") == 0){
                    ListCountries(countryHashTable,NumOfEntries);

                }
                else if(strcmp(command,"/diseaseFrequency") == 0){
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
                    if(strcmp(tok,"-") == 0){   //no country
                        stats = FrequencyStats(diseaseHashTable, date1, date2, NumOfEntries, virus, "-");
                    }
                    else stats = FrequencyStats(diseaseHashTable, date1, date2, NumOfEntries, virus, tok);

                    memset(mess, 0, sizeof(mess));
                    snprintf(mess,sizeof(mess),"%d",stats);
                    WriteMess(mess, writefd, buffer);
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
                    tok = strtok(NULL, " ");
                    strcpy(country,tok);
                    tok = strtok(NULL, " ");
                    strcpy(virus,tok);
                    tok = strtok(NULL, " ");
                    strcpy(date1,tok);
                    tok = strtok(NULL, " ");
                    strcpy(date2,tok);

                    int success = TopkAgeRanges(diseaseHashTable,date1,date2,NumOfEntries,virus,country,top);

                    memset(mess, 0, sizeof(mess));
                    snprintf(mess,sizeof(mess),"%d",success);
                    WriteMess(mess, writefd, buffer);

                }
                else if(strcmp(command,"/searchPatientRecord") == 0){
                    char id[15];
                    memset(id, 0, sizeof(id));

                    tok = strtok(NULL, " ");
                    strcpy(id,tok);

                    char* find = NULL;
                    find = FindinHashtable(recordHashTable,NumOfEntries,id);

                    WriteMess(find, writefd, buffer);
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
                    strcpy(country,tok);

                    PatientAdmissionsCountry(diseaseHashTable,date1,date2,NumOfEntries,virus,country,totaljobs,jobsArray);
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
                    strcpy(country,tok);

                    PatientDischarges(diseaseHashTable,date1,date2,NumOfEntries,virus,country,totaljobs,jobsArray);
                }
                else if(strcmp(command,"/intquitWorker") == 0){
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
                    // printf("telos worker %d\n",worker );

                    exit(1);
                }

                free(input);
                input = NULL;
                input = ReadMess(readfd,buffer);
            }
            free(input);

        }

        if(sigflag == 2){   //new records

            memset(buf, 0, sizeof(buf));
            memset(cntry, 0, sizeof(cntry));
            count = 0;
            n = 0;

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

                    if(num_of_files[n] == count){   //nothing changed
                        n++;
                        continue;
                    }
                    n++;
                    // printf("Something new\n" );

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


                    int index = hash(NumOfEntries,cntry);

                    int bucketSize = countryHashTable[0]->bytes;
                    int size = bucketSize/sizeof(bucketnode);
                    bucket* temp = countryHashTable[index];
                    while(temp != NULL){

                        for(i=0; i<size ;i++){
                            if(strcmp(cntry,temp->bucketnodeArray[i]->name)==0){
                                int exists;
                                for(j=0; j<count ;j++){
                                    exists = 0;
                                    date curdate = SetDate(datesArray[j]);
                                    DateExistsinTree(temp->bucketnodeArray[i]->Btree,&exists,curdate);  //see if exists in system


                                    if(!exists){     // found new file
                                        printf("%s\n", datesArray[j]);
                                        char str[200];
                                        char file[100];
                                        memset(str, 0, sizeof(str));
                                        memset(file, 0, sizeof(file));

                                        snprintf(file,sizeof(file),"%s/%s/%s",dir,cntry,datesArray[j]);
                                        FILE* record = fopen(file,"r");
                                        if(!record){
                                            fprintf(stderr, "Error opening file '%s'\n", datesArray[j]);
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
                                                    patientList = AddListnode(patientList,id,firstname,lastname,disease,cntry,age,datesArray[j],"-");
                                                    suc++;

                                                    InsertinHash(diseaseHashTable,patientList->patient,patientList->patient->diseaseID,NumOfEntries);
                                                    InsertinHash(countryHashTable,patientList->patient,patientList->patient->country,NumOfEntries);
                                                    InsertinHash(recordHashTable,patientList->patient,patientList->patient->recordID,NumOfEntries);
                                                }
                                                else fail++;
                                            }
                                            else{
                                                if(CheckValidDate(patientList,id,datesArray[j])){
                                                    fail++;
                                                    // printf("ERROR\n" );
                                                }
                                                else{
                                                    list keep = patientList;

                                                    list trav = patientList;
                                                    while(trav != NULL){
                                                		if(strcmp(trav->patient->recordID, id) == 0){
                                                            trav->patient->exitDate = SetDate(datesArray[j]);

                                                            break;
                                                        }
                                                		trav = trav->next;
                                                	}

                                                    patientList = keep;
                                                }
                                            }

                                        }
                                        fclose(record);

                                        strcpy(diseaseArray[0],"-");
                                        MakeDiseaseArray(temp->bucketnodeArray[i]->Btree,count,diseaseArray);


                                        SummaryForFile(temp->bucketnodeArray[i]->Btree, count, diseaseArray, curdate);
                                    }
                                }
                            }
                        }
                        temp = temp->next;
                    }

                    closedir(countryFiles);
                }
            }
            closedir(database);

        }

        if(sigflag == 3){   // SIGINT SIGQUIT
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

            break;
        }
    }

    // printf("telos worker %d\n",worker );

    return 0;
}
