#pragma once

typedef struct topk{
    int cases;
    char* str;
}topk;

typedef struct heapnode{
    topk* top;
    struct heapnode* left;
    struct heapnode* right;
    struct heapnode* parent;
}heapnode;

typedef heapnode* maxHeap;

typedef struct qnode{
    heapnode* data;
    struct qnode* next;
}qnode;

    //  functions  //

// insert in Queue //
void Enqueue(heapnode* data);
// remove from queue //
void Dequeue();

// create new heap node //
heapnode* CreateHeapnode(int cases, char* str);
// returns last node of complete binary tree //
heapnode* LastNode(heapnode* root, int i);
// print heap //
void PrintHeap(heapnode* root);
// apply the properties of heap to the complete tree //
void Heapify(heapnode* node);

// returns nth virus and counts its patients for country //
topk CountForDisease(hashtable* hashtable,char* str1,char* str2,int numEntries,char* country,int pos);
// consruct the max heap (used in TopkDiseases()) //
heapnode* InsertToHeapCountry(heapnode* root,hashtable* hashtable, int numEntries, char* str,int* size,char* date1,char* date2);
// print and remove top values of nodes //
void ExtractTopk(heapnode* root,int top,int size);
// frees the heap //
void FreeHeap(heapnode* root);
// function to create a max heap and print top diseases of given country //
void TopkDiseases(hashtable* hashtable, int numEntries,int top,char* country,char* date1,char* date2);

// counts patients of an avl tree(country) recursively for a disease //
void CountDiseaseForCountry(avlnode* root, int* count, date date1, date date2, char* disease);
// returns nth country and counts its patients for virus //
topk CountForCountry(hashtable* hashtable,char* str1,char* str2,int numEntries,char* disease,int pos);
// consruct the max heap (used in TopkCountries()) //
heapnode* InsertToHeapVirus(heapnode* root,hashtable* hashtable, int numEntries, char* str, int* size,char* date1,char* date2);
// function to create a max heap and print top countris with most patients of disease //
void TopkCountries(hashtable* hashtable, int numEntries,int top,char* virus,char* date1,char* date2);
