/*********************************************************
*
* Description: Implemention of a stack data structure
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdlib.h> /* malloc, free, size_t, NULL */
#include <string.h> /* memcpy */
#include <assert.h> /* assert */

#include "stack.h" /* Header file */

/*********************************************************/

struct stack_handler 
{
	size_t size;
	size_t capacity;
	size_t element_size;
	char data[1];
};

/*********************************************************/

stack_t* StackCreate(size_t capacity, size_t element_size)
{
	stack_t* stack = NULL;
	
	assert(0 != capacity);
	assert(0 != element_size);
	
	stack = (stack_t*) malloc(offsetof(stack_t, data) + capacity * element_size);
	
	if (NULL == stack)
	{
		return NULL;
	}
	
	stack -> size = 0;
	stack -> capacity = capacity;
	stack -> element_size = element_size;
	
	return stack;
}

/*********************************************************/

void StackDestroy (stack_t* stack)
{
	assert(NULL != stack);
	
	free(stack);
}

/*********************************************************/

void* StackPop(stack_t* stack)
{
	assert(NULL != stack);
	assert(0 < stack -> size);
	
	--stack -> size;
	
	return (stack -> data + stack -> size * stack -> element_size);
}

/*********************************************************/

void StackPush(stack_t* stack, const void* element)
{
	assert(NULL != stack);
	assert(NULL != element);
	assert(stack -> size < stack -> capacity);

	memcpy(stack -> data + stack -> size * stack -> element_size, element, stack -> element_size);
	
	++stack -> size;
}

/*********************************************************/

void* StackPeek(const stack_t* stack)
{
	assert(NULL != stack);
	assert(0 < stack -> size);
	
	return (void*) (stack -> data + (stack -> size - 1) * stack -> element_size);
}

/*********************************************************/

size_t StackSize(const stack_t* stack)
{
	assert(NULL != stack);
	
	return (stack -> size);
}

/*********************************************************/

int StackIsEmpty(const stack_t* stack)
{
	assert(NULL != stack);
	
	return (stack -> size == 0);
}

/*********************************************************/

size_t StackCapacity(const stack_t* stack)
{
	assert(NULL != stack);
	
	return (stack -> capacity);
}

/*********************************************************/

















