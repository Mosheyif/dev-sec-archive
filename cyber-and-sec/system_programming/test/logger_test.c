/*********************************************************
*
* Description: Test runner for the logger module
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./logger_test
*
*********************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int Logger(char*);
void Verify(int);

int main ()
{
   char* file_name = NULL;
   size_t size = 0;
   
   printf("Enter file name\n");
   
   Verify (getline(&file_name, &size, stdin) <= 1);
   
   file_name[strcspn(file_name, "\n")] = '\0';
      
   Verify(Logger(file_name));
      
   free(file_name);
   file_name = NULL;
   
return 0;
}
