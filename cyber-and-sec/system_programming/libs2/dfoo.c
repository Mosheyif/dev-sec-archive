/*********************************************************
*
* Description: Dynamic library module — Foo() prints global x's address
* Author: Moshe Yifrah
*
* Compile with: gcc -shared -fPIC -o dlib1.so dfoo.c defs.c
*
*********************************************************/

#include "decs.h"

void Foo()
{
  PrintGlobalVariableAddress();
}
