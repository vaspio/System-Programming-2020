#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "list.h"
#include "struct.h"
#include "avltree.h"
#include "hashtable.h"
#include "queries.h"
#include "topk.h"
#define HashIdNum 13 //prime

int main(int argc, char *argv[]){
    int i,disHashNumOfEntries,countHashNumOfEntries,bucketSize;
    char file[32];

    if(argc == 9){
        for(i=1; i<argc ;i++){
    		if(strcmp(argv[i],"-p") == 0) strcpy(file, argv[i+1]);

    		if(strcmp(argv[i],"-h1") == 0) disHashNumOfEntries = atoi(argv[i+1]);
    		if(strcmp(argv[i],"-h2") == 0) countHashNumOfEntries = atoi(argv[i+1]);

    		if(strcmp(argv[i],"-b") == 0) bucketSize = atoi(argv[i+1]);
        }
    }
    else{
        printf("Wrong input\n");
        exit(EXIT_FAILURE);
    }
    FILE* filename = fopen(file,"r");
    list patientList = InitializeList(filename);

    if(CheckErrorIds(patientList)) exit(EXIT_FAILURE);

    hashtable* diseaseHashTable = CreateHashtable(disHashNumOfEntries,bucketSize);
    hashtable* countryHashTable = CreateHashtable(countHashNumOfEntries,bucketSize);
    hashtable* recordHashTable = CreateHashtable(HashIdNum,bucketSize);

    list temp = patientList;
    while(temp != NULL){
        InsertinHash(diseaseHashTable,temp->patient,temp->patient->diseaseID,disHashNumOfEntries);
        InsertinHash(countryHashTable,temp->patient,temp->patient->country,countHashNumOfEntries);
        InsertinHash(recordHashTable,temp->patient,temp->patient->recordID,HashIdNum);

        temp = temp->next;    //traverse patient list
    }
    // PrintHashtable(diseaseHashTable, disHashNumOfEntries);
    // PrintHashtable(countryHashTable, countHashNumOfEntries);

    char* buffer = NULL;
    size_t characters, bufsize=100;
    char* tok;

    // printf("Enter command: ");
    characters = getline(&buffer,&bufsize,stdin);
    buffer[strlen(buffer)-1] = '\0';

    while(strcmp(buffer,"/exit") != 0){
        tok = strtok(buffer, " ");

        if(strcmp(tok, "/globalDiseaseStats") == 0){
            tok = strtok(NULL, " ");
            if(tok != NULL){
                char date1[15],date2[15];

                strcpy(date1,tok);
                tok = strtok(NULL, " ");
                strcpy(date2,tok);
                if(tok == NULL) printf("Error: Wrong dates given (/globalDiseaseStats)\n");
                else{
                    DiseaseStats(diseaseHashTable, date1, date2, disHashNumOfEntries);
                }
            }
            else{   //without dates
                DiseaseStats(diseaseHashTable,"-","-",disHashNumOfEntries);
            }
        }
        else if(strcmp(tok, "/diseaseFrequency") == 0){
            char virus[30],date1[15],date2[15];

            tok = strtok(NULL, " ");
            strcpy(virus,tok);
            tok = strtok(NULL, " ");
            strcpy(date1,tok);
            tok = strtok(NULL, " ");
            strcpy(date2,tok);

            tok = strtok(NULL, " ");
            if(tok == NULL) FrequencyStats(diseaseHashTable, date1, date2, disHashNumOfEntries, virus, "-");
            else FrequencyStats(diseaseHashTable, date1, date2, disHashNumOfEntries, virus, tok);

        }
        else if(strcmp(tok, "/topk-Diseases") == 0){
            char country[15];
            int top;

            tok = strtok(NULL, " ");
            top = atoi(tok);
            tok = strtok(NULL, " ");
            strcpy(country,tok);

            tok = strtok(NULL, " ");
            if(tok != NULL){
                char date1[15];
                strcpy(date1,tok);
                tok = strtok(NULL, " ");
                if(tok != NULL){
                    TopkDiseases(diseaseHashTable,disHashNumOfEntries,top,country,date1,tok);
                }
                else{
                    printf("Error with given dates\n" );
                }
            }
            else{
                TopkDiseases(diseaseHashTable,disHashNumOfEntries,top,country,"-","-");
            }

        }
        else if(strcmp(tok, "/topk-Countries") == 0){
            char virus[15];
            int top;

            tok = strtok(NULL, " ");
            top = atoi(tok);
            tok = strtok(NULL, " ");
            strcpy(virus,tok);

            tok = strtok(NULL, " ");
            if(tok != NULL){
                char date1[15];
                strcpy(date1,tok);
                tok = strtok(NULL, " ");
                if(tok != NULL){
                    TopkCountries(countryHashTable,countHashNumOfEntries,top,virus,date1,tok);
                }
                else{
                    printf("Error with given dates\n" );
                }
            }
            else{
                TopkCountries(countryHashTable,countHashNumOfEntries,top,virus,"-","-");
            }
        }
        else if(strcmp(tok, "/insertPatientRecord") == 0){
            char date1[15],date2[15];
            patientList = CreateListnode(patientList);

            tok = strtok(NULL, " ");
            patientList->patient->recordID = strdup(tok);
            tok = strtok(NULL, " ");
            patientList->patient->patientFirstName = strdup(tok);
            tok = strtok(NULL, " ");
            patientList->patient->patientLastName = strdup(tok);
            tok = strtok(NULL, " ");
            patientList->patient->diseaseID = strdup(tok);
            tok = strtok(NULL, " ");
            patientList->patient->country = strdup(tok);
            tok = strtok(NULL, " ");
            strcpy(date1,tok);
            patientList->patient->entryDate = SetDate(date1);

            tok = strtok(NULL, " ");
            if(tok == NULL) patientList->patient->exitDate.day = -1;
            else{
                strcpy(date2,tok);
                patientList->patient->exitDate = SetDate(date2);
            }

            InsertinHash(diseaseHashTable,patientList->patient,patientList->patient->diseaseID,disHashNumOfEntries);
            InsertinHash(countryHashTable,patientList->patient,patientList->patient->country,countHashNumOfEntries);
            // printf("Inserted successfully\n");
            printf("Record added");
        }
        else if(strcmp(tok, "/recordPatientExit") == 0){
            char recordID[15],exitDate[15];

            tok = strtok(NULL, " ");
            strcpy(recordID,tok);
            tok = strtok(NULL, " ");
            strcpy(exitDate,tok);

            InsertExitDate(recordHashTable,recordID,exitDate,HashIdNum);
            printf("Record updated" );
        }
        else if(strcmp(tok, "/numCurrentPatients") == 0){
            tok = strtok(NULL, " ");
            if(tok != NULL){
                CurrentPatients(diseaseHashTable, tok, disHashNumOfEntries);
            }
            else CurrentPatients(diseaseHashTable, "-", disHashNumOfEntries);
        }
        else printf("Wrong command\n" );

        break;  //for validator!

        // printf("Enter command: ");
        printf("\n" );

        characters = getline(&buffer,&bufsize,stdin);
        buffer[strlen(buffer)-1] = '\0';
    }
    printf("exiting\n" );

    free(buffer);
    FreeHashTable(diseaseHashTable,disHashNumOfEntries);
    FreeHashTable(countryHashTable,countHashNumOfEntries);
    FreeHashTable(recordHashTable, HashIdNum);
    FreeList(patientList);

    fclose(filename);
    return 0;
}
