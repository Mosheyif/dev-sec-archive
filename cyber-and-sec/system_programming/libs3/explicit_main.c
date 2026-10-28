/*********************************************************
*
* Description: Explicit linking — loads exbuild.so at runtime
*              via dlopen and calls library() to print memory layout
* Author: Moshe Yifrah
*
* Compile with: gcc -o explicit_main explicit_main.c -ldl
*
* Run by: ./explicit_main
*
*********************************************************/

#include <dlfcn.h>

void library(int, char * [], char * []);

typedef void (*Library)(int, char* [], char* []); 

int main (int argc, char *argv[], char *envp[])
{
  void* handler = dlopen("exbuild.so", RTLD_LAZY);

  Library foo = (Library)dlsym(handler, "library");
  
  foo(argc, argv, envp);
  
  dlclose(handler);

 return 0;
}
