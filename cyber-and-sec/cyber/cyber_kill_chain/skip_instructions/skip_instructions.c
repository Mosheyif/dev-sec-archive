/*********************************************************
 *
 * Description: Patches the caller's return address via a local
 *              stack pointer to skip instructions in the caller
 * Author: Moshe Yifrah
 *
 * Compile with: gcc -o skip_instructions skip_instructions.c
 *
 * Run by: ./skip_instructions
 *
 *********************************************************/

#include <stdio.h>

void func(long a, long b)
{
	/* Access the return address via the saved frame pointer.
	 * x86-64 frame layout: [ ... | saved RBP | return addr | a | b ]
	 * __builtin_frame_address(0) gives the saved RBP value;
	 * one pointer above it is the return address.
	 * The +10 byte offset must match the size of `num = 4` in the
	 * caller — verify with objdump/gdb if the offset changes. */
	void **ret_addr = (void **)__builtin_frame_address(0) + 1;
	*ret_addr = (char *)*ret_addr + 10;
	return;

	printf("%ld\n", b);
	printf("%ld\n", a);
}

int main()
{
	int num = 3;
	func(3, 1);
	num = 4;
	printf("num is now %d\n", num);

	return 0;
}
