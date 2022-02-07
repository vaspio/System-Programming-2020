#pragma once

#include <stdbool.h>
#include "struct.h"
#include "avltree.h"

typedef struct bucketnode{
      char* name;
      avltree Btree;    //pointer to an avl tree
}bucketnode;

typedef struct bucket{
    int bytes;  //size of bucket
    bucketnode** bucketnodeArray;   //array of bucketnodes
    struct bucket* next;
}bucket;

typedef bucket* hashtable;

    //  functions  //

// hash function taken from ~K08 //
int hash(int size, char* K);
// create new bucket //
bucket* CreateBucket(int bucketSize);
// create new hashtable //
hashtable* CreateHashtable(int numEntries, int bucketSize);
// insert item in bucket and make an avl tree for it //
bool InsertinBucket(hashtable* hashtable, patientRecord* patient, int index, char* string);
// insert in hashtable //
void InsertinHash(hashtable* hashtable, patientRecord* patient, char* str, int numEntries);
// finds if duplicate given entry exists //
bool FindDuplicates(hashtable* hashtable,int numEntries,char* str);
// finds duplicate entries //
bool CheckError(hashtable* hashtable,int numEntries);
// print the hashtable //
void PrintHashtable(hashtable* hashtable,int numEntries);
// free the hashtable //
void FreeHashTable(hashtable* hashtable,int numEntries);
