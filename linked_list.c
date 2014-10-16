/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    linked_list.c
  * @brief   Linked list functions
  *          This file contains the implementation of a linked list
  *          for notification messages.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "main.h"
#include "linked_list.h"

#include <stdio.h>
#include <stdlib.h>

/*** Fill node with data ***********************************************************/
void list_fillnode(ListStruct* node, char* date, char* msg)
{
	uint8_t i;
	
	/* copy date str */
	for(i=0; (i<LIST_DATE_MAX && date[i] != '\0'); ++i)
	{
		node->date[i] = date[i];
	}
	node->date[i] = '\0';
	
	/* copy message str */
	for(i=0; (i<LIST_MSG_MAX && msg[i] != '\0'); ++i)
	{
		node->msg[i] = msg[i];
	}
	node->msg[i] = '\0';
	
	/* set next pointer to null */
	node->next = NULL;
}

/*** Push node to the front  *******************************************************/
void list_pushfront(ListStruct** head, char* date, char* msg)
{	
	ListStruct* alias = *head;
	ListStruct* newitem = (ListStruct*)malloc(sizeof(ListStruct));
	
	list_fillnode(newitem, date, msg);
	
	/* if there is no head yet */
	if(alias == NULL)
	{
		*head = newitem;
		return;
	}
	
	newitem->next = alias;
	*head = newitem;
}

/*** Pop last node  ****************************************************************/
void list_popback(ListStruct* head)
{
	if(head == NULL) { return; }
	
	ListStruct* iterator = head;
	ListStruct* last = head;
	
	while(iterator->next != NULL)
	{
		last = iterator;
		iterator = iterator->next;
	}
	
	free(iterator);
	last->next = NULL;
}

/*** Clear list ********************************************************************/
void list_clear(ListStruct** head)
{
	ListStruct* iterator = *head;
	while(iterator != NULL)
	{
		ListStruct* next = iterator->next;
		free(iterator);
		iterator = next;
	}
	
	*head = NULL;
}

/*** Get list size *****************************************************************/
uint8_t list_size(ListStruct* head)
{
	if(head == NULL) { return 0; }
	
	uint8_t cntr = 1;
	ListStruct* iterator = head;
	while(iterator->next != NULL)
	{
		iterator = iterator->next;
		cntr++;
	}
	return cntr;
}