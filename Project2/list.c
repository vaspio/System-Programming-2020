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

list AddListnode(listnode* node,char* recordID,char* patientFirstName,char* patientLastName,char* diseaseID,char* country,int age,char* entryDate,char* exitDate){
	node = CreateListnode(node);

	node->patient->recordID = strdup(recordID);
	node->patient->patientFirstName = strdup(patientFirstName);
	node->patient->patientLastName = strdup(patientLastName);
	node->patient->diseaseID = strdup(diseaseID);
	node->patient->country = strdup(country);
	node->patient->age = age;

	if(entryDate[0] != '-'){
		node->patient->entryDate = SetDate(entryDate);
	}
	else node->patient->entryDate.day = -1;


	if(exitDate[0] != '-'){
		node->patient->exitDate = SetDate(exitDate);
	}
	else node->patient->exitDate.day = -1;

	return node;
}

bool CheckValidDate(listnode* node, char* id, char* exitdate){

	if(node == NULL) return 0;

	date check = SetDate(exitdate);
	while(node != NULL){
		if(strcmp(node->patient->recordID, id) == 0){
			if(CompareDates(check,node->patient->entryDate) <= 0){
				return 0;	//valid date
			}
			return 1; 	//error
		}
		node = node->next;
	}

	return 1;
}

bool CheckIdExists(listnode* node,char* id){
	if(node == NULL) return 0;

	while(node != NULL){
		if(strcmp(node->patient->recordID, id) == 0){
			return 1;
		}
		node = node->next;
	}
	return 0;
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
