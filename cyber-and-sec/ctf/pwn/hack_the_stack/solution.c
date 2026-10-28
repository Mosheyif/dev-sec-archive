/*********************************************************
*
* Description: Solution for hack_the_stack — overwrites const x,
*              accesses x via y's stack address, and skips a printf
*              by advancing func2's return address by 16 bytes
* Author: Moshe Yifrah
*
* Compile with: gcc -fno-stack-protector -o hack_the_stack solution.c
*
* Run by: ./hack_the_stack
*
*********************************************************/

#include <stdio.h>

void func2(void *arg)
{
	long f2;

	printf("Enter func2\n");

	/* Q3: f2 sits at rbp-0x8; the saved return address is at rbp+0x8
	   (&f2 + 2 qwords). Adding 16 skips the next printf call in func1. */
	*(&f2 + 2) += 16;

	printf("Exit func2\n");
}

void func1()
{
	long f1;
	printf("Enter func1\n");
	func2(&f1);
	printf("Exit func1\n"); /* skipped by Q3 */
}

int main()
{
	const long x = 1;
	long y = 5;

	/* Q1: cast away const and overwrite x directly via pointer */
	long *z = (long *)&x;
	*z = 1337;
	printf("x=%lu\n", x); /* → 1337 */

	/* Q2: x lives one qword above y on the stack (stack grows down,
	   so x was pushed first at the higher address y+1) */
	*((long *)&y + 1) = 1337;
	printf("x=%lu\n", x); /* → 1337 */

	func1();

	printf("Exit main\n");
	return 0;
}
