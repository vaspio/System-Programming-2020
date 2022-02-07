#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "struct.h"
#include "avltree.h"

avlnode* CreateAvlnode(patientRecord* patient){
    avlnode* node = (avlnode*)malloc(sizeof(avlnode));

    node->patient = patient;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    node->next = NULL;
    return node;
}

int Height(avlnode* node){
	int left,right;
	if(node == NULL) return 0;

	if(node->left == NULL) left = 1;
	else left = node->left->height + 1;

	if(node->right==NULL) right = 1;
	else right = node->right->height + 1;

    return (left > right) ? left : right;   //return bigger
}

avlnode* RightRotate(avlnode* x){
    avlnode* y = x->left;

    x->left = y->right;
    y->right = x;

    //new heights
    y->height = Height(y);
    x->height = Height(x);

    return y;   //Return new root
}

avlnode* LeftRotate(avlnode* x){
    avlnode* y = x->right;

    x->right = y->left;
    y->left = x;

    y->height = Height(y);
    x->height = Height(x);

    return y;
}

avlnode* Balance(avlnode* node){
    node->height = Height(node);

    if(Height(node->left) - Height(node->right) == 2){  //Left Left Case
        if(Height(node->left->right) > Height(node->left->left)){   //Left Right Case
            node->left = LeftRotate(node->left);
        }
        return RightRotate(node);
    }
    else if(Height(node->right) - Height(node->left) == 2){ //Right Right Case
        if(Height(node->right->left) > Height(node->right->right)){ //Right Left Case
            node->right = RightRotate(node->right);
        }
        return LeftRotate(node);
    }

    return node;
}

avlnode* AvlInsert(avlnode* node, patientRecord* patient){

    if(node == NULL) return CreateAvlnode(patient);

    if(CompareDates(patient->entryDate, node->patient->entryDate) == 1){
        node->left = AvlInsert(node->left, patient); //entryDate smaller
    }
    else if(CompareDates(patient->entryDate, node->patient->entryDate) == 0){
        node->right = AvlInsert(node->right, patient);
    }
    else{     //duplicates go there
        avlnode* temp = node;
        while(temp->next != NULL) temp = temp->next;

        temp->next = CreateAvlnode(patient);
        return node;
    }

    return Balance(node);
}

void PrintAvl(avlnode *root){   //inorder
    if(root != NULL){
        PrintAvl(root->left);
        if(strcmp(root->patient->recordID,"6361")==0)printf("rec:%s %02d/%02d/%d \n",root->patient->recordID,root->patient->entryDate.day,
                root->patient->entryDate.mon,root->patient->entryDate.year);
        PrintAvl(root->right);
    }
}

void FreeAvl(avlnode *root){
    if(root == NULL) return;

    FreeAvl(root->left);
    FreeAvl(root->right);

    if(root!=NULL){
        avlnode* temp;
        avlnode* prev = root->next;
        while(prev != NULL){
            temp = prev->next;
            free(prev);
            prev = temp;
        }
        free(root);
    }

}
