/*********************************************************
 *
 * Description: Declarations for a fixed-size allocator (FSA)
 * Author: Moshe Yifrah
 *
 *********************************************************/

#ifndef __FSA_H__
#define __FSA_H__

/* Includes ******************************************************/

#include <stddef.h> /* size_t */

/* Declarations ************************************************/

struct fixed_sized_allocator
{
	size_t next_free;
};

typedef struct fixed_sized_allocator fsa_t;

/* Implementation Functions ***********************************/

/* Suggests the total memory size needed for the pool.
 * Receives: Number of blocks and size of each block needed by the user.
 * Returns: The total allocation size needed (including alignment and metadata).
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

size_t FSASuggestSize(size_t num_of_blocks, size_t block_size);

/* Initializes the memory pool.
 * Receives: Pointer to raw memory, total pool size, and block size.
 * Returns: Pointer to the initialized memory pool.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */

fsa_t *FSAInit(void *memory, size_t pool_size, size_t block_size);

/* Allocates a single block from the memory pool.
 * Receives: Pointer to the memory pool.
 * Returns: Pointer to the allocated block, or NULL if the pool is full.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

void *FSAAlloc(fsa_t *fsa);

/* Frees a previously allocated block back to the pool.
 * Receives: Pointer to the memory pool, and pointer to the block to free.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

void FSAFree(fsa_t *fsa, void *ptr_block);

/* Counts the number of available free blocks in the pool.
 * Receives: Pointer to the memory pool.
 * Returns: The number of free blocks.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */

size_t FSACountFree(const fsa_t *fsa_pool);

#endif
