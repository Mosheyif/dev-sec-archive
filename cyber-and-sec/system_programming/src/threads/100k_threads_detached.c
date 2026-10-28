/*********************************************************
*
* Description: A multithreaded program that creates
* 100K detached threads
* Author: Moshe Yifrah
*
* Compile with: gcc -pthread 100k_threads_detached.c 
*
*********************************************************/

#include <stdio.h>   /* printf */
#include <stdlib.h>  /* malloc, NULL */
#include <pthread.h> /* pthread_create, pthread_join */

#define NUM_OF_THREADS 1000000

/*********************************************************/

#define CACHE_LINE_SIZE 64

typedef struct 
{
	int value;
	char padding[CACHE_LINE_SIZE - sizeof(int)];
} cache_aligned_int_t;

cache_aligned_int_t arr_g[NUM_OF_THREADS];

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
	pthread_attr_t attr;
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
	
	if (0 != pthread_attr_init(&attr))
	{
		perror("Failed to init attr\n");
		free(threads);
		return;
	}

	if (0 != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
	{
		perror("Failed to set detached state\n");
	}
	
	for(i = 0; i < NUM_OF_THREADS; ++i)
	{
		if (0 != pthread_create(&threads[i], &attr, InsertVal, (void*)(size_t)i))
		{
			fprintf(stderr, "Thread %d create failed\n", i);
			break;
		}
		
		++threads_created;
	}
	
	for(i = 0; i < NUM_OF_THREADS; ++i)
	{
		if (i != arr_g[i].value)
		{
			if (-1 == first_wrong_index)
			{
				first_wrong_index = i;
			}
			
			/* printf("In index %d, value is: %d\n", i, arr_g[i].value);*/
			continue;
		}
		
		++correct_entries_count;
	}
	
	printf("Correct entries: %d\n", correct_entries_count);
	
	if (-1 != first_wrong_index)
	{
		printf("First wrong index: %d\n", first_wrong_index);
	}
	
	pthread_attr_destroy(&attr);
	free(threads);
}

/*********************************************************/

void* InsertVal(void* args)
{
	size_t my_index = (size_t)args;
	
	/* Option 2: Detaching from within the thread */
    pthread_detach(pthread_self());
    
	arr_g[my_index].value = my_index;
	
	return NULL;
}


/*********************************************************/

