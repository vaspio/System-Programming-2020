#pragma once

typedef struct date{
    int day;
    int mon;
    int year;
}date;

typedef struct patientRecord{
    char* recordID;
    char* patientFirstName;
    char* patientLastName;
    char* diseaseID;
    char* country;
    date entryDate;
    date exitDate;
}patientRecord;

// compares struct date variables //
int CompareDates(date date1, date date2);
// convert string to struct date //
date SetDate(char* str);
