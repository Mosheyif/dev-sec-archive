/*********************************************************
*
* Description: Implemention of structures functions
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdio.h> /* printf */
#include <stdlib.h> /* malloc, realloc, free, NULL */
#include <assert.h> /* assert */

#include "struct.h"
#include "string.h"

/*********************************************************/

void ManageStruct ()
{
	elements array[NUM_OF_TYPES];

	if (!InitArr(array))
	{
		printf("Initialization error\n");
		return;
	}

	PrintArr(array);
	
	AddAll(array);
	
	PrintArr(array);
	
	CleanArr(array);
}

/*********************************************************/

int InitArr(elements array[])
{
	return
	(
		InitInt(&array[INT], 12) &&
		InitFloat(&array[FLOAT], 4.2) &&
		InitString(&array[STRING], "chapter")
	);
}

/*********************************************************/

int InitInt(elements* element, int value)
{
	element -> types.integer = value;
	element -> AddFunc = AddInt;
	element -> PrintFunc = PrintInt;
	element -> CleanUp = CleanUpInt;

	return 1;
}

/*********************************************************/

int InitFloat(elements* element, float value)
{
	element -> types.floating_p = value;
	element -> AddFunc = AddFloat;
	element -> PrintFunc = PrintFloat;
	element -> CleanUp = CleanUpFloat;

	return 1;
}

/*********************************************************/

int InitString(elements* element, char* str)
{
	size_t size = 0;

	assert(NULL != str);

	size = StrLen(str) + 1;

	element -> types.string = (char*) malloc(sizeof(char)*size);

	if (NULL == element -> types.string)
	{
	return 0;
	}

	element -> types.string = StrNCpy(element -> types.string, str, size);
	element -> AddFunc = AddString;
	element -> PrintFunc = PrintString;
	element -> CleanUp = CleanUpString;

	return 1;
}

/*********************************************************/

int AddInt(elements* element, int value)
{
	element -> types.integer += value;

	return 1;
}

/*********************************************************/

int AddFloat(elements* element, int value)
{
	element	->	types.floating_p += value;

	return 1;
}

/*********************************************************/

int AddString(elements* element, int value)
{
	int size = 0;
	char* str = NULL;
	char* temp = NULL;

	size = GetIntSize(value);
	str = ConvertIntToString(value, size);

	if (NULL == str)
	{
		return 0;
	}

	temp = realloc(element -> types.string, StrLen(element -> types.string) + size + 1);

	if (NULL == temp)
	{
		free(str);
		return 0;
	}

	element -> types.string = temp;
	StrCat(element -> types.string, str);

	free(str);
	str = NULL;

	return 1;
}
/*********************************************************/

void PrintInt(const elements* element)
{
	printf("Int value: %d\n", element -> types.integer);
}

/*********************************************************/

void PrintFloat(const elements* element)
{
	printf("Float value: %f\n", element -> types.floating_p);
}

/*********************************************************/

void PrintString(const elements* element)
{
	printf("String value: %s\n", element -> types.string);
}

/*********************************************************/
int CleanUpInt(elements* element)
{
	element -> types.integer = 0;
	
	return 1;
}

/*********************************************************/

int CleanUpFloat(elements* element)
{
	element -> types.floating_p = 0;
	
	return 1;
}

/*********************************************************/

int CleanUpString(elements* element)
{
	free(element -> types.string);
	element -> types.string = NULL;
	
	return 1;
}

/*********************************************************/

void PrintArr(const elements array[])
{
	size_t i = 0;
	
	for (; i < NUM_OF_TYPES; ++i)
	{
		array[i].PrintFunc(&array[i]);
	}
}

/*********************************************************/

int AddAll(elements array[])
{
	size_t i = 0, check = 0;
	
	for (; i < NUM_OF_TYPES; ++i)
	{
		check += array[i].AddFunc(&array[i], 10);
	}
	
 return (check == NUM_OF_TYPES);
 
}
/*********************************************************/

int CleanArr(elements array[])
{
	size_t i = 0, check = 0;
	
	for (; i < NUM_OF_TYPES; ++i)
	{
		check += array[i].CleanUp(&array[i]);
	}
	
	return (check == NUM_OF_TYPES);
}

/*********************************************************/

int GetIntSize(int value)
{
	int size = 0;
	
	if (value == 0)
	{
		return 1;
	}
	
	if (value < 0)
	{
		value = -value;
		size++;
	}
	
	for (; value != 0; value /= 10)
	{
		++size;
	}

	return size;
}

/*********************************************************/

char* ConvertIntToString(int value, int size)
{
	char* str = NULL;
	int is_negative = 0;
	int digit = 0;
	
	str = (char*)malloc(size + 1);
	
	if (NULL == str)
	{
			return NULL;
	}
	
	is_negative = (value < 0);
	
	str[size] = '\0';
	
	do
	{
		digit = value % 10;
		
		if (digit < 0) 
		{
			digit = -digit; 
		}
		
		str[--size] = digit + '0';
		value /= 10;
			
	} while (value != 0);
	
	if (is_negative)
	{
		str[0] = '-';
	}
	
	return str;
}

/*********************************************************/































