/*********************************************************
*
* Description: Implemention of several search and sort algorithms
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdlib.h> /* malloc, calloc, free, NULL */
#include <assert.h> /* Assert */

#include "sorts.h" /* Header file */

/*********************************************************/

static int FindElement (const int* arr, int element, size_t left, size_t right);
static void Swap (int* i, int* j);
static void FindMaxAndMin (const int* arr, size_t size, int* max, int* min);
static int* InitCntArr (const int* arr, size_t size, size_t size_cnt, int min);
static int* CreateResultArr (const int* arr, int* cnt_arr, size_t size, int min);
extern void *Memcpy(void *dst, const void *src, size_t n);
static void Qsort (void* arr, size_t size, size_t left, size_t right, CmpFunc cmp_func, void* pivot_buffer);
static size_t RearrangeArr(void* arr, size_t size, size_t left, size_t right, CmpFunc cmp_func, void* pivot_buffer);
static void GenericSwap(void* a, void* b, size_t size);

/*********************************************************/

int BinarySearchIterative (const int* arr, size_t size, int element)
{
	size_t mid = 0;
	size_t left = 0;
	size_t right = size;
	
	assert(NULL != arr);
	
	while (left < right)
	{
		mid = left + (right - left) / 2;
		
		if (arr[mid] == element)
		{
			return (int) mid;
		}
		
		if (arr[mid] < element)
		{
			left = mid + 1;
		}
		else
		{
			right = mid;
		}
	}

	return -1;
}

/*---------------------------------------------------------*/

int BinarySearchRecursive (const int* arr, size_t size, int element)
{
	return FindElement (arr, element, 0, size);
}

/*---------------------------------------------------------*/

static int FindElement (const int* arr, int element, size_t left, size_t right)
{
	size_t mid = left + (right - left) / 2;
	
	if (left >= right)
	{
		return -1;
	}
	
	if (arr[mid] == element)
	{
		return (int) mid;
	}
	
	if (arr[mid] < element)
	{
		return FindElement (arr, element, mid + 1, right);
	}
	
	return FindElement (arr, element, left, mid);
}

/*********************************************************/

void BubbleSort (int* arr, size_t size)
{
	size_t i = 0;
	int swapped = 0;
	
	assert(NULL != arr);
	
	/* Only so size will not underflow */
	if (1 >= size)
	{
		return;
	}
	
	do
	{
		for (swapped = 0, i = 0; i + 1 < size; ++i)
		{
			if (arr[i] > arr[i+1])
			{
				Swap(&arr[i], &arr[i+1]);
				swapped = 1;
			}
		}
		
		/* 
		Better optimization.
		Last checked element is in its final place. 
		*/
		--size; 
		
	} while (swapped);
}

/*---------------------------------------------------------*/

static void Swap (int* i, int* j)
{
	int temp = *i;
	*i = *j;
	*j = temp;
}

/*********************************************************/

int* CountingSort (int* arr, size_t size)
{
	int* result_arr = NULL;
	int* cnt_arr = NULL;
	int max = 0, min = 0;
	
	assert(NULL != arr);
	
	if (1 >= size)
	{
		return arr;
	}
	
	FindMaxAndMin(arr, size, &max, &min);
	
	cnt_arr = InitCntArr(arr, size, max - min + 1, min);
	
	if (NULL == cnt_arr)
	{
		return NULL;
	}

	result_arr = CreateResultArr(arr, cnt_arr, size, min);
	
	if (NULL == result_arr)
	{
		free(cnt_arr);
		return NULL;
	}
	
	Memcpy (arr, result_arr, sizeof(int) * size);
	
	free(result_arr);
	free(cnt_arr);
	
	return arr;
}

/*---------------------------------------------------------*/

static int* InitCntArr (const int* arr, size_t size, size_t size_cnt, int min)
{
	size_t i = 0;
	int* cnt_arr = (int*) calloc (size_cnt, sizeof(int));
	
	if (NULL == cnt_arr)
	{
		return NULL;
	}
	
	for (i = 0; i < size; ++i)
	{
		cnt_arr[arr[i] - min]++;
	}
	
	for (i = 1; i < size_cnt; ++i)
	{
		cnt_arr[i] += cnt_arr[i-1];
	}
	
	return cnt_arr;
}

/*---------------------------------------------------------*/

static void FindMaxAndMin (const int* arr, size_t size, int* max, int* min)
{
	size_t i = 0;
	*max = *min = arr[0];
	
	for (i = 1; i < size; ++i)
	{
		*max = (*max > arr[i])? *max : arr[i];
		*min = (*min < arr[i])? *min : arr[i];
	}
}

/*---------------------------------------------------------*/

static int* CreateResultArr (const int* arr, int* cnt_arr, size_t size, int min)
{
	size_t i = 0;
	int sorted_index = 0;
	int* result_arr = (int*) malloc (sizeof(int) * size);
	
	if (NULL == result_arr)
	{
		return NULL;
	}
	
	for(i = size; i > 0; --i)
	{
		sorted_index = cnt_arr[arr[i-1] - min] -1;
		result_arr[sorted_index] = arr[i-1];
		--cnt_arr[arr[i-1] - min];
	}
	
	return result_arr;
}

/*********************************************************/

void QuickSort (void* arr, size_t elements, size_t size, CmpFunc cmp_func)
{
	void* pivot_buffer = NULL;
	
	if (1 >= elements)
	{
		return;
	}
	
	pivot_buffer = malloc(size);
	
	if (NULL == pivot_buffer)
	{
		return;
	}

	Qsort(arr, size, 0, elements -1, cmp_func, pivot_buffer);
	
	free(pivot_buffer);
}

/*---------------------------------------------------------*/

static void Qsort (void* arr, size_t size, size_t left, size_t right, CmpFunc cmp_func, void* pivot_buffer)
{
	size_t split_index = 0;
	
	if (left >= right)
	{
		return;
	}
	
	split_index = RearrangeArr(arr, size, left, right, cmp_func, pivot_buffer);
	
	Qsort(arr, size, left, split_index, cmp_func, pivot_buffer);
	Qsort(arr, size, split_index +1, right, cmp_func, pivot_buffer);
}

/*---------------------------------------------------------*/

static size_t RearrangeArr(void* arr, size_t size, size_t left, size_t right, CmpFunc cmp_func, void* pivot_buffer)
{
	size_t pivot = left + (right - left) / 2;
	int i = (int) left -1;
	int j = (int) right +1;
	
	Memcpy(pivot_buffer, (char*) arr + (pivot * size), size);
	
	while (1)
	{
		do
		{
			++i;
		} while (cmp_func((char*)arr + (i * size), pivot_buffer) < 0);
	
		do
		{
			--j;
		} while (cmp_func((char*)arr + (j * size), pivot_buffer) > 0);
		
		if (i >= j)
		{
			return (size_t) j;
		}
		
		GenericSwap(((char*) arr + i * size), ((char*) arr + j * size), size);
	}
}

/*---------------------------------------------------------*/

static void GenericSwap(void* a, void* b, size_t size)
{
	char* p1 = (char*)a;
	char* p2 = (char*)b;
	char temp = 0;
	size_t i = 0;

	for (i = 0; i < size; ++i)
	{
		temp = p1[i];
		p1[i] = p2[i];
		p2[i] = temp;
	}
}

/*********************************************************/






