#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define LIST_DATE_MAX	20
#define LIST_MSG_MAX	60

struct ListStruct
{
	char date[LIST_DATE_MAX+1];		/* date & time format: Y/M/D H:M:S (24h) */ 
	char msg[LIST_MSG_MAX+1];		/* notification message */
	struct ListStruct* next;		/* pointer to the next item */
};
typedef struct ListStruct ListStruct;

/*** Functions *********************************************************************/
void list_fillnode(ListStruct* node, char* date, char* msg);		/* fill struct node with data */

void list_pushfront(ListStruct** head, char* date, char* msg);		/* push node to the front of the list */
void list_popback(ListStruct* head);								/* pop last node of the list */
void list_clear(ListStruct** head);									/* delete all nodes from list */
uint8_t list_size(ListStruct* head);								/* return the number of nodes in the list */

#endif