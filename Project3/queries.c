#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

#include "struct.h"
#include "avltree.h"
#include "hashtable.h"
#include "queries.h"

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

                avlnode* temp = root->next;
                while(temp != NULL){
                    if(strcmp(temp->patient->country,country) == 0){
                        *count=*count+1;
                    }
                    temp = temp->next;
                }
            }
        }
        CountPatientsCountry(root->right,count,date1,date2,country);
    }
}

int FrequencyStats(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country){
    int i,j,size,count=0;

    date date1,date2;
    if(strcmp(str1,"-") != 0){
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
                    }
                    else{
                        CountPatientsCountry(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2, country);
                    }

                    return count;

                }
            }
            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }
    return -1;
}

void CountDiseaseWithAge(avlnode* root,char* country,char* disease,date date1,date date2,int* age1,int* age2,int* age3,int* age4){

    if(root != NULL){
        CountDiseaseWithAge(root->left,country,disease,date1,date2,age1,age2,age3,age4);

        if(strcmp(root->patient->country,country) == 0){

            avlnode* temp = root;
            while(temp != NULL){
                if(strcmp(temp->patient->country,country) == 0){
                    if(temp->patient->age <= 20) *age1=*age1+1;

                    else if(temp->patient->age <= 40) *age2=*age2+1;
                    else if(temp->patient->age <= 60) *age3=*age3+1;
                    else *age4=*age4+1;
                }
                temp = temp->next;
            }

        }
        CountDiseaseWithAge(root->right,country,disease,date1,date2,age1,age2,age3,age4);
    }
}

char* TopkAgeRanges(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country,int top){
    int i,j,size,count=0;
    int in = 0;

    date date1,date2;
    date1 = SetDate(str1);
    date2 = SetDate(str2);


    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    char* mess = malloc(300*sizeof(char));
    memset(mess, 0, sizeof(mess));
    strcpy(mess,"-");

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name, virus) == 0){
                    int age1=0,age2=0,age3=0,age4=0;

                    CountDiseaseWithAge(hashtable[i]->bucketnodeArray[j]->Btree,country,virus,date1,date2,&age1,&age2,&age3,&age4);

                    double ages[4];
                    int index[4];
                    int sum;

                    ages[0] = age1;
                    ages[1] = age2;
                    ages[2] = age3;
                    ages[3] = age4;
                    sum = age1+age2+age3+age4;
                    if(sum != 0){
                        in = 1;

                        int k;
                        for(k=0; k<4 ;k++){ //take persentages
                            ages[k] = (ages[k]/sum)*100;
                            index[k] = k;
                        }

                        for(k=0; k<3 ;k++){ //sort ages
                            int max = k;

                            for(int p=k+1; p<4 ;p++){
                                if(ages[p] > ages[max]){
                                    max = p;
                                }
                            }
                            double swap = ages[max];
                            ages[max] = ages[k];
                            ages[k] = swap;

                            index[k] = max;
                            index[max] = k;
                        }
                        for(k=0; k<top ;k++){
                            if(ages[k] != 0){
                                char tempmess[100];
                                memset(tempmess, 0, sizeof(tempmess));

                                if(index[k] == 0) snprintf(tempmess,100,"0-20: %.2f%%",ages[k]);
                                else if(index[k] == 1) snprintf(tempmess,100,"21-40: %.2f%%",ages[k]);
                                else if(index[k] == 2) snprintf(tempmess,100,"41-60: %.2f%%",ages[k]);
                                else snprintf(tempmess,100,"60+: %.2f%%",ages[k]);

                                // if(index[k] == 0) printf("0-20: %.2f%% \n",ages[k]);
                                // else if(index[k] == 1) printf("21-40: %.2f%% \n",ages[k]);
                                // else if(index[k] == 2) printf("41-60: %.2f%%\n",ages[k]);
                                // else printf("60+: %.2f%%\n",ages[k]);

                                if(strcmp(mess,"-") == 0) strcpy(mess,tempmess);
                                else sprintf(mess,"%s\n%s",mess,tempmess);


                            }
                        }
                        return mess;

                    }
                    return mess;
                }
            }

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }
}

