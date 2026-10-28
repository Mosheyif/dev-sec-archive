/*********************************************************
*
* Description: Dynamic library module — Foo2() prints global x's address
* Author: Moshe Yifrah
*
* Compile with: gcc -shared -fPIC -o dlib2.so dfoo2.c defs.c
*
*********************************************************/

#include "decs.h"

void Foo2()
{
  PrintGlobalVariableAddress();
}
