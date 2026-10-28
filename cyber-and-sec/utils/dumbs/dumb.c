/*********************************************************
*
* Description: Writes a pointer's address to file 'stoopid',
*              to be read back by dumber.c
* Author: Moshe Yifrah
*
* Compile with: gcc -o dumb dumb.c
*
* Run by: ./dumb
*
*********************************************************/

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
    int i = 7;
    int j = 12;
    int k = 15;
    int *p = &j;

    FILE *fp;

    if(0 == (fp = fopen("stoopid", "w")))
    {
        printf("well, that didn't work!\n");
        return -1;
    }

    fprintf(fp, "%p\n", p);

    if(fclose(fp))
    {
        printf("oh well.");
        return -1;
    }

    sleep(3);

    return 0;
}
