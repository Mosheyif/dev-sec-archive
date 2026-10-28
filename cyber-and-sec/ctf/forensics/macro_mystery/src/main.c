/*********************************************************
*
* Description: Macro obfuscation CTF — the flag is hidden in a
*              #define chain across char_macros.h, flag_assembly.h
*              and print_macro.h; trace bijruztufnjq to recover it
* Author: Moshe Yifrah
*
* Compile with: gcc -o main main.c
*
* Run by: ./main
*
*********************************************************/

#include "print_macro.h"

int main()
{
	PRINT(bijfddsfdsruztufnjq); /* garbled — find the correct macro name */
	return 0;
}

/* Answer: bijruztufnjq  →  ILRD{&iT~Br0$8,xgc<?@} */