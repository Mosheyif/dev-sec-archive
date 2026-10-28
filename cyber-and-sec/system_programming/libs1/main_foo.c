/*********************************************************
*
* Description: Loads foo.so at runtime via dlopen and calls Foo()
* Author: Moshe Yifrah
*
* Compile with: gcc -o main_foo main_foo.c -ldl
*
* Run by: ./main_foo
*
*********************************************************/

#include <dlfcn.h>
int main()
{
void* handler = dlopen("dfoo.so", RTLD_LAZY);

typedef void (*FooFunc)(int);

FooFunc foo = (FooFunc)dlsym(handler, "Foo");
  foo(2);
  dlclose(handler);
  return 0;
}
