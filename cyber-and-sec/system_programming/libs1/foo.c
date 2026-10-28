/*********************************************************
*
* Description: Shared library example — Foo() prints an integer
* Author: Moshe Yifrah
*
* Compile with: gcc -shared -fPIC -o foo.so foo.c
*
*********************************************************/

#include <stdio.h>

void Foo(int num)
{
  printf("%d\n", num);
}
