/*********************************************************
*
* Description: A multithreaded program that creates
* 100K threads and don't use join
* Author: Moshe Yifrah
*
* Compile with: gcc -pthread 100k_threads.c 
*
*********************************************************/

#include <stdio.h>   /* printf */
#include <stdlib.h>  /* malloc, NULL */
#include <pthread.h> /* pthread_create, pthread_join */

#define NUM_OF_THREADS 100000

/*********************************************************/

int arr_g[NUM_OF_THREADS];

/*********************************************************/

void Create100KThreads();
void* InsertVal(void* args);

/*********************************************************/

int main()
{
	Create100KThreads();
	pthread_exit(NULL);
}


/*********************************************************/

void Create100KThreads()
{
	pthread_t* threads = NULL;
	int i = 0;
	int first_wrong_index = -1;
	int correct_entries_count = 0;
	int threads_created = 0;
	
	threads = (pthread_t*)malloc(NUM_OF_THREADS * sizeof(pthread_t));
	
	if (NULL == threads)
	{
		perror("Failed to allocate memory for threads array\n");
		return;
	}
	
	for(i = 0; i < NUM_OF_THREADS; ++i)
	{
		if (0 != pthread_create(&threads[i], NULL, InsertVal, (void*)(size_t)i))
		{
			fprintf(stderr, "Thread %d create failed\n", i);
			break;
		}
		
		++threads_created;
	}
	
	for(i = 0; i < NUM_OF_THREADS; ++i)
	{
		if (i != arr_g[i])
		{
			if (-1 == first_wrong_index)
			{
				first_wrong_index = i;
			}
			
			/*printf("In index %d, value is: %d\n", i, arr_g[i]);*/
			continue;
		}
		
		++correct_entries_count;
	}
	
	printf("Correct entries: %d\n", correct_entries_count);
	
	if (-1 != first_wrong_index)
	{
		printf("First wrong index: %d\n", first_wrong_index);
	}
	
	free(threads);
}

/*********************************************************/

void* InsertVal(void* args)
{
	size_t my_index = (size_t)args;
	
	arr_g[my_index] = my_index;
	
	return NULL;
}


/*********************************************************/

