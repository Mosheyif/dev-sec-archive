/*********************************************************
*
* Description: Baseline for lib_mystery — calls strcpy without the
*              mystery library; compare with lib_loader.c output
* Author: Moshe Yifrah
*
* Compile with: gcc -o direct_strcpy direct_strcpy.c
*
* Run by: ./direct_strcpy
*
*********************************************************/

#include <stdio.h>
#include <string.h>

int main()
{
	char src[] = "12345678901234567890123456789012345678901234567890";
	char dst[64];

	strcpy(dst, src);
	puts(dst);

	return 0;
}
