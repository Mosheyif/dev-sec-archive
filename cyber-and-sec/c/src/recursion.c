/*********************************************************
*
* Description: Implemention of several functions using recursion
* Author: Moshe Yifrah
*
*********************************************************/

#include <assert.h> /* Assert */
#include <stddef.h> /* NULL */

#include "recursion.h" /* Header file */
#include "stack.h" /* Stack header file */

/*********************************************************/

static void SortedInsert(stack_t* stack, void* element, cmp_func_t cmp);
static size_t Count(const char* str, size_t size);
static void Copy (char* dst, const char* src);
static void Cat (char* dst, const char* src);
extern int StrNCmp(const char *s1, const char *s2, size_t n);

/*********************************************************/

int Fibonacci(int element_index)
{
	int prev = 0;
	int cur = 1;
	
	for (; element_index > 0; --element_index)
	{
		cur += prev;
		prev = cur - prev;
	}
	
	return prev;
}

/*---------------------------------------------------------*/

int FibonacciRecursion(int element_index)
{
	if (1 >= element_index)
	{
		return (element_index < 0) ? 0 : element_index;
	}
	return (FibonacciRecursion(element_index - 1) + FibonacciRecursion(element_index - 2));
}

/*********************************************************/

 node_t* FlipList(node_t* node)
 {
 	node_t* new_head = NULL;
 	
	if (NULL == node || NULL == node -> next)
	{
		return node;
	}
	
	new_head = FlipList(node -> next);
	
	node -> next -> next = node;
	
	node -> next = NULL;
	
	return new_head;
}

/*********************************************************/

void SortStack(stack_t* stack, cmp_func_t cmp)
{
	void* temp = NULL;
	
	if (StackIsEmpty(stack))
	{
		return;
	}
	
	temp = *(void**)StackPop(stack);
	
	SortStack(stack, cmp);
	
	SortedInsert(stack, &temp, cmp);
}

/*---------------------------------------------------------*/

static void SortedInsert(stack_t* stack, void* element, cmp_func_t cmp)
{
	void* top_element = NULL;
	
	if (StackIsEmpty(stack) || cmp(*(void**)element, *(void**)StackPeek(stack)) >= 0)
	{
		StackPush(stack, element);
		return;
	}
	
	top_element = *(void**)StackPop(stack);
	
	SortedInsert(stack, element, cmp);
	
	StackPush(stack, &top_element);
}

/*********************************************************/

size_t RStrLen(const char* str)
{
	if (NULL == str)
	{
		return 0;
	}
	
	return Count (str, 0);
}

/*---------------------------------------------------------*/

static size_t Count(const char* str, size_t size)
{
	if ('\0' == *str) 
	{
		return size;
	}
	
	return Count(str+1, size+1);
}

/*********************************************************/

int RStrCmp(const char* str1, const char* str2)
{
	assert (NULL != str1 && NULL != str2);
	
	if ('\0' == *str1 || '\0' == *str2 || *str1 != *str2)
	{
		return *str1 - *str2;
	} 
	
	return RStrCmp(str1+1, str2+1);
}

/*********************************************************/

char* RStrCpy(char* dst, const char* src)
{
	assert (NULL != dst && NULL != src);
	
	Copy(dst, src);
	
	return dst;
}

/*---------------------------------------------------------*/

static void Copy (char* dst, const char* src)
{
	*dst = *src;
	
	if ('\0' == *src)
	{
		return;
	}
	
	Copy(dst + 1, src + 1);
}

/*********************************************************/

char* RStrCat(char* dst, const char* src)
{
	assert (NULL != dst && NULL != src);
	
	Cat(dst, src);
	
	return dst;
}

/*---------------------------------------------------------*/

static void Cat (char* dst, const char* src)
{
	if ('\0' == *dst)
	{
		Copy (dst, src);
		return;
	}
	
	Cat(dst + 1, src);
}

/*********************************************************/

char* RStrStr(const char *haystack, const char *needle)
{
	if ('\0' == *needle || (*haystack == *needle && 
		0 == StrNCmp(haystack, needle, RStrLen(needle))))
	{
		return (char*)haystack;
	}
	
	if ('\0' == *haystack)
	{
		return NULL;
	}
	
	return RStrStr(haystack + 1, needle);
}


