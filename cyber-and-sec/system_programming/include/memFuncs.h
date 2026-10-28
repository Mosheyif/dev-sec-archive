/*********************************************************
*
* Description: Declarations for custom memory utility functions
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef MEMFUNCS_H
#define MEMFUNCS_H

/* Includes ******************************************************/

#include <limits.h> /* CHAR_BIT */
#include <stddef.h> /* size_t */


/* Declarations ************************************************/

static const size_t WORD_BITS = sizeof(size_t)*CHAR_BIT;
static const size_t WORD_BYTES = sizeof(size_t);
static const int endian_test = 1;

/* Macros *******************************************************/

#define IS_LITTLE_ENDIAN ( *(char*)&(endian_test) == 1 )

/* Implemention Functions ***********************************/

void* Memset(void*, int, size_t);
void* Memcpy(void*, const void*, size_t);
void* Memmove(void*, const void*, size_t);
int IsLittleEndian();

/* Helping Functions ***************************************/

size_t CreateBuffer(unsigned char);

/* Testing Functions **************************************/

void print_result(char*, int);
int MemsetTest();
int MemcpyTest();
int MemmoveTest();
int IsLittleEndianTest();

#endif /* MEMFUNCS_H */























