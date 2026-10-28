/*********************************************************
*
* Description: Implemention of a function that acts according 
* to keyboard input
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc */

/*********************************************************/

#define NUM_OF_CHARS 256
#define ESC 27

typedef int (*func)();

/*********************************************************/

void PrintKeyPressedByIf();
void PrintKeyPressedBySwitch();
void PrintKeyPressedByLut();
int DoNothing();
int HandleEscape();
int PrintT();
int PrintA();
void InitArr(func []);

/*********************************************************/

int main()
{
	if (system("stty -icanon -echo"))
	{
		printf("System cmd error\n");
	}

	PrintKeyPressedByIf();
	PrintKeyPressedBySwitch();
	PrintKeyPressedByLut();

	if (system("stty icanon echo"))
	{
		printf("System cmd error\n");
	}

	return 0;
}

/*********************************************************/

void PrintKeyPressedByIf()
{
	char c;

	while(scanf("%c", &c))
	{
		if (c == 'A' || c == 'T')
		{
			printf("%c-pressed\n", c);	
		}

		else if (c == ESC)
		{
			return;
		}
	}
	
	return;
}

/*********************************************************/

void PrintKeyPressedBySwitch()
{
	char c;

	while(scanf("%c", &c))
	{
		switch (c)
		{
			case ('A'):
				printf("A-pressed\n");
				break;

			case ('T'):
				printf("T-pressed\n");
				break;

			case (ESC):
				return;

			default:
				break;
		}
	}
}

/*********************************************************/

int DoNothing()
{
	return 1;
}

/*********************************************************/

int HandleEscape()
{
	return 0;
}

/*********************************************************/

int PrintT()
{
	printf("T-pressed\n");
	return 1;
}

/*********************************************************/

int PrintA()
{
	printf("A-pressed\n");
	return 1;
}

/*********************************************************/

void InitArr(func arr[])
{
	int i;

	for (i = 0; i < NUM_OF_CHARS; i++)
	{
		arr[i] = DoNothing;
	}

	arr['A'] = PrintA;
	arr['T'] = PrintT;
	arr[ESC] = HandleEscape;
}

/*********************************************************/

void PrintKeyPressedByLut()
{
	func arr[NUM_OF_CHARS] = {0};
	char c;

	InitArr(arr);

	do
	{
		if (!scanf("%c", &c))
		{
			printf("scanf error\n");
		}
	} while (arr[(unsigned char) c]() != 0);

	return;
}

/*********************************************************/




























