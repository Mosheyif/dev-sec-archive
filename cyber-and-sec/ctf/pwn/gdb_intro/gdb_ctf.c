/*********************************************************
*
* Description: GDB intro challenge — obfuscated pointer arithmetic
*              hides values written into array 'a'; use GDB to
*              inspect memory and determine what secret() returns
* Author: Moshe Yifrah
*
* Compile with: gcc -o gdb_ctf gdb_ctf.c
*
* Run by: ./gdb_ctf <your_id>
*
*********************************************************/

#include <stdio.h>

char *secret(char *param);

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		puts("Please insert your ID");
		return 1;
	}
	size_t a[3] = {0};

	*(size_t *)((char *)a + 1) = *(size_t *)secret(argv[1]);
	*((size_t *)((char *)a) + 1) = 2314885686279031104; /* 0x2020202048454c4c */
	*(void **)a = NULL;
	*((void **)a + 1) = NULL;
	*((void **)a + 2) = NULL;

	return 0;
}
