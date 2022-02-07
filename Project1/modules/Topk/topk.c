#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "struct.h"
#include "avltree.h"
#include "hashtable.h"
#include "queries.h"
#include "topk.h"

qnode* head = NULL;
qnode* end = NULL;

void Enqueue(heapnode* data){
    qnode* node = malloc(sizeof(qnode));

    node->data = data;

    if(head == NULL) end = node;   //empty queue
    else head->next = node;

    head = node;
}
void Dequeue(){
    if(end != NULL){
        qnode* temp = end;
        if(end == head){
            head = NULL;
            end = NULL;
        }
        else end = end->next;

        // free(temp->str);
        free(temp);
    }
}
void FreeQ(){
    while(end != NULL){
        Dequeue();
    }
    free(head);
    free(end);
}

heapnode* CreateHeapnode(int cases, char* str){
    heapnode* node = (heapnode*)malloc(sizeof(heapnode));
    node->top = malloc(sizeof(topk));

    node->top->cases = cases;
    node->top->str = strdup(str);
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}
heapnode* LastNode(heapnode* root, int i){
    int bin = i;

    if(!root) return NULL;

    if(i == 1) return root;

    // Set bin to the value of the most significant binary digit set in bin
    while(bin & (bin - 1))
        bin = bin & (bin - 1);

    // Ιgnore highest binary digit
    bin >>= 1;
    while(bin){   //take the path
        if(i & bin){
            if(root->right) root = root->right;

            else return NULL;
        }
        else{
            if(root->left) root = root->left;

            else return NULL;
        }
        bin >>= 1;    //next
    }
    return root;
}

void PrintHeap(heapnode* root){
    if(root!=NULL){
        PrintHeap( root->left );
        printf("%d %s\n",root->top->cases,root->top->str );
        PrintHeap( root->right );
    }
}

void Heapify(heapnode* node){
    if(node->parent == NULL){
        printf("root here\n" );
        return;
    }

    char temp[20];
    int cas;
    heapnode* parent = node->parent;
    while(node->top->cases > parent->top->cases){
        strcpy(temp,node->top->str);
        cas = node->top->cases;

        free(node->top->str);
        node->top->str = strdup(parent->top->str);
        node->top->cases = parent->top->cases;

        free(parent->top->str);
        parent->top->str = strdup(temp);
        parent->top->cases = cas;

        node = parent;
        parent = node->parent;

        if(parent == NULL) break;
    }
}

topk CountForDisease(hashtable* hashtable,char* str1,char* str2,int numEntries,char* country,int pos){
    int i,j,size,count=0,num=0;
    bool flag=0;
    topk x;

    date date1,date2;
    if(strcmp(str1,"-") != 0){
        char old1[15],old2[15];
        strcpy(old1,str1);
        strcpy(old2,str2);

        date1 = SetDate(str1);
        date2 = SetDate(str2);

        strcpy(str1,old1);
        strcpy(str2,old2);
    }
    else date1.day = -1;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name, "-") != 0){
                    num++;
                    if(num == pos){
                        CountPatientsCountry(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2, country);
                        x.cases = count;
                        x.str = strdup(hashtable[i]->bucketnodeArray[j]->name);

                        hashtable[i] = temp;
                        return x;
                    }

                }
            }
            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }
    x.cases = -1;
    return x;
}

heapnode* InsertToHeapCountry(heapnode* root,hashtable* hashtable, int numEntries, char* str, int* size,char* date1,char* date2){
    heapnode* first = root;

    // take first disease and count for country
    topk top = CountForDisease(hashtable,date1,date2,numEntries,str,1);
    root = CreateHeapnode(top.cases,top.str);
    free(top.str);

    *size = *size+1;
    Enqueue(root);

    // take next disease and count for country
    top = CountForDisease(hashtable,date1,date2,numEntries,str,2);
    int count=2;
    while(top.cases != -1){
        heapnode* parent = end->data;

        Dequeue();

        heapnode* leftChild;
        leftChild = CreateHeapnode(top.cases,top.str);
        free(top.str);

        *size = *size+1;
        Enqueue(leftChild);
        parent->left = leftChild;
        leftChild->parent = parent;
        Heapify(leftChild);

        count++;
        top = CountForDisease(hashtable,date1,date2,numEntries,str,count);

        heapnode* rightChild = NULL;
        if(top.cases != -1){
            rightChild = CreateHeapnode(top.cases,top.str);
            free(top.str);

            *size = *size+1;
            Enqueue(rightChild);
            rightChild->parent = parent;
            parent->right = rightChild;
            Heapify(rightChild);

            count++;
            top = CountForDisease(hashtable,date1,date2,numEntries,str,count);
        }
        else parent->right = NULL;

    }
    return root;
}

void ExtractTopk(heapnode* root,int top,int size){
    // printf("Top %d\n",top );
    heapnode* last;
    int i;
    for(i=0; i<top ;i++){
        printf("%s %d \n",root->top->str,root->top->cases );

        last = LastNode(root, size);

        free(root->top->str);
        root->top->str = strdup(last->top->str);
        root->top->cases = last->top->cases;

        heapnode* parent = last->parent;
        if(parent->right == last) parent->right = NULL;
        else parent->left = NULL;

        free(last->top->str);
        free(last->top);
        free(last);
        size--;

        heapnode* temp = root;
        int left=-1,right=-1;
        if(temp->left != NULL) left = temp->left->top->cases;
        if(temp->right != NULL) right = temp->right->top->cases;
        while((left > temp->top->cases) || (right > temp->top->cases)){
            if(left >= right){
                Heapify(temp->left);
                temp = temp->left;
            }
            else{
                Heapify(temp->right);
                temp = temp->right;
            }
            left = -1;
            right = -1;
            if(temp->left != NULL) left = temp->left->top->cases;
            if(temp->right != NULL) right = temp->right->top->cases;
        }

    }
}

