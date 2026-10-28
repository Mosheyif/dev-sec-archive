/*********************************************************
*
* Description: Test runner for search and sort algorithms
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./sorts_test
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sorts.h" /* Header file */

#define SIZE 5000

int BinarySearchTest(void);
int BubbleSortTest(void);
int CountingSortTest(void);
int QuickSortTest(void);

static int IsSorted(const int* arr, size_t size);
static void InitUnsortedArr(int arr[], int from, int to);
static int IntCmp(const void* arg1, const void* arg2);
static void PrintArr(int arr[]);


static void print_result(char* func_name, int result)
{
	printf("Function: %-15s %s\n", func_name, (result ? "Passed" : "Failed"));
}

int main()
{
	srand((unsigned int)time(NULL));
	
	printf("--- Sort & Search Tests ---\n\n");
	print_result("BinarySearches", BinarySearchTest());
	print_result("BubbleSort", BubbleSortTest());
	print_result("CountingSort", CountingSortTest());
	print_result("QuickSort", QuickSortTest());
	printf("\n------------------------------------\n");

	return 0;
}

static void InitUnsortedArr(int arr[], int from, int to)
{
	int i = 0;
	
	for(i = 0; i < SIZE; ++i)
	{
		arr[i] = from + (rand() % (to - from + 1));
	}
}

static void PrintArr(int arr[])
{
	int i = 0;
	
	for(i = 0; i < SIZE; ++i)
	{
		if (0 == i % 100)
		{
			printf("%d\t", arr[i]);
		}
		
		if(0 == i % 1000)
		{
		printf("\n");
		}
	}
	printf("\n");
}

int BinarySearchTest(void)
{
	int status = 1;
	int i = 0;
	int b_index = 0, q_index = 0, c_index = 0;
	int bubble_arr[SIZE] = {0};
	int quick_arr[SIZE] = {0};
	int* counting_arr = (int*) malloc(sizeof(int)*SIZE);
	
	if(NULL == counting_arr)
	{
		return 0;
	}
	
	InitUnsortedArr(bubble_arr, -1000000, 1000000);
	InitUnsortedArr(quick_arr, -1000000, 1000000);
	InitUnsortedArr(counting_arr, 0, 100);

	BubbleSort(bubble_arr, SIZE);
	QuickSort(quick_arr, SIZE, sizeof(int), IntCmp);
	counting_arr = CountingSort(counting_arr, SIZE);
	
	for(i = 0; i < SIZE && status; ++i)
	{
		b_index = BinarySearchIterative(bubble_arr, SIZE, bubble_arr[i]);
		q_index = BinarySearchIterative(quick_arr, SIZE, quick_arr[i]);
		c_index = BinarySearchIterative(counting_arr, SIZE, counting_arr[i]);
		
		status &= (b_index == i || bubble_arr[b_index] == bubble_arr[i]);
		status &= (q_index == i || quick_arr[q_index] == quick_arr[i] );
		status &= (c_index == i || counting_arr[c_index] == counting_arr[i] );
		
		status &= (b_index == BinarySearchRecursive(bubble_arr, SIZE, bubble_arr[i]));
		status &= (q_index == BinarySearchRecursive(quick_arr, SIZE, quick_arr[i]));
		status &= (c_index == BinarySearchRecursive(counting_arr, SIZE, counting_arr[i]));
	}
	
	free(counting_arr);
	return status;
}



int BubbleSortTest(void)
{
	int arr[SIZE] = {0};
	
	InitUnsortedArr(arr, -1000000, 1000000);
	
	printf("Before:\n");
	PrintArr(arr);
	
	BubbleSort(arr, SIZE);
	
	printf("After:\n");
	PrintArr(arr);
	
	return IsSorted(arr, SIZE);
}

int CountingSortTest(void)
{
	int status = 0;
	int* arr = (int*) malloc(sizeof(int)*SIZE);
	
	if(NULL == arr)
	{
		return 0;
	}
	
	InitUnsortedArr(arr, 0, 100);
	
	printf("Before:\n");
	PrintArr(arr);
	
	arr = CountingSort(arr, SIZE);
	
	printf("After:\n");
	PrintArr(arr);
	
	status = IsSorted(arr, SIZE);
	
	free(arr);
	
	return status;
}

int QuickSortTest(void)
{
	int arr[SIZE] = {0};
	
	InitUnsortedArr(arr, -1000000, 1000000);
	
	printf("Before:\n");
	PrintArr(arr);
	
	QuickSort(arr, SIZE, sizeof(int), IntCmp);
	
	printf("After:\n");
	PrintArr(arr);
	
	return IsSorted(arr, SIZE);
}

static int IntCmp(const void* arg1, const void* arg2)
{
	return (*(int*)arg1 - *(int*)arg2);
}


static int IsSorted(const int* arr, size_t size)
{
	size_t i = 0;

	if (size <= 1)
	{
		return 1;
	}

	for (i = 0; i < size - 1; ++i)
	{
		if (arr[i] > arr[i + 1])
		{
			return 0;
		}
	}

	return 1;
}

