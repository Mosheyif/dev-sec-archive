/*********************************************************
*
* Description: Overrides getenv() to trigger SIGSEGV on any call,
*              crashing the target binary via LD_PRELOAD injection
* Author: Moshe Yifrah
*
* Compile with: gcc -shared -fPIC hijack.c -o hijack.so
*
* Run by: LD_PRELOAD=./hijack.so ./make_this_crash
*
*********************************************************/

#include <stddef.h>

/* Overriding the standard getenv function */
char *getenv(const char *name) {
    int *ptr = NULL;
    *ptr = 1337; /* Triggers a Segmentation Fault immediately */
    return NULL;
}
