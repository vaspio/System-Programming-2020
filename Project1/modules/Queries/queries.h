#pragma once
#include "struct.h"
#include "hashtable.h"

// counts patients of an avl tree recursively //
void CountPatients(avlnode* root, int* count, date date1, date date2);
// counts patients of an avl tree recursively only in a country //
void CountPatientsCountry(avlnode* root, int* count, date date1, date date2, char* country);
// calculates and prints the number of cases for each disease(can print for only period of time too) //
void DiseaseStats(hashtable* hashtable, char* str1, char* str2 ,int numEntries);
// calculates and prints the number of cases for one disease(option:only in one country) //
void FrequencyStats(hashtable* hashtable,char* str1,char* str2,int numEntries,char* virus,char* country);
// inserts exit date of a patient //
void InsertExitDate(hashtable* hashtable,char* recordId,char* exitDate,int numEntries);
// current hospitalized patients because of a disease //
void CountCurrentPatients(avlnode* root, int* count);
// calculates and prints current cases of diseases(or only one disease) //
void CurrentPatients(hashtable* hashtable, char* virus, int numEntries);
