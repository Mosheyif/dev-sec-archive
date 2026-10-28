/*********************************************************
*
* Description: Declarations for custom string library exercises
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef __STRING_H__
#define __STRING_H__

/* Includes ******************************************************/

#include <stddef.h> /* size_t */

/* String Manipulation *******************************************/

size_t StrLen(const char *s);
int StrCmp(const char *s1, const char *s2);
char* StrCpy(char *dst, const char *src);
char* StrNCpy(char *dst, const char *src, size_t dsize);
int StrNCmp(const char *s1, const char *s2, size_t n);
int StrCaseCmp(const char *s1, const char *s2);
char* StrCat(char* dst, const char* src);
char* StrNCat(char* dst, const char* src, size_t n);
char* StrDup(const char *s);
char* StrTok(char* str, const char* delim);
char* SpaceReview(char* read);

/* Search & Character Utils **************************************/

char* StrChr(const char *s, int c);
char* StrStr(const char *haystack, const char *needle);
int IsExist(char c, const char* s);
size_t StrSpn(const char *s, const char *accept);
int IsPalindrome(const char* str);
int IsSpace(int c);

/* Math & Utility ************************************************/

int contains_7(int num);
void _7Boom(int from, int to);

#endif /* STRING_H */
