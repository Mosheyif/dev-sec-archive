/*********************************************************
*
* Description: Vulnerable program using srand(time(0)) for
*              password generation, exploitable via timing attack
* Author: Moshe Yifrah
*
* Compile with: gcc -o vuln vuln.c
*
* Run by: ./vuln
*
*********************************************************/

#include <stdio.h> /* printf */
#include <stdlib.h> /* malloc */
#include <time.h> /* time */
#include <string.h> /* strncpy */

#define PASS_SIZE (20)
int main () {
    char *password = (char*)malloc(PASS_SIZE);
    char user_buf[PASS_SIZE+1];
    int i;

    srand(time(0));

    for (i=0; i<PASS_SIZE; i++)
    {
        password[i] = 'A' + rand()%('z' - 'A' + 1);
    }

    while (1)
    {
        fgets(user_buf, PASS_SIZE+1, stdin);

        if (strncmp(user_buf, password, PASS_SIZE) == 0)
        {
            printf("password correct!!\n");
            break;
            /*secret();*/
        }
        else
        {
            printf(":-(\n");
        }
    }

    return(0);
}
