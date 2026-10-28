/*********************************************************
*
* Description: Static linking — links exbuild.a at compile time
*              and calls library() to print memory layout
* Author: Moshe Yifrah
*
* Compile with: gcc -o static_main static_main.c exbuild.a
*
* Run by: ./static_main
*
*********************************************************/

void library(int, char * [], char * []);

int main (int argc, char *argv[], char *envp[])
{
  library(argc, argv, envp);

 return 0;
}
