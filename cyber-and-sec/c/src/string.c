/*********************************************************
*
* Description: Implemention of string.h library
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdio.h> /* printf */
#include <stdlib.h> /* malloc, NULL, size_t */
#include <ctype.h> /* tolower */
#include <assert.h> /* assert */

#include "string.h"

/*********************************************************/

size_t StrLen(const char *s)
{
	size_t len = 0;

	assert(s != NULL);

	while (s[len++]);

	return len -1;
}

/*********************************************************/

int StrCmp(const char *s1, const char *s2)
{
	assert(s1 != NULL && s2 != NULL);

	while (*s1 == *s2 && *s1 != '\0')
	{
		++s1;
		++s2;
	}

	return (*(const unsigned char*)s1 - *(const unsigned char*)s2);
}

/*********************************************************/

char* StrCpy(char *dst, const char *src)
{
	char *dstPtr = dst;

	for (; *src != '\0'; src++ , dst++)
	{
		*dst = *src;
	}

	*dst = '\0';

	return dstPtr;
}

/*********************************************************/

char* StrNCpy(char *dst, const char *src, size_t dsize)
{
	char *dstPtr = dst;
	size_t i = 0;

	for (; i < dsize; i++)
	{
		*dst = *src;

		if (*src != '\0')
		{
			src++;
		}

		dst++;
	}

	return dstPtr;
}

/*********************************************************/

int StrNCmp(const char *s1, const char *s2, size_t n)
{
	size_t i = 0;

	for (; i < n && *s1 == *s2 && *s1 != '\0'; ++i)
	{
		if (i+1 < n)
		{
			++s1;
			++s2;
		}
	}

	return (*(const unsigned char*)s1 - *(const unsigned char*)s2);
}

/*********************************************************/

int StrCaseCmp(const char *s1, const char *s2)
{
	while ((tolower(*s1) == tolower(*s2)) && *s1 != '\0')
	{
		++s1;
		++s2;
	}

	return (tolower(*(const unsigned char*)s1) - tolower(*(const unsigned char*)s2));
}

/*********************************************************/

char* StrChr(const char *s , int c)
{
	while (1)
	{
		if (*s == (char)c)
		{
			return (char *)s;
		}

		if (*s == '\0')
		{
			return NULL;
		}

		s++;
	}
	
	return NULL;
}

/*********************************************************/

char* StrDup(const char *s)
{
	char *dup = (char*) malloc(StrLen(s) + 1);
	char* start = dup;

	if (dup == NULL)
	{
		return NULL;
	}

	while (1)
	{
		*dup = *s;

		if (*dup == '\0')
		{
			break;
		}

		s++;
		dup++;
	}
	
	return start;
}

/*********************************************************/

char* StrCat(char* dst, const char* src)
{
	StrCpy(dst+StrLen(dst), src);

	return dst;
}

/*********************************************************/

char* StrNCat(char* dst, const char* src, size_t n)
{
	char* start = dst;
	size_t i;

	while (*dst!= '\0') 
	{
		dst++;
	}

	for (i = 0; i < n && *src != '\0'; i++, src++, dst++) 
	{
		*dst = *src;
	}

	*dst = '\0';

	return start;
}

char* StrStr(const char *haystack, const char *needle)
{
	size_t needle_len = StrLen(needle); 

	if (0 == needle_len)
	{
		return (char*)haystack;
	}

	for (; *haystack != '\0'; haystack++)
	{
		if (*haystack == *needle && 0 == StrNCmp(haystack, needle, needle_len))
		{
			return (char*)haystack;
		}
	}

	return NULL;
}

/*********************************************************/

int IsExist(char c, const char* s)
{
	size_t i = 0;

	for (i = 0; s[i] != '\0'; i++)
	{
		if (s[i] == c)
		{
			return 1;
		}
	}
	
	return 0;
}

/*********************************************************/

size_t StrSpn(const char *s, const char *accept)
{
	size_t i = 0;

	for (i = 0; s[i] != '\0'; i++)
	{
		if (!IsExist(s[i], (const char *) accept))
		{
			break;
		}
	}
	
	return i;
}

/*********************************************************/

char* StrTok (char* str, const char* delim)
{
	char* start;
	char map[256] = {0}; 
	static char* saved;

	if (str) 
	{
		saved = str;
	}

	if (!saved) 
	{
		return NULL;
	}

	while (*delim) 
	{
		map[(unsigned char)*delim] = 1;
		delim++;
	}

	while (*saved && map[(unsigned char)*saved]) 
	{
		saved++;
	}

	if (*saved == '\0') 
	{
		return NULL;
	}

	start = saved;

	while (*saved && !map[(unsigned char)*saved])
	{
	saved++;
	}

	if (*saved != '\0') 
	{
		*saved = '\0';
		saved++;
	}
	else 
	{
		saved = NULL;
	}

	return start;
}

/*********************************************************/

int IsPalindrome(const char* str)
{
	const char* end = NULL;

	if (str == NULL)
	{
		return 0;
	}

	if (StrLen(str) <= 1)
	{
		return 1;
	}

	end = str + StrLen(str) -1;

	for (; str < end; str++, end--)
	{
		if (*str != *end)
		{
			return 0;
		}
	}
	
	return 1;
}

/*********************************************************/

int contains_7 (int num)
{
	for (; num != 0 ; num /= 10)
	{
		if (num % 10 == 7 || num % 10 == -7)
		{
			return 1;
		}
	}

	return 0;
}

/*********************************************************/

void _7Boom (int from, int to)
{
	int num = 0;

	for (num = from; num < to; num++)
	{
		if (num % 7 == 0 || contains_7(num))
		{
			printf("BOOM\n");
		}

		else
		{
			printf("%d\n", num);
		}
	}
}

/*********************************************************/

int IsSpace(int c)
{
	return (c == ' ' || c == '\t');
}

/*********************************************************/

char* SpaceReview (char* read)
{
	char* start = NULL;
	char* write = NULL;

	if (read == NULL)
	{
		return NULL;
	}

	start = write = read;

	while (IsSpace((unsigned char)*read))
	{
		++read;
	}

	while (*read)
	{
		if (!IsSpace((unsigned char)*read) || 
		(*(read+1) &&
		!IsSpace((unsigned char)*(read-1))))
		{
			*write = *read;
			++write;
		}

		++read;
	}

	if (IsSpace((unsigned char)*(write-1)))
	{
		--write;
	}
	
	*(write) = '\0';

	return start; 
}

/*********************************************************/
