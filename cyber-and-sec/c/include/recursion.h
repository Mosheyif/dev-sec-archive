/*********************************************************
*
* Description: Declarations for recursion exercises
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef __RECURSION_H__
#define __RECURSION_H__

/* Includes ******************************************************/

#include "stack.h"

/* Declarations ************************************************/

typedef struct Node
{
    int data;
    struct Node* next;
} node_t;

typedef int (*cmp_func_t)(const void *data1, const void *data2);

/* Implementation Functions ***********************************/

/* Calculates the given element index of Fibonacci sequence iteratively 
 * Receives: Index
 * Returns: Its value in Fibonacci sequence
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */
 
int Fibonacci (int element_index);

/* -----------------------------------------------------------*/

/* Calculates the given element index of Fibonacci sequence recursively 
 * Receives: Index
 * Returns: Its value in Fibonacci sequence
 * Time Complexity: O(n^2)
 * Space Complexity: O(n)
 */
 
int FibonacciRecursion (int element_index);

/* -----------------------------------------------------------*/

/* Recursively flips a singly linked list
 * Time Complexity: O(n)
 * Space Complexity: O(n)
 */
 
 node_t* FlipList(node_t* node);
 
/* -----------------------------------------------------------*/

/* Receives: A stack and sorts it.
 * Time Complexity: O(n^2).
 * Space Complexity: O(n)
 */
 
void SortStack(stack_t* stack, cmp_func_t cmp);

/* -----------------------------------------------------------*/

/* Receives: String.
 * Returns: Its size excluding the null terminator.
 * Time Complexity: O(n).
 * Space Complexity: O(n)
 */
 
size_t RStrLen(const char* str);

/* -----------------------------------------------------------*/

/* Receives: 2 strings.
 * Returns: Their comparison value, str1 - str2.
 * Time Complexity: O(n)
 * Space Complexity: O(n)
 */
 
 int RStrCmp(const char* str1, const char* str2);

/* -----------------------------------------------------------*/

/* Receives: A destination and a source strings.
 * Returns: Copies the source string to destination and returns 
 * it. Must be allocated.
 * Time Complexity: O(n)
 * Space Complexity: O(n)
 */

char* RStrCpy(char* dst, const char* src);

/* -----------------------------------------------------------*/

/* Receives: A destination and a source strings.
 * Returns: Copies the source string to the end of the destination 
 * string and returns it. Must be allocated.
 * Time Complexity: O(n+m)
 * Space Complexity: O(n+m)
 */

char* RStrCat(char* dst, const char* src);

/* -----------------------------------------------------------*/

/* Receives: 2 strings.
 * Returns: Searches for the needle string in the haystack one
 * and returns a pointer to the found character in haystack, if 
 * not found returns NULL.
 * Time Complexity: O(n*m)
 * Space Complexity: O(n)
 */

char* RStrStr(const char *haystack, const char *needle);

/* -----------------------------------------------------------*/

#endif
