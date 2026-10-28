/*********************************************************
 *
 * Description: Implemention of simple functions using pointers
 * Author: Moshe Yifrah
 *
 *********************************************************/

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/*********************************************************/

char *StrDup(const char *);

/*********************************************************/

extern char **environ;

/*********************************************************/

int SumRow(int *row, size_t cols)
{
	size_t col, sum = 0;

	for (col = 0; col < cols; col++)
	{
		sum += row[col];
	}

	return sum;
}

/*********************************************************/

void SumRows(size_t rows, size_t cols, int **arr, int *result)
{
	size_t row;

	for (row = 0; row < rows; row++)
	{
		result[row] = SumRow(arr[row], cols);
	}
}

/*********************************************************/

void InitArmy(size_t *army, size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
	{
		army[i] = (i + 1) % size;
	}
}

/*********************************************************/

size_t josephus(size_t *arr, size_t size)
{
	size_t i = 0, alive = size;

	InitArmy(arr, size);

	for (alive = size; alive > 1; alive--)
	{
		arr[arr[i]] = arr[arr[arr[i]]];
		i = arr[i];
	}

	return arr[i];
}

/*********************************************************/

void PrintDataTypes()
{
	printf("sizeof(char) = %lu\n", (unsigned long)sizeof(char));
	printf("sizeof(signed char) = %lu\n", (unsigned long)sizeof(signed char));
	printf("sizeof(unsigned char) = %lu\n", (unsigned long)sizeof(unsigned char));

	printf("sizeof(short) = %lu\n", (unsigned long)sizeof(short));
	printf("sizeof(short int) = %lu\n", (unsigned long)sizeof(short int));
	printf("sizeof(signed short) = %lu\n", (unsigned long)sizeof(signed short));
	printf("sizeof(signed short int) = %lu\n", (unsigned long)sizeof(signed short int));
	printf("sizeof(unsigned short) = %lu\n", (unsigned long)sizeof(unsigned short));
	printf("sizeof(unsigned short int) = %lu\n", (unsigned long)sizeof(unsigned short int));

	printf("sizeof(int) = %lu\n", (unsigned long)sizeof(int));
	printf("sizeof(signed) = %lu\n", (unsigned long)sizeof(signed));
	printf("sizeof(signed int) = %lu\n", (unsigned long)sizeof(signed int));
	printf("sizeof(unsigned) = %lu\n", (unsigned long)sizeof(unsigned));
	printf("sizeof(unsigned int) = %lu\n", (unsigned long)sizeof(unsigned int));

	printf("sizeof(long) = %lu\n", (unsigned long)sizeof(long));
	printf("sizeof(long int) = %lu\n", (unsigned long)sizeof(long int));
	printf("sizeof(signed long) = %lu\n", (unsigned long)sizeof(signed long));
	printf("sizeof(signed long int) = %lu\n", (unsigned long)sizeof(signed long int));
	printf("sizeof(unsigned long) = %lu\n", (unsigned long)sizeof(unsigned long));
	printf("sizeof(unsigned long int) = %lu\n", (unsigned long)sizeof(unsigned long int));

	printf("sizeof(float) = %lu\n", (unsigned long)sizeof(float));
	printf("sizeof(double) = %lu\n", (unsigned long)sizeof(double));
	printf("sizeof(long double) = %lu\n", (unsigned long)sizeof(long double));

	printf("sizeof(size_t) = %lu\n", (unsigned long)sizeof(size_t));
	printf("sizeof(ptrdiff_t) = %lu\n", (unsigned long)sizeof(ptrdiff_t));

	printf("sizeof(wchar_t) = %lu\n", (unsigned long)sizeof(wchar_t));
}

/*********************************************************/

char *ToLowerStr(char *s)
{
	size_t i;

	for (i = 0; s[i] != '\0'; i++)
	{
		s[i] = tolower(s[i]);
	}
	return s;
}

/*********************************************************/

size_t CountRows(const char **str)
{
	size_t rows = 0;

	while (*str != NULL)
	{
		rows++;
		str++;
	}

	return rows;
}

/*********************************************************/

void PrintEnvVars()
{
	char **env = environ;
	int i = 0;
	char **str = (char **)malloc(CountRows((const char **)env) * sizeof(char *));

	for (i = 0; env[i] != NULL; i++)
	{
		str[i] = ToLowerStr(strtok(StrDup(env[i]), "="));
		printf("%s\n", str[i]);
		free(str[i]);
	}

	free(str);
}

/*********************************************************/
