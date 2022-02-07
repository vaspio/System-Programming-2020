#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

    tok = strtok(str, "-");
    date1.day = atoi(tok);
    tok = strtok(NULL, "-");
    date1.mon = atoi(tok);
    tok = strtok(NULL, " ");
    date1.year = atoi(tok);

    return date1;
}
