/*********************************************************
*
* Description: A multithreaded program that creates threads 
* with a complicated calculation in order to check the performance
* and finds the best number of threads
* Author: Moshe Yifrah
*
* Compile with: gcc -pthread multithread_sum_of_divs_opt.c
* Then run: "for i in 1 2 4 5 8 10 16 20; do echo "=== Threads: $i ==="; 
* perf stat ./a.out $i; echo ""; done"
*
*********************************************************/

#include <stdio.h>   /* printf */
#include <stdlib.h>  /* malloc, NULL */
#include <pthread.h> /* pthread_create, pthread_join */

#define NUM_OF_ROUNDS 10

/*********************************************************/

typedef struct range_of_cal
{
	size_t min;
	size_t max;
	size_t sum_of_divisors;
} range_t;

int number = 1000000000UL;

/*********************************************************/

void CreateThreads();
void* CalDivisorsInRange(void* args);

/*********************************************************/

int main(int argc, char *argv[])
{
	size_t num_threads = atoi(argv[1]);
	size_t range_per_thread = number / num_threads;
	
	if (argc != 2)
	{
		printf("Usage: %s <number_of_threads>\n", argv[0]);
		return 1;
	}
	
	CreateThreads(num_threads, range_per_thread);

	pthread_exit(NULL);
}


/*********************************************************/

void CreateThreads(size_t num_of_threads, size_t range_per_thread)
{
	pthread_t* threads = NULL;
	range_t* ranges = NULL;
	size_t total_sum = 0;
	size_t i = 0;
	
	threads = (pthread_t*)malloc(num_of_threads * sizeof(pthread_t));
	ranges = (range_t*)malloc(num_of_threads * sizeof(range_t));
	
	if (NULL == threads || NULL == ranges)
	{
		perror("Failed to allocate memory for threads array\n");
		
		if (NULL != threads) { free(threads); }
		if (NULL != ranges) { free(ranges); }
		
		return;
	}
	
	for(i = 0; i < num_of_threads; ++i)
	{
		ranges[i].min = i * range_per_thread + 1;
		ranges[i].max = (i + 1) * range_per_thread;
		ranges[i].sum_of_divisors = 0;
	
		if (0 != pthread_create(&threads[i], NULL, CalDivisorsInRange, &ranges[i]))
		{
			fprintf(stderr, "Thread %lu create failed\n", i);
			free(threads);
			return;
		}
	}
	
	for(i = 0; i < num_of_threads; ++i)
	{
		if (0 != pthread_join(threads[i], NULL))
		{
			fprintf(stderr, "Thread %lu join failed\n", i);
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
	int round = 0;
	
	for(round = 0; round < NUM_OF_ROUNDS; ++round)
	{
		for(i = range -> min; i <= range -> max; ++i)
		{
			if (0 == number % i)
			{
				local_sum += i;
			}
		}
	}

	range -> sum_of_divisors = local_sum / round;

	return NULL;
}


/*********************************************************/