char* FindinHashtable(hashtable* hashtable,int numEntries,char* str){
    int bucketSize = hashtable[0]->bytes;
    int size = bucketSize/sizeof(bucketnode);

    int index = hash(numEntries,str);

    char* record = malloc(516*sizeof(char));
    memset(record, 0, sizeof(record));

    bucket* temp = hashtable[index];
    while(temp != NULL){

        for(int i=0; i<size ;i++){
            if(strcmp(str,temp->bucketnodeArray[i]->name) == 0){

                char tempstr[200];
                snprintf(tempstr,sizeof(tempstr),"%s %s %s %s %d",str,temp->bucketnodeArray[i]->Btree->patient->patientFirstName,
                    temp->bucketnodeArray[i]->Btree->patient->patientLastName,temp->bucketnodeArray[i]->Btree->patient->diseaseID,
                    temp->bucketnodeArray[i]->Btree->patient->age);

                if(temp->bucketnodeArray[i]->Btree->patient->exitDate.day == -1){   //no exit date
                    snprintf(record,516*sizeof(char),"%s %d-%d-%d --",tempstr,temp->bucketnodeArray[i]->Btree->patient->entryDate.day,
                        temp->bucketnodeArray[i]->Btree->patient->entryDate.mon,temp->bucketnodeArray[i]->Btree->patient->entryDate.year);
                }
                else{
                    snprintf(record,516*sizeof(char),"%s %d-%d-%d %d-%d-%d",tempstr,temp->bucketnodeArray[i]->Btree->patient->entryDate.day,
                        temp->bucketnodeArray[i]->Btree->patient->entryDate.mon,temp->bucketnodeArray[i]->Btree->patient->entryDate.year,
                        temp->bucketnodeArray[i]->Btree->patient->exitDate.day,temp->bucketnodeArray[i]->Btree->patient->exitDate.mon,
                        temp->bucketnodeArray[i]->Btree->patient->exitDate.year);
                }

                return record;
            }
        }
        temp = temp->next;
    }

    strcpy(record,"-");
    return record;
}

void ListCountries(hashtable* hashtable,int numEntries){
    int i,j,size,check;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    check = getpid();

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name,"-") != 0){
                    printf("%s %.4d\n",hashtable[i]->bucketnodeArray[j]->name,check);
                }
            }

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }
}

char* PatientAdmissionsCountry(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country,int totaljobs,char* jobsArray[totaljobs]){
    int i,j,size,count=0;

    date date1,date2;
    date1 = SetDate(str1);
    date2 = SetDate(str2);

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    char* mess = malloc(200*sizeof(char));
    memset(mess, 0, sizeof(mess));
    strcpy(mess,"-");
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name, virus) == 0){
                    if(strcmp(country,"-") == 0){
                        for(int k=0; k<totaljobs ;k++){
                            count = 0;
                            CountPatientsCountry(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2, jobsArray[k]);

                            if(count != 0){
                                char tempmess[100];
                                memset(tempmess, 0, sizeof(tempmess));
                                snprintf(tempmess,100,"%s %d\n",jobsArray[k],count);

                                if(strcmp(mess,"-") == 0) strcpy(mess,tempmess);
                                else snprintf(mess,200,"%s%s",mess,tempmess);

                            }
                        }
                    }
                    else{
                        CountPatientsCountry(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2, country);

                        char tempmess[100];
                        memset(tempmess, 0, sizeof(tempmess));
                        snprintf(tempmess,100,"%s %d",country,count);

                        if(count != 0) strcpy(mess,tempmess);
                    }

                    return mess;
                }
            }

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }

    return mess;
}

void CountPatientsCountryExit(avlnode* root, int* count, date date1, date date2, char* country){

    if(root != NULL){

        CountPatientsCountryExit(root->left,count,date1,date2,country);

        if(strcmp(root->patient->country,country) == 0){
            if(root->patient->exitDate.day != -1){

                if(CompareDates(date1, root->patient->exitDate) && CompareDates(root->patient->exitDate, date2)){  // [date1,date2]
                    *count=*count+1;

                    avlnode* temp = root->next;
                    while(temp != NULL){
                        if(strcmp(temp->patient->country,country) == 0){
                            *count=*count+1;
                        }
                        temp = temp->next;
                    }
                }
            }
        }
        CountPatientsCountryExit(root->right,count,date1,date2,country);
    }
}

