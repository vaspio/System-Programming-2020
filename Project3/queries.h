#pragma once

#include "struct.h"
#include "hashtable.h"

// counts patients of an avl tree recursively //
void CountPatients(avlnode* root, int* count, date date1, date date2);
// counts patients of an avl tree recursively only in a country //
void CountPatientsCountry(avlnode* root, int* count, date date1, date date2, char* country);
// calculates and prints the number of cases for one disease(option:only in one country) //
int FrequencyStats(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country);
// counts for 4 age groups with country and disease given //
void CountDiseaseWithAge(avlnode* root,char* country,char* disease,date date1,date date2,int* age1,int* age2,int* age3,int* age4);
// finds and prints top-k percentage of age ranges for country and disease //
char* TopkAgeRanges(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country,int top);
// finds if str exists in hashtable and return record //
char* FindinHashtable(hashtable* hashtable,int numEntries,char* str);
// print the list of countries with the process id //
void ListCountries(hashtable* hashtable,int numEntries);
// prints total of patients in country(or all countries) for virus in dates //
char* PatientAdmissionsCountry(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country,int totaljobs,char* jobsArray[totaljobs]);
// counts patients of an avl tree that exited recursively only in a country //
void CountPatientsCountryExit(avlnode* root, int* count, date date1, date date2, char* country);
// prints total of patients exited in country(or all countries) for virus in dates //
char* PatientDischarges(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country,int totaljobs,char* jobsArray[totaljobs]);
// finds if date exists in tree //
void DateExistsinTree(avlnode* root,int* exists,date date);
// summary statistics only for a file //
void SummaryForFile(avlnode* root, int count, char diseaseArray[count*10][50], date file);
