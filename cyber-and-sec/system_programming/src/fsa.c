/*********************************************************
 *
 * Description: Implemention of a fixed size allocator
 * Author: Moshe Yifrah
 *
 *********************************************************/

#include <assert.h> /* assert */
#include <stddef.h> /* size_t, NULL */

#include "fsa.h" /* Header file */

/*********************************************************/

size_t FSASuggestSize(size_t num_of_blocks, size_t block_size)
{
	size_t block_aligned_size =
	    ((block_size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);

	return (num_of_blocks * block_aligned_size + sizeof(fsa_t));
}

/*********************************************************/

fsa_t *FSAInit(void *memory, size_t pool_size, size_t block_size)
{
	fsa_t *fsa = (fsa_t *)memory;
	size_t offset = sizeof(fsa_t);
	size_t block_aligned_size =
	    ((block_size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);

	assert(NULL != fsa);
	assert(pool_size != 0);
	assert(block_size != 0);
	assert(pool_size >= block_aligned_size + sizeof(fsa_t));

	for (; offset < pool_size - block_aligned_size; offset += block_aligned_size)
	{
		*((size_t *)((char *)memory + offset)) = offset + block_aligned_size;
	}

	*((size_t *)((char *)memory + offset)) = 0;

	fsa->next_free = sizeof(fsa_t);

	return fsa;
}

/*********************************************************/

void *FSAAlloc(fsa_t *fsa)
{
	size_t allocated_block = fsa->next_free;

	assert(NULL != fsa);

	if (0 == allocated_block)
	{
		return NULL;
	}

	fsa->next_free = *((size_t *)((char *)fsa + allocated_block));

	return ((void *)((char *)fsa + allocated_block));
}

/*********************************************************/

void FSAFree(fsa_t *fsa, void *ptr_block)
{
	assert(NULL != fsa);
	assert(NULL != ptr_block);

	*((size_t *)ptr_block) = fsa->next_free;
	fsa->next_free = (size_t)((char *)ptr_block - (char *)fsa);
}

/*********************************************************/
size_t FSACountFree(const fsa_t *fsa)
{
	size_t count = 0;
	size_t offset = fsa->next_free;

	assert(NULL != fsa);

	for (; 0 != offset; offset = *((size_t *)((char *)fsa + offset)))
	{
		++count;
	}

	return count;
}

/*********************************************************/
