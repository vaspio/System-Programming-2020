#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "struct.h"
#include "list.h"

listnode* CreateListnode(listnode* prev){
	listnode* node = (listnode*)malloc(sizeof(listnode));

    node->patient = (patientRecord*)malloc(sizeof(patientRecord));
    node->next = prev;

	return node;
}

list InitializeList(FILE* file){
    int count;
    listnode* temp=NULL;
	bool flag=0;	//for wrong dates
    char str[200],date1[15],date2[15];
    char* tok;
    char* tok2;


	while(fgets(str, 100, file) != NULL){
	    if(!flag) temp = CreateListnode(temp);

	    flag = 0;

	    tok = strtok(str, " ");
	    temp->patient->recordID = strdup(tok);
	    tok = strtok(NULL, " ");
	    temp->patient->patientFirstName = strdup(tok);
	    tok = strtok(NULL, " ");
	    temp->patient->patientLastName = strdup(tok);
	    tok = strtok(NULL, " ");
	    temp->patient->diseaseID = strdup(tok);
	    tok = strtok(NULL, " ");
	    temp->patient->country = strdup(tok);

	    tok = strtok(NULL, " ");
	    strcpy(date1,tok);
	    tok = strtok(NULL, " ");
	    strcpy(date2,tok);
		temp->patient->entryDate = SetDate(date1);
	    if(date2[0] != '-'){
	        temp->patient->exitDate = SetDate(date2);

	        if(CompareDates(temp->patient->exitDate, temp->patient->entryDate) == 1){ //check for errors
	            flag = 1;
	            printf("Entry with recordID:%s skipped because of wrong entry and exit dates\n",temp->patient->recordID);
	        }
	    }
	    else temp->patient->exitDate.day = -1;    // "-" blank

	}

	return temp;
}

bool CheckErrorIds(listnode* node){
	listnode* trav = NULL;

	if(node == NULL) return 1;

	while(node != NULL){	//check for error with recordID
		trav = node->next;
		while(trav != NULL){
			if(strcmp(node->patient->recordID, trav->patient->recordID) == 0){
				printf("Error with duplicate recordID:%s\n",node->patient->recordID);
				return 1;
			}
			trav = trav->next;
		}
		node = node->next;
	}
}

void FreeList(listnode* node){
	listnode* temp;

	while(node != NULL){
		temp = node->next;

		free(node->patient->recordID);
		free(node->patient->patientFirstName);
		free(node->patient->patientLastName);
		free(node->patient->diseaseID);
		free(node->patient->country);
		free(node->patient);
		free(node);
		node = temp;
	}
}
