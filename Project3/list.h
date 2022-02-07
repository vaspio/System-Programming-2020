#pragma once

#include "struct.h"

typedef struct listnode{
    patientRecord* patient;
    struct listnode* next;
}listnode;

typedef listnode* list;

    //  functions  //

// create new node //
listnode* CreateListnode(listnode* prev);
// adds node to a list of struct patientRecord //
list AddListnode(listnode* node,char* recordID,char* patientFirstName,char* patientLastName,char* diseaseID,char* country,int age,char* entryDate,char* exitDate);
// checks if exit date given is validate else returns 1 //
bool CheckValidDate(listnode* node, char* id, char* exitdate);
// checks if patient id already exists in list //
bool CheckIdExists(listnode* node,char* id);
// returns if error with duplicate ids exists //
bool CheckErrorIds(listnode* node);
// free list nodes //
void FreeList(listnode* node);
