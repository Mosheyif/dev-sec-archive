/*********************************************************
*
* Description: Test runner for custom string library
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./string_test
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

#include "string.h"

void print_result(char* func_name, int result)
{
    printf("Function: %-15s %s\n", func_name, (result ? "Passed" : "Failed"));
}

int StrLenTest()
{
    return (
        strlen("") == StrLen("") &&
        strlen("abcde") == StrLen("abcde") &&
        StrLen("hello\0world") == 5
    );
}

int StrCmpTest()
{
    return (
        StrCmp("abc", "abc") == strcmp("abc", "abc") &&
        StrCmp("abc", "abd") == strcmp("abc", "abd") &&
        StrCmp("abd", "abc") > 0 &&
        StrCmp("A", "a") < 0
    );
}

int StrCpyTest()
{
    char buf1[20] = {0};
    char buf2[20] = {0};
    StrCpy(buf1, "hello");
    strcpy(buf2, "hello");
    return (strcmp(buf1, buf2) == 0);
}

int StrNCpyTest()
{
    char buf[20] = {0};
    StrNCpy(buf, "hello", 3);
    return (buf[0] == 'h' && buf[1] == 'e' && buf[2] == 'l' && buf[3] == '\0');
}

int StrNCmpTest()
{
    return (
        StrNCmp("abc", "abc", 3) == 0 &&
        StrNCmp("abc", "abd", 2) == 0 &&
        StrNCmp("abc", "abd", 3) < 0 &&
        StrNCmp("abcd", "abc", 4) > 0
    );
}

int StrCaseCmpTest()
{
    return (
        StrCaseCmp("aBc", "abc") == 0 &&
        StrCaseCmp("HELLO", "hello") == 0 &&
        StrCaseCmp("a", "B") < 0 &&
        StrCaseCmp("Z", "a") > 0
    );
}

int StrChrTest()
{
    const char* str = "hello";
    return (
        StrChr(str, 'e') == str + 1 &&
        StrChr(str, 'l') == str + 2 &&
        StrChr(str, 'z') == NULL &&
        StrChr(str, '\0') == str + 5
    );
}

int StrDupTest()
{
    char* d1 = StrDup("hello");
    char* d2 = StrDup("");
    int res = (d1 != NULL && strcmp(d1, "hello") == 0 && 
               d2 != NULL && strcmp(d2, "") == 0);
    free(d1);
    free(d2);
    return res;
}

int StrCatTest()
{
    char buf[20] = "hello";
    StrCat(buf, " world");
    return (strcmp(buf, "hello world") == 0);
}

int StrNCatTest()
{
    char buf[20] = "hello";
    StrNCat(buf, " world", 3);
    return (strcmp(buf, "hello wo") == 0);
}

int StrStrTest()
{
    const char* haystack = "hello world";
    return (
        StrStr(haystack, "world") == haystack + 6 &&
        StrStr(haystack, "hello") == haystack &&
        StrStr(haystack, "xyz") == NULL
    );
}

int StrSpnTest()
{
    return (
        StrSpn("12345abc", "1234567890") == 5 &&
        StrSpn("abc", "123") == 0 &&
        StrSpn("Hello", "oelH") == strspn("Hello", "oelH")
    );
}

int StrTokTest()
{
    char str[] = "a,b,c";
    char* t1 = StrTok(str, ",");
    char* t2 = StrTok(NULL, ",");
    char* t3 = StrTok(NULL, ",");
    char* t4 = StrTok(NULL, ",");
    
    return (
        t1 != NULL && strcmp(t1, "a") == 0 &&
        t2 != NULL && strcmp(t2, "b") == 0 &&
        t3 != NULL && strcmp(t3, "c") == 0 &&
        t4 == NULL
    );
}

int IsPalindromeTest()
{
    return (
        IsPalindrome("racecar") == 1 &&
        IsPalindrome("abba") == 1 &&
        IsPalindrome("hello") == 0 &&
        IsPalindrome("a") == 1 &&
        IsPalindrome("") == 1
    );
}

int SpaceReviewTest()
{
    char s1[] = "  hello	   world  	";
    char s2[] = "nospaces";
    char s3[] = "   ";
    
    SpaceReview(s1);
    SpaceReview(s2);
    SpaceReview(s3);
    
    return (
        strcmp(s1, "hello	world") == 0 &&
        strcmp(s2, "nospaces") == 0 &&
        strcmp(s3, "") == 0
    );
}

int main()
{
    printf("--- Running String Tests ---\n\n");
    print_result("StrLen", StrLenTest());
    print_result("StrCmp", StrCmpTest());
    print_result("StrCpy", StrCpyTest());
    print_result("StrNCpy", StrNCpyTest());
    print_result("StrNCmp", StrNCmpTest());
    print_result("StrCaseCmp", StrCaseCmpTest());
    print_result("StrChr", StrChrTest());
    print_result("StrDup", StrDupTest());
    print_result("StrCat", StrCatTest());
    print_result("StrNCat", StrNCatTest());
    print_result("StrStr", StrStrTest());
    print_result("StrSpn", StrSpnTest());
    print_result("StrTok", StrTokTest());
    print_result("IsPalindrome", IsPalindromeTest());
    print_result("SpaceReview", SpaceReviewTest());
    printf("\n------------------------------------\n");
    
    return 0;
}
