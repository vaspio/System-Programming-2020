#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "extrafunctions.h"

char* ReadMess(int readfd, int buffer){
    char num[10] = "-";
    char mess[1024];
    memset(mess, 0, sizeof(mess));

    while(read(readfd, mess, 1) >= 0){  //read how many bytes needed to read
        if(strcmp(mess,"#") != 0){

            if(strcmp(num,"-") == 0) strcpy(num,mess);

            else strcat(num, mess);
        }
        else break;
    }
    char* send = malloc(atoi(num)*sizeof(char)+1);

    if(atoi(num) < buffer){
        if(read(readfd, mess, atoi(num)) == -1){
            printf("Problem writing in pipe\n");
        }
    }
    else{
        if(read(readfd, mess, buffer) == -1){
            printf("Problem writing in pipe\n");
        }

        char substr[buffer+1];
        int temp = atoi(num);
        while(temp >= buffer){
            temp -= buffer;

            memset(substr, 0, sizeof(substr));
            if(temp < buffer){
                if(read(readfd, substr, temp) == -1){
                    printf("Problem writing in pipe\n");
                }
            }
            else{
                if(read(readfd, substr, buffer) == -1){
                    printf("Problem writing in pipe\n");
                }
            }
            strcat(mess, substr);

        }
    }
    strcpy(send,mess);

    return send;
}

void WriteMess(char* mess, int writefd, int buffer){
    int chars = strlen(mess);

    int temp = chars;
    int digits = 0;
    while(temp > 0){
        temp /= 10;
        digits++;
    }

    char send[1024];
    memset(send, 0, sizeof(send));
    snprintf(send, sizeof(send),"%d#%s",chars,mess);

    int k=1;    // multiplier
    char substr[buffer+1];
    substr[buffer] = '\0';

    strncpy(substr, send, buffer);
    if(strlen(substr) < buffer){
        if(write(writefd, substr, strlen(substr)) == -1){
            printf("Problem writing in pipe\n");
        }
    }
    else{
        if(write(writefd, substr, buffer) == -1){
            printf("Problem writing in pipe\n");
        }

        temp = strlen(send);
        while(temp >= buffer){
            memset(substr, 0, sizeof(substr));
            strncpy(substr, send+(buffer*k), buffer);

            if(strlen(substr) < buffer){
                if(write(writefd, substr, strlen(substr)) == -1){
                    printf("Problem writing in pipe\n");
                }
            }
            else{
                if(write(writefd, substr, buffer) == -1){
                    printf("Problem writing in pipe\n");
                }
            }

            k++;
            temp -= buffer;
        }
    }
}

void MakeDiseaseArray(avlnode* root, int count, char diseaseArray[count*10][50]){   //inorder

    if(root != NULL){
        MakeDiseaseArray(root->left,count,diseaseArray);

        avlnode* temp = root;
        while(temp != NULL){
            int j = 0;
            while(strcmp(diseaseArray[j],temp->patient->diseaseID) != 0){
                if(strcmp(diseaseArray[j],"-") == 0){
                    strcpy(diseaseArray[j],temp->patient->diseaseID);
                    strcpy(diseaseArray[j+1],"-");
                    break;
                }
                j++;
            }
            temp = temp->next;
        }

        MakeDiseaseArray(root->right,count,diseaseArray);
    }
}

void SummaryStatistics(avlnode* root, int count, char diseaseArray[count*10][50], int writefd, int buffer){

    if(root != NULL){
        SummaryStatistics(root->left,count,diseaseArray,writefd,buffer);

        char head[100];
        memset(head, 0, sizeof(head));
        snprintf(head, sizeof(head),"%d-%d-%d\n%s",root->patient->entryDate.day,root->patient->entryDate.mon,root->patient->entryDate.year,root->patient->country);

        int chars,digits;
        chars = strlen(head);
        int find = chars;
        digits = 0;
        while(find > 0){
            find /= 10;
            digits++;
        }

        char headmess[128];
        memset(headmess, 0, sizeof(headmess));
        snprintf(headmess, sizeof(headmess),"%d#%s",chars,head);


        if(write(writefd ,headmess,strlen(headmess)+digits+2) == -1){
            printf("Problem writing in pipe from workers\n");
        }


        int slot=0,i=0,j=0,k;
        bool u=0;
        int used[count*5];
        used[0] = -1;

        avlnode* sec = root;
        while(strcmp(diseaseArray[i],"-") != 0){
            u = 0;
            for(k=0; k<slot ;k++){
                if(used[k] == i){
                    u=1;
                    break;
                }
            }
            if(u){
                i++;
                continue;
            }

            if(strcmp(sec->patient->diseaseID,diseaseArray[i]) == 0){
                used[slot] = i;
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
                        while(strcmp(temp->patient->diseaseID,diseaseArray[i]) != 0){
                            temp = temp->next;

                            if(temp == NULL) break;
                        }
                    }

                }
                // printf("%s\n",diseaseArray[i] );
                // printf("Age range 0-20 years: %d cases\n",age1);
                // printf("Age range 21-40 years: %d cases\n",age2);
                // printf("Age range 41-60 years: %d cases\n",age3);
                // printf("Age range 60+ years: %d cases\n",age4);

                char mess[500];
                memset(mess, 0, sizeof(mess));
                snprintf(mess, sizeof(mess),"%s\nAge range 0-20 years: %d cases\nAge range 21-40 years: %d cases\nAge range 41-60 years: %d cases\nAge range 60+ years: %d cases\n",diseaseArray[i],age1,age2,age3,age4);

                int chars,digits;
                chars = strlen(mess);
                int find = chars;
                digits = 0;
                while(find > 0){
                    find /= 10;
                    digits++;
                }

                char finalmess[516];
                memset(finalmess, 0, sizeof(finalmess));
                snprintf(finalmess, sizeof(finalmess),"%d#%s",chars,mess);

                if(write(writefd,finalmess,strlen(finalmess)+digits+2) == -1){
                    printf("Problem writing in pipe from workers\n");
                }

            }

            i++;
            if(strcmp(diseaseArray[i],"-") == 0){
                i = 0;
                if(sec->next == NULL) break;
                else sec = sec->next;
            }
        }

        SummaryStatistics(root->right,count,diseaseArray,writefd,buffer);
    }
}
