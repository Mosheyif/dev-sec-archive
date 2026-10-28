/*********************************************************
*
* Description: Test runner for struct and union exercises
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./struct_test
*
*********************************************************/

#include <stdio.h> /* printf */

#include "struct.h"

int main()
{

    ManageStruct();
    return 0;
}

void print_result(char* func_name, int result)
{
    printf("Function: %-23s %s\n", func_name, (result ? "Passed" : "Failed"));
}
