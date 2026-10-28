/*********************************************************
 *
 * Description: Test runner for the fixed-size allocator (FSA)
 * Author: Moshe Yifrah
 *
 * Compile with: make
 *
 * Run by: ./fsa_test
 *
 *********************************************************/

#include "fsa.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	size_t pool_size = FSASuggestSize(10, 12);
	printf("Size: %lu\n", pool_size);
	int *pool = (int *)malloc(pool_size);

	if (NULL == pool)
	{
		printf("Error in allocating the pool\n");
		exit(0);
	}

	fsa_t *fsa = FSAInit(pool, pool_size, 12);

	printf("Next free: %lu\n", fsa->next_free);

	int *block1 = FSAAlloc(fsa);
	int *block2 = FSAAlloc(fsa);
	int *block3 = FSAAlloc(fsa);
	int *block4 = FSAAlloc(fsa);
	int *block5 = FSAAlloc(fsa);
	int *block6 = FSAAlloc(fsa);
	printf("block1: %p, block2: %p\n", (void *)block1, (void *)block2);
	printf("Free: %lu\n", FSACountFree(fsa));
	FSAFree(fsa, block3);
	printf("Free: %lu\n", FSACountFree(fsa));
	FSAFree(fsa, block1);
	printf("Free: %lu\n", FSACountFree(fsa));
	FSAFree(fsa, block5);
	printf("Free: %lu\n", FSACountFree(fsa));
	FSAFree(fsa, block4);
	printf("Free: %lu\n", FSACountFree(fsa));
	FSAFree(fsa, block2);
	printf("Free: %lu\n", FSACountFree(fsa));
	FSAFree(fsa, block6);
	printf("Free: %lu\n", FSACountFree(fsa));
	block2 = FSAAlloc(fsa);
	printf("Free: %lu\n", FSACountFree(fsa));
	FSAFree(fsa, block2);
	printf("Free: %lu\n", FSACountFree(fsa));

	free(pool);

	return 0;
}
