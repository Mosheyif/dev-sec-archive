/*********************************************************
*
* Description: Implemention of a singly linked list
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdlib.h> /* malloc, free, size_t, NULL */
#include <assert.h> /* assert */

#include "slist.h" /* Header file */

/*********************************************************/

static int Count(void* element, void* params);

/*********************************************************/

struct node 
{
	void* data;
	node_t* next;
};

struct list_handler
{
	node_t* head;
};

/*********************************************************/

list_t* ListCreate(void)
{
	list_t* list = (list_t*) malloc(sizeof(list_t));
	node_t* head = (node_t*) malloc(sizeof(node_t));
	node_t* tail = (node_t*) malloc(sizeof(node_t));
	
	if (NULL == list || NULL == head || NULL == tail)
	{
		/* Freeing NULL is legal */
		
		free(list);
		free(head);
		free(tail);
		
		return NULL;
	}
	
	head -> data = NULL;
	head -> next = tail;
	
	tail -> data = NULL;
	tail -> next = NULL;
	
	list -> head = head;

	return list;
}

/*********************************************************/

void ListDestroy(list_t* list)
{
	node_t* node = NULL;
	node_t* next = NULL;
	
	assert(NULL != list);
	
	for  (node = list -> head; NULL != node; node = next)
	{
		next = node -> next;
		free(node);
	}
	
	free(list);
}

/*********************************************************/

list_iter_t ListInsert(list_iter_t iter, void* data)
{
	node_t* node = (node_t*) malloc(sizeof(node_t));
	
	if (NULL == node)
	{
		return iter;
	}
	
	node -> data = iter -> data;
	node -> next = iter -> next;
	iter -> next = node;
	iter -> data = data;
	
	return iter;
}

/*********************************************************/

list_iter_t ListRemove(list_iter_t iter)
{
	node_t* to_be_freed = NULL;
	
	assert(NULL != iter);
	assert(NULL != iter -> next);
	
	to_be_freed = iter -> next;
	
	iter -> data = to_be_freed -> data;
	iter -> next = 	to_be_freed -> next;
	
	free(to_be_freed);
	
	return iter;
}

/*********************************************************/

list_iter_t ListFind(list_t* list, void* data, CmpFunc cmp_func)
{
	list_iter_t iter = NULL;
	list_iter_t end = NULL;
	
	assert(NULL != list);
	assert(NULL != cmp_func);
	
	iter = ListBegin(list);
	end = ListEnd(list);
	
	while (!ListIsSameIterator(iter, end) && 0 != cmp_func(ListGetData(iter), data))
    {
        iter = ListNext(iter);
    }
	
	return iter;
}

/*********************************************************/

size_t ListCount(const list_t* list)
{
	size_t size = 0;
	
	assert(NULL != list);
	
	if (0 == ListForEach(list, Count, &size))
	{
		return size;
	}
	
	else 
	{
		return 0;
	}
}

/*---------------------------------------------------------*/

static int Count(void* element, void* params)
{
	(void) element;
	++ (*((size_t*) params));
	
	return 0;
}

/*********************************************************/

int ListIsEmpty(const list_t* list)
{
	assert(NULL != list);
	
	return (NULL == list -> head -> next -> next);
}

/*********************************************************/

int ListForEach(const list_t* list, ActionFunc func, void* params)
{
	list_iter_t iter = NULL;
	list_iter_t end = NULL;
	int failures = 0; 
	
	assert(NULL != list);
	assert(NULL != func);
	
	iter = ListBegin(list);
	end = ListEnd(list);
	
	while (!ListIsSameIterator(iter, end))
	{
		if (0 != func(ListGetData(iter), params))
		{
			++failures;
		}
		
		iter = ListNext(iter);
	}	
	
	return failures;
}

/*********************************************************/

list_iter_t ListBegin(const list_t* list)
{
	assert(NULL != list);
	
	return list -> head -> next;
}

/*********************************************************/

list_iter_t ListEnd(const list_t* list)
{
	node_t* node = NULL;
	
	assert(NULL != list);
	
	node = list -> head;
	
	while (NULL != node -> next)
	{
		node = node -> next;
	}
	
	return node;
}

/*********************************************************/

list_iter_t ListNext(const list_iter_t iter)
{
	return iter -> next;
}

/*********************************************************/

void* ListGetData(const list_iter_t iter)
{
	return iter -> data;
}

/*********************************************************/

void ListSetData(const list_iter_t iter, void* data)
{
	iter -> data = data;
}

/*********************************************************/

int ListIsSameIterator(const list_iter_t iter1, const list_iter_t iter2)
{
	return (iter1 == iter2);
}

/*********************************************************/













