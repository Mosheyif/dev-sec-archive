/*********************************************************
*
* Description: Declarations for a chained hash table
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

/* Includes ******************************************************/

#include <stddef.h> /* size_t */
#include "slist.h"

/* Declarations ************************************************/

typedef struct hash_table hash_table_t;
typedef size_t (*HashFunc)(void*);

/* Implementation Functions ***********************************/

/* Initializes the table handler structure.
 * Receives: Number of buckets, a hash function and a compare 
 * function to compare between two keys.
 * Returns: Pointer to the handler.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
 
hash_table_t* HTCreate(size_t num_of_buckets, HashFunc hash_func, CmpFunc cmp_func);

/* -----------------------------------------------------------*/

/* Destroys (frees) the table.
 * Receives: Pointer to the table handler.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
 
void HTDestroy(hash_table_t* table);

/* -----------------------------------------------------------*/

/* Creates an element in the table containing the user's value. 
 * Receives: Pointer to the table, a unique key and a value.
 * Returns: void, overrides data.
 * Time Complexity: Average O(1), worst case O(n).
 * Space Complexity: O(1)
 */
 
void HTInsert(hash_table_t* table, void* key, void* value);

/* -----------------------------------------------------------*/

/* Removes the element retrieved by key.
 * Receives: Pointer to the table and a unique key.
 * Returns: Void, does nothing if key not found.
 * Time Complexity: Average O(1), worst case O(n).
 * Space Complexity: O(1)
 */
 
void HTRemove(hash_table_t* table, void* key);

/* -----------------------------------------------------------*/

/* Receives: Pointer to the table and a unique key.
 * Returns: The element identified by the key, if key not found,
 * returns dummy element.
 * Time Complexity: Average O(1), worst case O(n).
 * Space Complexity: O(1)
 */
 
void* HTFind(const hash_table_t* table, void* key); 

/* -----------------------------------------------------------*/

/* Counts table's elements.
 * Receives: Pointer to the table.
 * Returns: The number of elements currently stored in
 * the table.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */

size_t HTSize(const hash_table_t* table);

/* -----------------------------------------------------------*/

/* Checks whether the table is empty or not.
 * Receives: Pointer to the table.
 * Returns: 1 if the table is empty, else 0.
 * Time Complexity: O(num_of_buckets)
 * Space Complexity: O(1)
 */

int HTIsEmpty(const hash_table_t* table); 

/* -----------------------------------------------------------*/

/* For each element, calls the action function with the parameters.
 * Receives: Pointer to the table, pointer to the action
 * function and a generic pointer to the parameters.
 * Returns: 1 for failure and stops, 0 for success.
 * Time Complexity: O(n * action_function)
 * Space Complexity: O(n * action_function)
 */

int HTForEach(const hash_table_t* table, ActionFunc action_func, void* params);

/* -----------------------------------------------------------*/

#endif
