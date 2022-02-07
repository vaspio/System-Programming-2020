#pragma once

#include "struct.h"

typedef struct avlnode{
    patientRecord* patient;
    int height;
	struct avlnode* left;
    struct avlnode* right;
    struct avlnode* next;
}avlnode;

typedef avlnode* avltree;

    //  functions  //

// create a node for a AVL tree //
avlnode* CreateAvlnode(patientRecord* patient);
// returns height of an AVL node //
int Height(avlnode* node);
// right rotate subtree rooted with x //
avlnode* RightRotate(avlnode* x);
// left rotate subtree rooted with x //
avlnode* LeftRotate(avlnode* x);
// inserts an AVL node //
avlnode* AvlInsert(avlnode* node, patientRecord* patient);
// print the AVL tree //
void PrintAvl(avlnode *root);
// balances the tree //
avlnode* Balance(avlnode* node);
// free avl tree contents //
void FreeAvl(avlnode *root);
