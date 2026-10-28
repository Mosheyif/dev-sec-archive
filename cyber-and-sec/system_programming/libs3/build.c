/*********************************************************
*
* Description: Memory layout demo — prints addresses of all storage
*              classes: text, data, BSS, rodata, heap, stack, env
* Author: Moshe Yifrah
*
* Compile with: gcc -shared -fPIC -o exbuild.so build.c
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
extern void ExternFunction() {}

static void StaticFunction() {}

int global_val;
const int const_global_val;
int static_global_val;
const int const_static_global_val;

void library(int argc, char *argv[], char *envp[]) 
{
    int* heap_var = malloc(10);
    const int* const_heap_var = malloc(10);
    
    int local_var = 0;
    const int const_local_var = 0;
    
    static int static_local_var;
    const static int const_static_local_var;
    
    char* string_literal = "string";
    const char* const_string_literal = "const_string";
    
    printf("=== Functions (Text Segment) ===\n");
    printf("ExternFunction:          %p\n", (void*)ExternFunction);
    printf("StaticFunction:          %p\n", (void*)StaticFunction);
    
    printf("\n=== Globals / Statics (BSS / Data / RODATA) ===\n");
    printf("global_val:              %p\n", (void*)&global_val);
    printf("const_global_val:        %p\n", (void*)&const_global_val);
    printf("static_global_val:       %p\n", (void*)&static_global_val);
    printf("const_static_global_val: %p\n", (void*)&const_static_global_val);
    printf("static_local_var:        %p\n", (void*)&static_local_var);
    printf("const_static_local_var:  %p\n", (void*)&const_static_local_var);
    
    printf("\n=== String Literals (RODATA Segment) ===\n");
    printf("string_literal:          %p\n", (void*)string_literal);
    printf("const_string_literal:    %p\n", (void*)const_string_literal);
    
    printf("\n=== Heap (Dynamic Memory) ===\n");
    printf("heap_var (data addr):    %p\n", (void*)heap_var);
    printf("const_heap_var (data):   %p\n", (void*)const_heap_var);
    
    printf("\n=== Stack (Local Variables) ===\n");
    printf("local_var:               %p\n", (void*)&local_var);
    printf("const_local_var:         %p\n", (void*)&const_local_var);
    printf("heap_var (pointer addr): %p\n", (void*)&heap_var);
    
    printf("\n=== CLI Args & Environment Variables (High Memory) ===\n");
    printf("argv array address:      %p\n", (void*)argv);
    
    if (argc > 0) {
        printf("argv[0] (CLI arg):       %p\n", (void*)argv[0]); 
    }
    
    if (envp[0] != NULL) {
        printf("envp[0] (Env Var):       %p\n", (void*)envp[0]);
    }

    free(heap_var);
    free((void*)const_heap_var);
    
    getchar();
}
