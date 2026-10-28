/*********************************************************
*
* Description: Implicit linking — links exbuild.so at compile time
*              and calls library() to print memory layout
* Author: Moshe Yifrah
*
* Compile with: gcc -o implicit_main implicit_main.c -L. -lexbuild
*
* Run by: LD_LIBRARY_PATH=. ./implicit_main
*
*********************************************************/

void library(int, char * [], char * []);

int main (int argc, char *argv[], char *envp[])
{
  library(argc, argv, envp);

 return 0;
}
