/*********************************************************
*
* Description: Declarations for a stack data structure
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef __STACK_H__
#define __STACK_H__

/* Includes ******************************************************/

#include <stddef.h> /* size_t */

/* Declarations ************************************************/

typedef struct stack_handler stack_t ;

/* Implementation Functions ***********************************/

/* Initializes the stack handler structure.
 * Receives: Maximum capacity of elements in the stack and a 
 * fixed size of each elements.
 * Returns: Pointer to the handler.
 * Time Complexity: O(1)
 * Space Complexity: O(n)
 */
 
stack_t* StackCreate(size_t capacity, size_t element_size);

/* -----------------------------------------------------------*/

/* Destroys (frees) the stack.
 * Receives: Pointer to the stack handler.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
void StackDestroy (stack_t* stack);

/* -----------------------------------------------------------*/

/* Pops the stack head element to the user.
 * Receives: Pointer to the stack handler.
 * Returns: Pointer to the popped block.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
void* StackPop(stack_t* stack);

/* -----------------------------------------------------------*/

/* Pushes the received element to the head of the stack.
 * Receives: Pointer to the stack handler and an element.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
void StackPush(stack_t* stack, const void* element);

/* -----------------------------------------------------------*/

/* Peeks to the stack head element.
 * Receives: Pointer to the stack handler.
 * Returns: Pointer to the peeked block.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
 
void* StackPeek(const stack_t* stack);

/* -----------------------------------------------------------*/

/* Calculates the occupied size of the stack.
 * Receives: Pointer to the stack handler.
 * Returns: The number of elements currently stored in
 * the stack.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

size_t StackSize(const stack_t* stack);

/* -----------------------------------------------------------*/

/* Checks whether the stack is empty or not.
 * Receives: Pointer to the stack handler.
 * Returns: 1 if the stack is empty, else 0.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

int StackIsEmpty(const stack_t* stack);

/* -----------------------------------------------------------*/

/* Returns the capacity of the stack.
 * Receives: Pointer to the stack handler.
 * Returns the capacity of the stack.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */

size_t StackCapacity(const stack_t* stack);

/* -----------------------------------------------------------*/

#endif
























