/*********************************************************
*
* Description: Defines the global variable x and
*              PrintGlobalVariableAddress() shared across libs2 modules
* Author: Moshe Yifrah
*
* Compile with: gcc -shared -fPIC -o libdefs.so defs.c
*
*********************************************************/

#include <stdio.h>
#include "decs.h"

int x = 3;

void PrintGlobalVariableAddress()
{
  printf("Address of x: %p\n", &x);
}