char* PatientDischarges(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country,int totaljobs,char* jobsArray[totaljobs]){
    int i,j,size,count=0;

    date date1,date2;
    date1 = SetDate(str1);
    date2 = SetDate(str2);

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    char* mess = malloc(200*sizeof(char));
    memset(mess, 0, sizeof(mess));
    strcpy(mess,"-");
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name, virus) == 0){
                    if(strcmp(country,"-") == 0){
                        for(int k=0; k<totaljobs ;k++){
                            count = 0;
                            CountPatientsCountryExit(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2, jobsArray[k]);

                            if(count != 0){
                                char tempmess[100];
                                memset(tempmess, 0, sizeof(tempmess));
                                snprintf(tempmess,100,"%s %d\n",jobsArray[k],count);

                                if(strcmp(mess,"-") == 0) strcpy(mess,tempmess);
                                else snprintf(mess,200,"%s%s",mess,tempmess);
                            }
                        }
                    }
                    else{
                        CountPatientsCountryExit(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2, country);

                        char tempmess[100];
                        memset(tempmess, 0, sizeof(tempmess));
                        snprintf(tempmess,100,"%s %d",country,count);

                        if(count != 0) strcpy(mess,tempmess);
                    }

                    return mess;
                }
            }

            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }
    return mess;
}


void DateExistsinTree(avlnode* root,int* exists,date date){
    if(root != NULL){
        DateExistsinTree(root->left,exists,date);

        if(CompareDates(date, root->patient->entryDate) == -1){
            *exists = 1;
        }

        DateExistsinTree(root->right,exists,date);
    }
}


void SummaryForFile(avlnode* root, int count, char diseaseArray[count*10][50], date file){

    if(root != NULL){
        SummaryForFile(root->left,count,diseaseArray,file);

        if(CompareDates(file, root->patient->entryDate) == -1){
            printf("%d-%d-%d\n%s\n",root->patient->entryDate.day,root->patient->entryDate.mon,
                root->patient->entryDate.year,root->patient->country);

            int slot=0,l=0,j=0,k;
            bool u=0;
            int used[count*5];
            used[0] = -1;

            avlnode* sec = root;
            while(strcmp(diseaseArray[l],"-") != 0){
                u = 0;
                for(k=0; k<slot ;k++){
                    if(used[k] == l){
                        u=1;
                        break;
                    }
                }
                if(u){
                    l++;
                    continue;
                }

                if(strcmp(sec->patient->diseaseID,diseaseArray[l]) == 0){
                    used[slot] = l;
                    used[slot+1] = -1;
                    slot++;

                    int age1=0,age2=0,age3=0,age4=0;

                    avlnode* temp = sec;
                    while(temp != NULL){
                        if(temp->patient->age <= 20) age1++;

                        else if(temp->patient->age <= 40) age2++;
                        else if(temp->patient->age <= 60) age3++;
                        else age4++;

                        temp = temp->next;
                        if(temp != NULL){
                            while(strcmp(temp->patient->diseaseID,diseaseArray[l]) != 0){
                                temp = temp->next;

                                if(temp == NULL) break;
                            }
                        }

                    }
                    printf("%s\n",diseaseArray[l] );
                    printf("Age range 0-20 years: %d cases\n",age1);
                    printf("Age range 21-40 years: %d cases\n",age2);
                    printf("Age range 41-60 years: %d cases\n",age3);
                    printf("Age range 60+ years: %d cases\n",age4);
                }
                l++;
                if(strcmp(diseaseArray[l],"-") == 0){
                    l = 0;
                    if(sec->next == NULL) break;
                    else sec = sec->next;
                }
            }
        }

        SummaryForFile(root->right,count,diseaseArray,file);
    }
}
