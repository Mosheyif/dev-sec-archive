/*********************************************************
*
* Description: A multithreaded program that creates threads 
* with a complicated calculation in order to check the performance
* Author: Moshe Yifrah
*
* Compile with: gcc -pthread multithread_sum_of_divs.c
*
*********************************************************/

#include <stdio.h>   /* printf */
#include <stdlib.h>  /* malloc, NULL */
#include <pthread.h> /* pthread_create, pthread_join */

#define NUM_OF_THREADS 10000
#define RANGE_PER_THREAD 100000

/*********************************************************/

typedef struct range_of_cal
{
	size_t min;
	size_t max;
	size_t sum_of_divisors;
} range_t;

int number = NUM_OF_THREADS * RANGE_PER_THREAD;

/*********************************************************/

void CreateThreads();
void* CalDivisorsInRange(void* args);

/*********************************************************/

int main()
{	
	CreateThreads();

	pthread_exit(NULL);
}


/*********************************************************/

void CreateThreads()
{
	pthread_t* threads = NULL;
	range_t* ranges = NULL;
	size_t total_sum = 0;
	int i = 0;
	
	threads = (pthread_t*)malloc(NUM_OF_THREADS * sizeof(pthread_t));
	ranges = (range_t*)malloc(NUM_OF_THREADS * sizeof(range_t));
	
	if (NULL == threads || NULL == ranges)
	{
		perror("Failed to allocate memory for threads array\n");
		
		if (NULL != threads) { free(threads); }
		if (NULL != ranges) { free(ranges); }
		
		return;
	}
	
	for(i = 0; i < NUM_OF_THREADS; ++i)
	{
		ranges[i].min = i * RANGE_PER_THREAD + 1;
		ranges[i].max = (i + 1) * RANGE_PER_THREAD;
		ranges[i].sum_of_divisors = 0;
	
		if (0 != pthread_create(&threads[i], NULL, CalDivisorsInRange, &ranges[i]))
		{
			fprintf(stderr, "Thread %d create failed\n", i);
			free(threads);
			return;
		}
	}
	
	for(i = 0; i < NUM_OF_THREADS; ++i)
	{
		if (0 != pthread_join(threads[i], NULL))
		{
			fprintf(stderr, "Thread %d join failed\n", i);
			free(threads);
			free(ranges);
			return;
		}
		
		total_sum += ranges[i].sum_of_divisors;
	}
	
	printf("Sum of divisors of %u is: %lu\n", number, total_sum);
	
	free(threads);
	free(ranges);
}

/*********************************************************/

void* CalDivisorsInRange(void* args)
{
	range_t* range = (range_t*) args;
	size_t local_sum = 0;
	size_t i = 0;

	for(i = range -> min; i <= range -> max; ++i)
	{
		if (0 == number % i)
		{
			local_sum += i;
		}
	}

	range -> sum_of_divisors = local_sum;

	return NULL;
}


/*********************************************************/

