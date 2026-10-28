/*********************************************************
*
* Description: Declarations for a singly linked list
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef __SLIST_H__
#define __SLIST_H__

/* Includes ******************************************************/

#include <stddef.h> /* size_t */

/* Declarations ************************************************/

typedef struct list_handler list_t;
typedef struct node node_t;
typedef node_t* list_iter_t;
typedef int (*ActionFunc) (void*, void*);
typedef int (*CmpFunc) (void*, void*);

/* Implementation Functions ***********************************/

/* Initializes the list handler structure.
 * Receives: void.
 * Returns: Pointer to the handler.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
list_t* ListCreate(void);

/* -----------------------------------------------------------*/

/* Destroys (frees) the list.
 * Receives: Pointer to the list handler.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
 
void ListDestroy(list_t* list);

/* -----------------------------------------------------------*/

/* Creates a node with the recieved user data before the sent 
 * location in iter. 
 * Receives: Iter and data.
 * Returns: An iter pointing to the inserted node.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
list_iter_t ListInsert(list_iter_t iter, void* data);

/* -----------------------------------------------------------*/

/* Removes the node pointed by iter.
 * Receives: Iter.
 * Returns: The iter pointing to the node after the removed node.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
 
list_iter_t ListRemove(list_iter_t iter);

/* -----------------------------------------------------------*/

/* Returns the first node holding the data.
 * Receives: Pointer to the list handler, data and a pointer to a 
 * comparison function, implemented by the user.
 * Returns: The first node holding the data.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
 
list_iter_t ListFind(list_t* list, void* data, CmpFunc cmp_func);

/* -----------------------------------------------------------*/

/* Counts list's nodes.
 * Receives: Pointer to the list handler.
 * Returns: The number of nodes currently stored in
 * the list.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */

size_t ListCount(const list_t* list);

/* -----------------------------------------------------------*/

/* Checks whether the list is empty or not.
 * Receives: Pointer to the list handler.
 * Returns: 1 if the list is empty, else 0.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

int ListIsEmpty(const list_t* list);

/* -----------------------------------------------------------*/

/* For each node, calls the action function with the parameters.
 * Receives: Pointer to the list handler, pointer to the action
 * function and a generic pointer to the parameters.
 * Returns: How many failures the function had, 0 for success.
 * Time Complexity: O(action_function)
 * Space Complexity: O(action_function)
 */

int ListForEach(const list_t* list, ActionFunc func, void* params);

/* -----------------------------------------------------------*/

/* Receives: Pointer to the list handler.
 * Returns: The iterator of the first node.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

list_iter_t ListBegin(const list_t* list);

/* -----------------------------------------------------------*/

/* Receives: Pointer to the list handler.
 * Returns: The iterator of the last node.
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */

list_iter_t ListEnd(const list_t* list);

/* -----------------------------------------------------------*/

/* Receives: Iterator of a node.
 * Returns: The iterator of the next node.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

list_iter_t ListNext(const list_iter_t iter);

/* -----------------------------------------------------------*/

/* Receives: Iterator of a node.
 * Returns: Pointer to the data within the iterator's node.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

void* ListGetData(const list_iter_t iter);

/* -----------------------------------------------------------*/

/* Receives: Iterator of a node.
 * Returns: Pointer to the data within the iterator's node.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

void ListSetData(const list_iter_t iter, void* data);

/* -----------------------------------------------------------*/

/* Receives: Two iterators.
 * Returns: 1 if they are the same, else 0.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

int ListIsSameIterator(const list_iter_t iter1, const list_iter_t iter2);

#endif
























