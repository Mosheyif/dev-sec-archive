/*********************************************************
*
* Description: Implemention of a variable size allocator
* Author: Moshe Yifrah
*
*********************************************************/

#include <assert.h> /* assert */
#include <stddef.h> /* size_t, NULL */
#include <stdint.h> /* SIZE_MAX */

#include "vsa.h"    /* Header file */

#define IS_FREE(info) (((info) & 1) == 0) 
#define GET_SIZE(info) ((info) & ~1UL)

vsa_error_t g_vsa_errno = VSA_SUCCESS;

#ifndef NDEBUG
static const size_t g_VSA_MAGIC = 0xDEADBEEF;
#endif

/*********************************************************/

struct block_header 
{
	size_t block_size;

	#ifndef NDEBUG

	size_t pool_id;

	#endif
};

/*********************************************************/

static void* FindBlock(const void*, size_t);
static header_t* SplitBlock(header_t*, size_t);
static void* GetFirstFitBlock(const void*, size_t);
static void* GetBestFitBlock(const void*, size_t);
static void InsertMagic(header_t*);
static void Defrag(const void*);

/*********************************************************/

void* VSAInit(void* pool, size_t pool_size)
{
	header_t* header = NULL;
	header_t* footer = NULL;
	
	if (NULL == pool)
	{
		g_vsa_errno = VSA_ERR_NULL_POOL;
		return NULL;
	}
	
	if (pool_size < sizeof(header_t) * 2)
	{
		g_vsa_errno = VSA_ERR_POOL_SIZE;
		return NULL;
	}
	
	header = (header_t*) pool;
	footer = (header_t*) ((char*) pool + pool_size - sizeof(header_t));
	
	header -> block_size = pool_size - 2 * sizeof(header_t);
	InsertMagic(header);
	
	footer -> block_size = 0;
	
	return (pool);
}

/*********************************************************/

void* VSAAlloc(const void* pool, size_t block_size)
{
	header_t* block = NULL;
	size_t block_aligned_size = ((block_size + sizeof(void*) - 1) / sizeof(void*)) * sizeof(void*);
	
	if (0 == block_size)
	{
		g_vsa_errno = VSA_ERR_BLOCK_SIZE;
		return NULL;
	}
	
	if (NULL == pool)
	{
		g_vsa_errno = VSA_ERR_NULL_POOL;
		return NULL;
	}	
	
	block = FindBlock(pool, block_aligned_size);
	
	if (NULL == block)
	{
		g_vsa_errno = VSA_ERR_BLOCK_SIZE;
		return NULL;
	}
	
	block = SplitBlock(block, block_aligned_size);

	block -> block_size |= 1;
	InsertMagic(block);
	
	return (void*) (block + 1);
}

/*********************************************************/

static void* FindBlock(const void* pool, size_t block_size)
{

	void* block = GetFirstFitBlock(pool, block_size);

	if (NULL == block)
	{
		Defrag(pool);
		block = GetBestFitBlock(pool, block_size);
	}

	return block;
}

/*********************************************************/

static header_t* SplitBlock(header_t* block, size_t block_size)
{
	size_t original_size = GET_SIZE(block->block_size);
	header_t* next_block = NULL;
	
	if (original_size >= block_size + sizeof(header_t) + sizeof(void*))
	{
		next_block = (header_t*) ((char*) block + sizeof(header_t) + block_size); 
		next_block -> block_size = original_size - block_size - sizeof(header_t);
		block -> block_size = block_size; 
		
		InsertMagic(next_block);
	}

	return block;
}

/*********************************************************/

static void* GetFirstFitBlock(const void* pool, size_t block_size)
{
	header_t* block = (header_t*) pool;
	
	while (0 < GET_SIZE(block -> block_size))
	{
		if (IS_FREE(block -> block_size) && block -> block_size >= block_size)
		{
			return block;
		}
	
	block = (header_t*)((char*) block + sizeof(header_t) + GET_SIZE(block -> block_size));
	}

	return NULL;
}

/*********************************************************/

static void* GetBestFitBlock(const void* pool, size_t block_size)
{
	header_t* block = (header_t*) pool;
	header_t* best_block = NULL;
	size_t best_size = SIZE_MAX;
	
	while (0 < GET_SIZE(block -> block_size))
	{
		if (IS_FREE(block -> block_size))
		{
			if (GET_SIZE(block -> block_size) == block_size)
			{
				return block;
			}
			
			if(GET_SIZE(block -> block_size) > block_size && GET_SIZE(block -> block_size) < best_size)
			{
				best_size = GET_SIZE(block -> block_size);
				best_block = block;
			}
		}
	
	block = (header_t*)((char*) block + sizeof(header_t) + GET_SIZE(block -> block_size));
	}

	return best_block;
}

/*********************************************************/

static void InsertMagic(header_t* block)
{
	(void)block;
	
	#ifndef NDEBUG
	
	block -> pool_id = g_VSA_MAGIC;
	
	#endif
}

/*********************************************************/

static void Defrag(const void* pool)
{
	header_t* block = NULL;
	header_t* next = NULL;
	
	if (NULL == pool)
	{
		g_vsa_errno = VSA_ERR_NULL_POOL;
		return;
	}
	
	block = (header_t*) pool;
	
	while (0 != GET_SIZE(block -> block_size))
	{
		next = (header_t*) ((char*) block + sizeof(header_t) + GET_SIZE(block -> block_size)); 
		
		if (0 != GET_SIZE(next -> block_size) &&
			IS_FREE(block -> block_size) &&
				IS_FREE(next -> block_size))
		{
			block -> block_size = GET_SIZE(block -> block_size) + GET_SIZE(next -> block_size) + sizeof(header_t);
		}
		
		else
		{
			block = next;
		}
	}
}

/*********************************************************/

void VSAFree(const void* block_ptr)
{
	header_t* block = NULL;
	
	if(NULL == block_ptr)
	{
		g_vsa_errno = VSA_ERR_NULL_BLOCK;
		return;
	}
	
	block = ((header_t*) block_ptr - 1);
	
	#ifndef NDEBUG
	
	if (block -> pool_id != g_VSA_MAGIC)
	{
		VSAPrintError(VSA_ERR_MAGIC_CORRUPTION);
	}
	
	assert(block -> pool_id == g_VSA_MAGIC);
	
	block -> pool_id = 0; 
	
	#endif
	
	if (IS_FREE(block -> block_size))
	{
		g_vsa_errno = VSA_ERR_DOUBLE_FREE;
		return;
	}
	
	block -> block_size &= ~1UL;
}

/*********************************************************/

size_t VSAMaxChunkAvailable(const void* pool)
{
	size_t max = 0;
	header_t* block = NULL;
	
	if (NULL == pool)
	{
		g_vsa_errno = VSA_ERR_NULL_POOL;
		return 0;
	}
	
	Defrag(pool);
	
	block = (header_t*) pool;
	
	while (0 < GET_SIZE(block -> block_size))
	{
		if (IS_FREE(block -> block_size) && GET_SIZE(block -> block_size) > max)
		{
			max = GET_SIZE(block -> block_size);
		}
	
	block = (header_t*)((char*) block + sizeof(header_t) + GET_SIZE(block -> block_size));
	}
	
	return max;
}

/*********************************************************/
