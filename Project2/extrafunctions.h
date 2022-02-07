#pragma once

#include "struct.h"
#include "avltree.h"

// reads a message with buffer sized parts, merges it and returns it //
char* ReadMess(int readfd, int buffer);
// splits a message and writes it in buffer sized parts //
void WriteMess(char* mess, int writefd, int buffer);
// makes recursively an array with the diseases in the avl tree //
void MakeDiseaseArray(avlnode* root, int count, char diseaseArray[count*10][50]);
// finds the summary statistics and sends them to the parent with pipe //
void SummaryStatistics(avlnode* root, int count, char diseaseArray[count*10][50], int writefd, int buffer);
