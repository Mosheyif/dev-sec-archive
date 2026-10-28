/*********************************************************
*
* Description: Explicit linking demo — loads dlib1.so and dlib2.so
*              via dlopen and calls Foo/Foo2 from each
* Author: Moshe Yifrah
*
* Compile with: gcc -o main main.c -ldl
*
* Run by: ./main
*
*********************************************************/

#include <dlfcn.h>

void PrintGlobalVariableAddress();

typedef void (*FooFunc)();

int main()
{
  PrintGlobalVariableAddress();
  
  void* handler = dlopen("dlib1.so", RTLD_LAZY);
  void* handler2 = dlopen("dlib2.so", RTLD_LAZY);  

  FooFunc foo = (FooFunc)dlsym(handler, "Foo");
  FooFunc foo2 = (FooFunc)dlsym(handler2, "Foo2");  
  
  foo();
  foo2();
  
  dlclose(handler);
  dlclose(handler2);
  
  return 0;
}