void FreeHeap(heapnode* root){
    if(root != NULL){
        FreeHeap(root->right);
        free(root->top->str);
        free(root->top);

        FreeHeap(root->left);
        free(root);
    }
}

void TopkDiseases(hashtable* hashtable, int numEntries,int top,char* country,char* date1,char* date2){
    if(top == 0){
        printf("error with top\n" );
        return;
    }
    head = NULL;
    end = NULL;
    int size=0;
    maxHeap maxHeap = InsertToHeapCountry(maxHeap,hashtable,numEntries,country,&size,date1,date2);

    // printf("root %d %s size:%d\n",maxHeap->top->cases,maxHeap->top->str,size );
    // PrintHeap(maxHeap);

    ExtractTopk(maxHeap,top,size);

    FreeHeap(maxHeap);
    FreeQ();

}

void CountDiseaseForCountry(avlnode* root, int* count, date date1, date date2, char* disease){

    if(root != NULL){
        CountDiseaseForCountry(root->left,count,date1,date2,disease);

        if(strcmp(root->patient->diseaseID,disease) == 0){
            if(date1.day == -1 || (CompareDates(date1, root->patient->entryDate) && CompareDates(root->patient->entryDate, date2))){  // [date1,date2]
                *count=*count+1;
                // printf("rec:%s\n", root->patient->recordID);

                avlnode* temp = root->next;
                while(temp != NULL){
                    // printf("next \n" );
                    if(strcmp(temp->patient->diseaseID,disease) == 0){
                        // printf("rec:%s\n", temp->patient->recordID);
                        *count=*count+1;
                    }
                    temp = temp->next;
                }
            }
        }
        CountDiseaseForCountry(root->right,count,date1,date2,disease);
    }
}

topk CountForCountry(hashtable* hashtable,char* str1,char* str2,int numEntries,char* disease,int pos){
    int i,j,size,count=0,num=0;
    bool flag=0;
    topk x;

    date date1,date2;
    if(strcmp(str1,"-") != 0){
        char old1[15],old2[15];
        strcpy(old1,str1);
        strcpy(old2,str2);

        date1 = SetDate(str1);
        date2 = SetDate(str2);

        strcpy(str1,old1);
        strcpy(str2,old2);
    }
    else date1.day = -1;

    int bucketSize = hashtable[0]->bytes;
    size = bucketSize/sizeof(bucketnode);

    bucket* temp;
    for(i=0; i<numEntries ;i++){
        temp = hashtable[i];
        do{
            for(j=0; j<size ;j++){
                if(strcmp(hashtable[i]->bucketnodeArray[j]->name, "-") != 0){
                    num++;
                    if(num == pos){
                        CountDiseaseForCountry(hashtable[i]->bucketnodeArray[j]->Btree, &count, date1, date2, disease);
                        x.cases = count;
                        x.str = strdup(hashtable[i]->bucketnodeArray[j]->name);

                        hashtable[i] = temp;
                        return x;
                    }

                }
            }
            hashtable[i] = hashtable[i]->next;
        }while(hashtable[i] != NULL);

        hashtable[i] = temp;
    }
    x.cases = -1;
    return x;
}

heapnode* InsertToHeapVirus(heapnode* root,hashtable* hashtable, int numEntries, char* str, int* size,char* date1,char* date2){
    heapnode* first = root;

    // take first country and count for disease
    topk top = CountForCountry(hashtable,date1,date2,numEntries,str,1);
    root = CreateHeapnode(top.cases,top.str);
    free(top.str);

    *size = *size+1;
    Enqueue(root);

    // take next country and count for disease
    top = CountForCountry(hashtable,date1,date2,numEntries,str,2);
    int count=2;
    while(top.cases != -1){
        heapnode* parent = end->data;

        Dequeue();

        heapnode* leftChild;
        leftChild = CreateHeapnode(top.cases,top.str);
        free(top.str);

        *size = *size+1;
        Enqueue(leftChild);
        parent->left = leftChild;
        leftChild->parent = parent;
        Heapify(leftChild);

        count++;
        top = CountForCountry(hashtable,date1,date2,numEntries,str,count);

        heapnode* rightChild = NULL;
        if(top.cases != -1){
            rightChild = CreateHeapnode(top.cases,top.str);
            free(top.str);

            *size = *size+1;
            Enqueue(rightChild);
            rightChild->parent = parent;
            parent->right = rightChild;
            Heapify(rightChild);

            count++;
            top = CountForCountry(hashtable,date1,date2,numEntries,str,count);
        }
        else parent->right = NULL;

    }
    return root;
}

void TopkCountries(hashtable* hashtable, int numEntries,int top,char* virus,char* date1,char* date2){
    if(top == 0){
        printf("error with top\n" );
        return;
    }
    head = NULL;
    end = NULL;
    int size=0;
    maxHeap maxHeap = InsertToHeapVirus(maxHeap,hashtable,numEntries,virus,&size,date1,date2);

    // PrintHeap(maxHeap);

    ExtractTopk(maxHeap,top,size);

    FreeHeap(maxHeap);
    FreeQ();
}
