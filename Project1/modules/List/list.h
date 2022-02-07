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
// returns if error with duplicate ids exists //
bool CheckErrorIds(listnode* node);
// open a file and copy its contents into a new list //
list InitializeList(   FILE* file);
// free list nodes //
void FreeList(listnode* node);
