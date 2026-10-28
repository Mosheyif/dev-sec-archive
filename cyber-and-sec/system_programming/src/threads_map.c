/*********************************************************
*
* Description: A multithreaded program that is used to map
* the address space of the threads.
* Author: Moshe Yifrah
*
* Compile with: -pthread
*
* In order to run the file, run the following command from
* the git home directory: 
* make -f compiling_necessities/threads_map/Makefile run

*********************************************************/

#include <stdio.h>     /* printf, getchar */
#include <stdlib.h>    /* malloc, NULL, exit */
#include <unistd.h>    /* sleep, getgid, pause */
#include <pthread.h>   /* pthread_create, pthread_join */
#include <sys/types.h> /* pid_t */

#include "string.h"    /* StrCpy */

/*********************************************************/

int global_var;
__thread int tls_var = 10;

/*********************************************************/

void* PrintHello(void* args);
void* PrintBye(void* args);

/*********************************************************/

int main()
{
	char* heap_val = NULL;
	pthread_t* t1 = NULL;
	pthread_t* t2 = NULL;
	int x = 5;
	char c = 0;
	
	heap_val = (char*) malloc(20);
	
	StrCpy(heap_val, "I'm in the heap");
	
	t1 = (pthread_t*) malloc(sizeof(pthread_t));
	t2 = (pthread_t*) malloc(sizeof(pthread_t));
	
	if (0 != pthread_create(t1, NULL, PrintHello, heap_val))
	{
		perror("Thread 1 create failed\n");
	}
	
	printf("The gloabl variable address at Main is: %p\n", (void*)&global_var);
	printf("The local variable address at Main is: %p\n", (void*)&x);
	printf("The heap variable address at Main is: %p\n", (void*)heap_val);
	
	if (0 != pthread_create(t2, NULL, PrintBye, heap_val))
	{
		perror("Thread 2 create failed\n");
	}
	
	if (0 != pthread_join(*t1, NULL))
	{
		perror("Thread 1 join failed\n");
	}
	
	if (0 != pthread_join(*t2, NULL))
	{
		perror("Thread 2 join failed\n");
	}
	
	printf("Exiting...\n");
	
	c = getchar();
	
	printf("Main is printing the char: %c\n", c);
	
	sleep(1);
	
	free(heap_val);
	free(t1);
	free(t2);
	
	return 0;
}

/*********************************************************/

void* PrintHello(void* args)
{
	int x = 2;
	char c = 0;
	
	if (NULL == args)
	{
		(void)args;
	}
	
	printf("Hello\n");
	printf("The gloabl variable address at PrintHello is: %p\n", (void*)&global_var);
	printf("The local variable address at PrintHello is: %p\n", (void*)&x);
	printf("The heap variable address at PrintHello is: %p\n", args);
	
	c = getchar();
	
	printf("PrintHello is printing the char: %c\n", c);
	
	return NULL;
}

/*********************************************************/

void* PrintBye(void* args)
{
	int x = 3;
	char c = 0;
		
	if (NULL == args)
	{
		(void)args;
	}
	
	printf("Bye\n");
	printf("The gloabl variable address at PrintBye is: %p\n", (void*)&global_var);
	printf("The local variable address at PrintBye is: %p\n", (void*)&x);
	printf("The heap variable address at PrintBye is: %p\n", args);
	
	c = getchar();
	
	printf("PrintBye is printing the char: %c\n", c);
	
	return NULL;
}

/*********************************************************/
