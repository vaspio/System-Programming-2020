#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "struct.h"

int CompareDates(date date1, date date2){
    int res;
    if(date1.year == date2.year){
        if(date1.mon == date2.mon){
            if(date1.day == date2.day) res = -1;    //duplicates
            else if(date1.day > date2.day) res = 0;
            else res = 1;
        }
        else if(date1.mon > date2.mon) res = 0;
        else res = 1;
    }
    else if(date1.year > date2.year) res = 0;
    else res = 1;

    return res;
}

date SetDate(char* str){
    date date1;
    char* tok;

    char* temp = strdup(str);

    tok = strtok(temp, "-");
    date1.day = atoi(tok);
    tok = strtok(NULL, "-");
    date1.mon = atoi(tok);
    tok = strtok(NULL, " ");
    date1.year = atoi(tok);

    free(temp);
    
    return date1;
}


void SelectionSort(char array[][50], int rows){
    int i,j,min;
    date date1,date2;

    char temp[50];
    for(i=0; i<rows-1 ;i++){
        min = i;
        strcpy(temp, array[i]);

        for(j=i+1; j<rows ;j++){
            date1 = SetDate(array[j]);
            date2 = SetDate(temp);
            if(CompareDates(date1, date2) == 1){
                strcpy(temp, array[j]);
                min = j;
            }
        }

        if(min != i){
            char swap[50];
            strcpy(swap, array[i]);     //swap item[pos] and item[i]
            strcpy(array[i], array[min]);
            strcpy(array[min], swap);
        }
    }
}
