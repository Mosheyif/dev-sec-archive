/*********************************************************
*
* Description: Stack manipulation challenge — fill AREA1 and AREA2
*              to overwrite const x and modify a return address
* Author: Moshe Yifrah
*
* Compile with: gcc -fno-stack-protector -o hack_the_stack challenge.c
*
* Run by: ./hack_the_stack
*
*********************************************************/

#include <stdio.h>

void func2(void *arg)
{
	long f2;

	printf("Enter func2\n");

	/* AREA2 */
	/* Q3: Advance func2's return address by 16 bytes to skip
	   printf("Exit func1") in the caller. */

	printf("Exit func2\n");
}

void func1()
{
	long f1;
	printf("Enter func1\n");
	func2(&f1);
	printf("Exit func1\n");
}

int main()
{
	const long x = 1;
	long y = 5;

	/* AREA1 */
	/* Q1: Change x to 1337 using a pointer to x.              */
	/* Q2: Change x to 1337 without referencing x directly —   */
	/*     use y's address and the known stack layout.          */

	printf("x=%lu\n", x);

	func1();

	printf("Exit main\n");
	return 0;
}
