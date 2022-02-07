#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "avltree.h"
#include "hashtable.h"

void CountPatients(avlnode* root, int* count, date date1, date date2){   //inorder

    if(root != NULL){
        CountPatients(root->left,count,date1,date2);

        if(date1.day == -1 || (CompareDates(date1, root->patient->entryDate) && CompareDates(root->patient->entryDate, date2))){  // [date1,date2]
            *count=*count+1;

            avlnode* temp = root;
            while(temp->next != NULL){
                *count=*count+1;
                temp = temp->next;
            }
        }

        CountPatients(root->right,count,date1,date2);
    }
}

void CountPatientsCountry(avlnode* root, int* count, date date1, date date2, char* country){   //inorder

    if(root != NULL){

        CountPatientsCountry(root->left,count,date1,date2,country);

        if(strcmp(root->patient->country,country) == 0){
            if(date1.day == -1 || (CompareDates(date1, root->patient->entryDate) && CompareDates(root->patient->entryDate, date2))){  // [date1,date2]
                *count=*count+1;
                // printf("rec:%s\n", root->patient->recordID);
                avlnode* temp = root->next;
                while(temp != NULL){
                    // printf("next \n" );
                    if(strcmp(temp->patient->country,country) == 0){
                        // printf("rec:%s\n", temp->patient->recordID);
                        *count=*count+1;
                    }
                    temp = temp->next;
                }
            }
        }
        CountPatientsCountry(root->right,count,date1,date2,country);
    }
}

void DiseaseStats(hashtable* hashtable, char* str1, char* str2 ,int numEntries){
    int i,j,size,count;
    // int sum;

    date date1,date2;
    if(strcmp(str1,"-") != 0){
        // printf("Stats occured between %s %s\n",str1,str2 );
        date1 = SetDate(str1);
        date2 = SetDate(str2);
    }
    else date1.day = -1;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                count = 0;
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name,"-") != 0){
                    CountPatients(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2);
                    // printf("%s case(s): %d\n",hashtable[i]->bucketnodeArray[j]->name,count);
                    printf("%s %d \n",hashtable[i]->bucketnodeArray[j]->name,count);
                }
                // sum+=count;
            }

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }
    // printf("sum:%d\n",sum );
}

void FrequencyStats(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country){
    int i,j,size,count=0;
    bool flag=0;

    date date1,date2;
    if(strcmp(str1,"-") != 0){
        printf("Stats occured between %s %s\n",str1,str2 );
        date1 = SetDate(str1);
        date2 = SetDate(str2);
    }
    else date1.day = -1;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name, virus) == 0){
                    if(strcmp(country,"-") == 0){
                        CountPatients(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2);
                        printf("No country was given. Finding cases for %s\n",virus);
                        // printf("%s case(s): %d\n",virus,count );
                        printf("%s %d\n",virus,count );
                    }
                    else{
                        CountPatientsCountry(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2, country);
                        printf("Finding cases for %s in %s\n",virus,country);
                        // printf("%s case(s): %d\n",virus,count );
                        printf("%s %d\n",virus,count );
                        PrintAvl(hashtable[i]->bucketnodeArray[j]->Btree);
                    }

                    flag = 1;
                    break;
                }
            }
            if(flag) break;

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;

        if(flag) break;
    }
}

void InsertExitDate(hashtable* hashtable,char* recordId,char* exitDate,int numEntries){
    int i,index,size;

    index = hash(numEntries,recordId);

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp = hashtable[index];
    while(temp != NULL){

        for(i=0; i<size ;i++){
            if(strcmp(recordId,temp->bucketnodeArray[i]->name)==0){
                if(temp->bucketnodeArray[i]->Btree->patient->exitDate.day == -1){
                    temp->bucketnodeArray[i]->Btree->patient->exitDate = SetDate(exitDate);

                    if(CompareDates(temp->bucketnodeArray[i]->Btree->patient->entryDate,temp->bucketnodeArray[i]->Btree->patient->exitDate)){
                        // printf("Inserted successfully\n" );
                    }
                    else{
                        temp->bucketnodeArray[i]->Btree->patient->exitDate.day = -1;
                        // printf("Failed. Exit date doesn't match with the entry date \n" );
                    }
                }
                else{
                    printf("Exit date already exists\n" );
                }
            }
        }
        temp = temp->next;
    }
}

void CountCurrentPatients(avlnode* root, int* count){   //inorder

    if(root != NULL){
        CountCurrentPatients(root->left,count);

        if(root->patient->exitDate.day == -1){
            *count=*count+1;

            avlnode* temp = root->next;
            while(temp != NULL){
                if(temp->patient->exitDate.day == -1) *count=*count+1;

                temp = temp->next;
            }
        }

        CountCurrentPatients(root->right,count);
    }
}

void CurrentPatients(hashtable* hashtable, char* virus, int numEntries){
    int i,j,size,count;
    bool flag=0;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name, "-") != 0){
                    if(strcmp(virus,"-") != 0){
                        if(strcmp(hashtable[i]->bucketnodeArray[j]->name,virus) == 0){
                            CountCurrentPatients(hashtable[i]->bucketnodeArray[j]->Btree, &count);
                            printf("Current %s case(s): %d\n",hashtable[i]->bucketnodeArray[j]->name,count);
                            flag = 1;
                            break;
                        }
                    }
                    else{
                        CountCurrentPatients(hashtable[i]->bucketnodeArray[j]->Btree, &count);
                        printf("Current %s case(s): %d\n",hashtable[i]->bucketnodeArray[j]->name,count);
                        count = 0;
                    }

                }
            }
            if(flag) break;

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;

        if(flag) break;
    }
}
