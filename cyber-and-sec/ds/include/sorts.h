/*********************************************************
*
* Description: Declarations for search and sort algorithms
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef __SORTS_H__
#define __SORTS_H__

/* Declarations ************************************************/

typedef int (*CmpFunc)(const void*, const void*);

/* Implementation Functions ***********************************/

/* Searches the element in the array using binary search iteratively 
 * Receives: Sorted array, size and an integer.
 * Returns: The index it was found, -1 if not.
 * Time Complexity: O(log(n))
 * Space Complexity: O(1)
 */
 
int BinarySearchIterative (const int* arr, size_t size, int element);

/* -----------------------------------------------------------*/

/* Searches the element in the array using binary search recursively 
 * Receives: Sorted array, size and an integer.
 * Returns: The index it was found, -1 if not.
 * Time Complexity: O(log(n))
 * Space Complexity: O(log(n))
 */
 
int BinarySearchRecursive (const int* arr, size_t size, int element);

/* -----------------------------------------------------------*/

/* Sorts the given array in place using bubble sort algorithm.
 * Receives: An array to sort and its size.
 * Time Complexity: O(n^2)
 * Space Complexity: O(1)
 */
 
void BubbleSort (int* arr, size_t size);
 
/* -----------------------------------------------------------*/

/* Sorts the given array using counting sort algorithm.
 * Receives: An array to sort and its size.
 * Returns: A pointer to the sorted array.
 * Time Complexity: O(n+k) when k is the range of the array (MAX - MIN + 1).
 * Space Complexity: O(n+k)
 */
 
int* CountingSort (int* arr, size_t size);

/* -----------------------------------------------------------*/

/* Sorts the given array in place using quick sort algorithm.
 * Receives: An array to sort, number of element and size in bytes 
 * of each element, and a pointer to a comparison function.
 * Time Complexity: Best-Avrg: O(n log(n)), worst: O(n^2).
 * Space Complexity: O(log(n)), worst: O(n).
 */
 
void QuickSort (void* arr, size_t elements, size_t size, CmpFunc cmp_func);

/* -----------------------------------------------------------*/

#endif
