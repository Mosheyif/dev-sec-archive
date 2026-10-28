/*********************************************************
*
* Description: Implemention of memset, memcpy, memmove
* Author: Moshe Yifrah
*
*********************************************************/

#include <assert.h> /* assert */
#include <limits.h> /* CHAR_BIT */
#include <stddef.h> /* size_t, NULL */

#include "memFuncs.h"

/*********************************************************/

void *Memset(void *dst, int c, size_t n)
{
	unsigned char *dst_str = (unsigned char *) dst;
	size_t buffer = CreateBuffer((unsigned char) c);

	assert(NULL != dst);

	/* Aligning dst pointer when address is not divisible by pointer size */

	while (0 != ((size_t) dst_str & (WORD_BYTES - 1)) && 0 < n)
	{
		*dst_str++ = (unsigned char) c;
		--n;
	}

	/* Copying word-sized chunks for optimization */

	while (n >= WORD_BYTES)
	{
		*((size_t *) dst_str) = buffer;
		dst_str += WORD_BYTES;
		n -= WORD_BYTES;
	}

	/* Copying remaining trailing bytes */

	while (0 < n)
	{
		*dst_str++ = (unsigned char) c;
		--n;
	}

	return dst;
}

/*********************************************************/

void *Memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *src_str = (unsigned char *) src;
	unsigned char *dst_str = (unsigned char *) dst;

	assert(NULL != src_str);
	assert(NULL != dst_str);

	/* Aligning dst pointer when address is not divisible by pointer size */

	while (0 != ((size_t) dst_str & (WORD_BYTES - 1)) && 0 < n)
	{
		*dst_str++ = *src_str++;
		--n;
	}

	/* Copying word-sized chunks for optimization.
	Possible only if src address is also aligned */

	if (0 == ((size_t) src_str & (WORD_BYTES - 1)))
	{
		while (n >= WORD_BYTES)
		{
			*((size_t *) dst_str) = *((size_t *) src_str);
			dst_str += WORD_BYTES;
			src_str += WORD_BYTES;
			n -= WORD_BYTES;
		}
	}

	/* Copying remaining trailing bytes */

	while (0 < n)
	{
		*dst_str++ = *src_str++;
		--n;
	}

	return dst;
}

/*********************************************************/

void *Memmove(void *dst, const void *src, size_t n)
{
	unsigned char *src_str = (unsigned char *) src;
	unsigned char *dst_str = (unsigned char *) dst;

	assert(NULL != src_str);
	assert(NULL != dst_str);

	/* When no overlap exists, calling Memcpy */

	if (dst_str >= (src_str + n) || dst_str < src_str)
	{
		return Memcpy(dst, src, n);
	}

	/* Initializing reversed copying */

	src_str = src_str + n - 1;
	dst_str = dst_str + n - 1;

	/* Aligning dst pointer when address is not divisible by pointer size */

	while ((((size_t)(dst_str + 1)) & (WORD_BYTES - 1)) != 0 && 0 < n)
	{
		*dst_str-- = *src_str--;
		--n;
	}

	/* Copying word-sized chunks for optimization.
	Possible only if src address is also aligned */

	if (0 == (((size_t)(src_str + 1)) & (WORD_BYTES - 1)))
	{
		while (n >= WORD_BYTES)
		{
			*((size_t *) (dst_str - (WORD_BYTES - 1))) = *((size_t *) (src_str - (WORD_BYTES - 1)));

			dst_str -= WORD_BYTES;
			src_str -= WORD_BYTES;
			n -= WORD_BYTES;
		}
	}

	/* Copying remaining trailing bytes */

	while (0 < n)
	{
		*dst_str-- = *src_str--;
		--n;
	}

	return dst;
}

/*********************************************************/

size_t CreateBuffer(unsigned char c)
{
	size_t buffer = c;
	size_t size = 0;

	/* Broadcasting the byte across the entire word */

	for (size = CHAR_BIT; size < WORD_BITS; size *= 2)
	{
		buffer |= (buffer << size);
	}

	return buffer;
}

/*********************************************************/

int IsLittleEndian()
{
	unsigned int i = 1;

	/* Reading the byte at the lowest memory address */

	return *(char *) &i;
}

/*********************************************************/

