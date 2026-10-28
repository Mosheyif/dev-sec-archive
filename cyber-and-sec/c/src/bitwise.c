/*********************************************************
*
* Description: Implemention of bitwise operations functions
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdio.h> /* printf */
#include <assert.h> /* assert */
#include <limits.h> /* CHAR_BIT */

#include "bitwise.h"

/*********************************************************/

long Pow2 (unsigned int x, unsigned int y)
{
	/* Limiting the maximum shifting allowed (31) */
	assert(y < (sizeof(unsigned int) * CHAR_BIT));

	return (long)(Multiply(1UL << y, (unsigned long int)x));
}

unsigned long Multiply (unsigned long x, unsigned long y)
{
	unsigned long sum = 0;

	while (0 != y)
	{
		if ((1 & y) > 0)
		{
			sum = Add(sum, x);
		}
		
		x <<= 1;
		y >>= 1;
	} 

	return sum;
}

unsigned long Add (unsigned long x, unsigned long y)
{
	unsigned long sum, leftover;

	do
	{
		sum = x ^ y;
		leftover = x & y;
		leftover <<= 1;

		x = sum;
		y = leftover;

	} while (leftover != 0);

	return sum;
}

/*********************************************************/

int IsPowerOf2Loop(unsigned int n)
{
	while (n > 0)
	{
		if ((1 & n) > 0)
		{
			return ((n >> 1) == 0);
		}
		
		n >>= 1;
	}
	
	return 0;
}

/*********************************************************/

int IsPowerOf2(unsigned int n)
{
	return ((0 < n) && (n & (n - 1)) == 0);
}

/*********************************************************/

int AddOne(int num)
{
	return (int) Add(1UL, (unsigned long) num);
}

/*********************************************************/

void Print3Bits(unsigned int *arr, size_t size)
{
	size_t i;

	assert(NULL != arr);

	for (i = 0; i < size; ++i)
	{
		if (CountSetBits((int)arr[i]) == 3)
		{
			printf("%u ", arr[i]);
		}
	}
	
	printf("\n");
}

/*********************************************************/

unsigned char ByteMirrorLoop(unsigned char word)
{
	unsigned char result = 0;
	int i;

	for (i = 0; i < CHAR_BIT; ++i)
	{
		result <<= 1;
		result |= (1 & word);
		word >>= 1;
	}
	
	return result;
}

/*********************************************************/

unsigned char ByteMirror(unsigned char byte)
{

	byte = ((byte & 0xF0) >> 4) | ((byte & 0x0F) << 4);
	byte = ((byte & 0xCC) >> 2) | ((byte & 0x33) << 2);
	byte = ((byte & 0xAA) >> 1) | ((byte & 0x55) << 1);

	return byte;
}

/*********************************************************/

int IsTwoAndSixBitsOn(unsigned char byte)
{
	return (IsBitOn(byte, 0x02) && IsBitOn(byte, 0x20));
}

/*********************************************************/

int IsBitOn(unsigned char byte, unsigned char bit)
{
	return (byte & bit);
}

/*********************************************************/

int IsTwoOrSixBitsOn(unsigned char byte)
{
	return (IsBitOn(byte, 0x02) || IsBitOn(byte, 0x20));
}

/*********************************************************/

unsigned char Swap3rd5th(unsigned char byte)
{
	return (SwapBits(byte, 3, 5));
}

/*********************************************************/

unsigned char SwapBits(unsigned char byte, size_t bit1, size_t bit2)
{
	unsigned char diff = ((byte >> bit1) ^ (byte >> bit2)) & 1;

	return (byte ^ (diff << bit1) ^ (diff << bit2));
}

/*********************************************************/

unsigned int SmallestDivisibleBy16(unsigned int num)
{
	return ((num >> 4) << 4);
}

/*********************************************************/

void SwapNoTmp(unsigned int *num1, unsigned int *num2)
{
	assert(NULL != num1);
	assert(NULL != num2);
	assert(num1 != num2);

	*num1 ^= *num2;
	*num2 ^= *num1;
	*num1 ^= *num2;
}

/*********************************************************/

size_t CountSetBitsLoop(int num)
{
	size_t count = 0;
	unsigned int bits = (unsigned int) num;

	for (; 0 != bits ; bits >>= 1)
	{
		if (1 & bits)
		{
			++count;
		}
	}
	
	return count;
}

/*********************************************************/
size_t CountSetBits(int n)
{
	n = ((n & 0x55555555)+((n >> 1) & 0x55555555));
	n = ((n & 0x33333333)+((n >> 2) & 0x33333333));
	n = ((n & 0x0F0F0F0F)+((n >> 4) & 0x0F0F0F0F));
	n = ((n & 0x00FF00FF)+((n >> 8) & 0x00FF00FF));
	n = ((n & 0x0000FFFF)+((n >> 16) & 0x0000FFFF));

	return ((size_t)n);
}

/*********************************************************/
void PrintFloatBits(float num)
{
	unsigned int bits = *(unsigned int*)&num;
	int i;

	for (i = sizeof(bits) * CHAR_BIT -1; i >= 0; --i)
	{
		printf("%d", (1 & (bits >> i)));
	}
	
	printf("\n");
}

/*********************************************************/







