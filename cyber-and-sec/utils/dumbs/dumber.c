/*********************************************************
*
* Description: Reads a pointer address from file 'stoopid'
*              (written by dumb.c) and dereferences it
* Author: Moshe Yifrah
*
* Compile with: gcc -o dumber dumber.c
*
* Run by: Run dumb first, then ./dumber
*
*********************************************************/

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
int i = 8;
int j = 16;
int k = 32;
int *p = &j;

FILE *fp;

if(0 == (fp = fopen("stoopid", "r")))
{
    printf("well, that didn't work!\n");
    return -1;
}

fscanf(fp, "%p\n", &p);

if(fclose(fp))
{
    printf("oh well.");
    return -1;
}

printf("p points to: %d\n", *p);

return 0;
}
