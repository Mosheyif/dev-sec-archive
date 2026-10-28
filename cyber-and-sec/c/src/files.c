/*********************************************************
*
* Description: Implemention of simple struct printer
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdio.h> /* printf */

#define SIZE 10

/*********************************************************/

typedef void (*func)(int);

typedef struct Print_me 
{

	int num;
	func Print;
	
} print_me;

/*********************************************************/

void PrintStruct();
void InitStruct(print_me []);
void Print(int);

/*********************************************************/

int main()
{
	PrintStruct();

	return 0;
}

/*********************************************************/

void PrintStruct()
{
	print_me arr[SIZE];
	int i;

	InitStruct(arr);

	for (i = 0; i < SIZE; i++)
	{
		arr[i].Print(arr[i].num);
	}
}

/*********************************************************/

void InitStruct(print_me arr[])
{
	int i;

	for (i = 0; i < SIZE; i++)
	{
		arr[i].num = i;
		arr[i].Print = Print;
	}
}

/*********************************************************/

void Print(int num)
{
	printf("%d\n" , num);
}

/*********************************************************/






