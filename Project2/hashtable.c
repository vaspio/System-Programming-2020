#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "hashtable.h"

int hash(int size, char* K){  //hash function taken from k08
    int h=0,a=127;
    for (; *K!='\0'; K++)
        h = (a*h + *K) % size;
    return h;
}

bucket* CreateBucket(int bucketSize){
    bucket* newBucket = malloc(sizeof(bucket));

    newBucket->bytes = bucketSize;
    newBucket->next = NULL;

    int size = bucketSize/sizeof(bucketnode);
    newBucket->bucketnodeArray = malloc(size*sizeof(bucketnode*));

    for(int i=0; i<size ;i++){
        newBucket->bucketnodeArray[i] = malloc(sizeof(bucketnode));
        newBucket->bucketnodeArray[i]->name = strdup("-");
    }
    return newBucket;
}

hashtable* CreateHashtable(int numEntries, int bucketSize){
    hashtable* hashtable = malloc(numEntries*sizeof(hashtable));

    int i;
    for(i=0; i<numEntries ;i++){
        hashtable[i] = CreateBucket(bucketSize);
    }
    return hashtable;
}

bool InsertinBucket(hashtable* hashtable, patientRecord* patient, int index, char* string){
    int size,i;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);
    for(i=0; i<size ;i++){
        if(strcmp(hashtable[index]->bucketnodeArray[i]->name,"-") == 0){  //empty slot
            free(hashtable[index]->bucketnodeArray[i]->name);

            hashtable[index]->bucketnodeArray[i]->name = strdup(string);
            hashtable[index]->bucketnodeArray[i]->Btree = AvlInsert(NULL,patient);
            return 1;
        }
        else if(strcmp(hashtable[index]->bucketnodeArray[i]->name,string) == 0){
            hashtable[index]->bucketnodeArray[i]->Btree = AvlInsert(hashtable[index]->bucketnodeArray[i]->Btree, patient);
            return 1;
        }
    }
    return 0;
}

void InsertinHash(hashtable* hashtable, patientRecord* patient, char* str, int numEntries){
    int index;
    bool flag;

    index = hash(numEntries,str);
    flag = InsertinBucket(hashtable, patient, index, str);

    bucket* temp = hashtable[index];
    while(!flag){
        if(hashtable[index]->next == NULL){ // need new bucket
            hashtable[index]->next = CreateBucket(hashtable[0]->bytes);
        }
        hashtable[index] = hashtable[index]->next;
        flag = InsertinBucket(hashtable, patient, index, str);

    }
    hashtable[index] = temp;    //reset pointer

}

bool FindDuplicates(hashtable* hashtable,int numEntries,char* str){
    int i,size,count=0;

    int index = hash(numEntries,str);

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp = hashtable[index];
    while(temp != NULL){

        for(i=0; i<size ;i++){
            if(strcmp(str,temp->bucketnodeArray[i]->name)==0){
                count++;
                if(count==2) {
                    printf("%s\n", temp->bucketnodeArray[i]->name);
                    return 1;
                }
            }
        }
        temp = temp->next;
    }
    return 0;
}

bool CheckError(hashtable* hashtable,int numEntries){
    int i,j,size;
    bool error=0;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name,"-")!=0){
                    error = FindDuplicates(hashtable,numEntries,hashtable[i]->bucketnodeArray[j]->name);
                    if(error) return 1;
                }
            }

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }
    return 0;
}

void PrintHashtable(hashtable* hashtable,int numEntries){
    int i,j,size;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        printf("%d %d ",i ,size);
        do{
            for(j=0; j<size ;j++){
                printf("%s ",hashtable[i]->bucketnodeArray[j]->name);
            }
            printf("\t");

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
        printf("\n");
    }
    printf("\n");
}

void FreeHashTable(hashtable* hashtable,int numEntries){
    int i,j,size;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* trav;
    for(i=0; i<numEntries ;i++){
        do{
            for(j=0; j<size ;j++){
                if(hashtable[i]->bucketnodeArray[j] != NULL){
                    if(strcmp(hashtable[i]->bucketnodeArray[j]->name,"-") != 0){    //for uninitialized values
                        FreeAvl(hashtable[i]->bucketnodeArray[j]->Btree);
                    }
                    free(hashtable[i]->bucketnodeArray[j]->name);
                    free(hashtable[i]->bucketnodeArray[j]);
                }
            }
            free(hashtable[i]->bucketnodeArray);

            trav = hashtable[i];
            hashtable[i] = hashtable[i]->next;
            free(trav);
        }while(hashtable[i] != NULL);

    }
    free(hashtable);

}
