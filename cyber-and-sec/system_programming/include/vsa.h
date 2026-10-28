/*********************************************************
*
* Description: Declarations for a variable-size allocator (VSA)
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef __VSA_H__
#define __VSA_H__

/* Includes ******************************************************/

#include <stddef.h> /* size_t */

/* Declarations ************************************************/

typedef struct block_header header_t;

typedef enum vsa_error 
{
    VSA_SUCCESS = 0,
    VSA_ERR_NULL_POOL,
    VSA_ERR_POOL_SIZE,
    VSA_ERR_DOUBLE_FREE,
    VSA_ERR_MAGIC_CORRUPTION,
    VSA_ERR_BLOCK_SIZE,
    VSA_ERR_NULL_BLOCK
} vsa_error_t;

extern vsa_error_t g_vsa_errno;

/* Implementation Functions ***********************************/

/* Initializes the memory pool.
 * Receives: Pointer to raw memory and total pool size.
 * Returns: Pointer to the initialized memory pool.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
void* VSAInit(void* pool, size_t pool_size); 


/* Allocates a block of the specified size from the memory pool.
 * Receives: Pointer to the memory pool and a block size.
 * Returns: Pointer to the allocated block, or NULL if the pool is full.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
 
void* VSAAlloc(const void* pool, size_t block_size);


/* Frees a previously allocated block back to the pool.
 * Receives: Pointer to the block to free.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
void VSAFree(const void* block_ptr);


/* Calculates the largest chunk available in the pool.
 * Receives: Pointer to the memory pool.
 * Returns: The largest chunk available in the pool.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
 
size_t VSAMaxChunkAvailable(const void* pool);


/* Prints an appropriate error message to stderr based on the provided error code.
 * Receives: A vsa_error_t enum value.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
void VSAPrintError(vsa_error_t error);

#endif
