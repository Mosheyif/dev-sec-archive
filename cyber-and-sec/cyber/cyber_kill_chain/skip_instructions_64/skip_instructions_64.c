/*********************************************************
*
* Description: 64-bit ROP-based stack pivot that hijacks
*              control flow to call finish() then restore state
* Author: Moshe Yifrah
*
* Compile with: gcc -o skip_instructions_64 skip_instructions_64.c
*
* Run by: ./skip_instructions_64
*
*********************************************************/

#include <stdio.h>

void finish(int a, int b) {
  if (42 == a) {
    printf("That's an interesting value for a.\n");
  }
  if (0 == b) {
    printf("How mundane.\n");
  }
  if (32767 == a * b) {
    printf("Keep in mind that neither success not failure is ever final.\n");
  }
  if (8001 == a * b || 9001 == a * b) {
    printf("Realize that while the first 90 percent of code accounts for 90 "
           "percent of the development time, so does the last 10 percent of "
           "the code.");
  }

  printf("You've successfully finished this part of the exercise.\n");
}


void func() {

	// Add code here
	
	register void **rbp asm("rbp");

	/* 1. Allocate the fake stack locally inside func() to obey the rules */
	void *fake_stack[1024];

	/* 2. Harvest the caller's hardware state safely */
	void *orig_rbp = (void *)((volatile void **)rbp)[0]; 
	void *orig_ret = (void *)((volatile void **)rbp)[1]; 
	void *orig_rsp = (void *)(rbp + 2); 

	void *pivot_gadget;
	void *restore_gadget;

	/* Point to the middle of the local fake stack to allow downward growth */
	void **pivot_point = &fake_stack[512]; 

	/* 3. Generate pure ROP gadgets inline */
	__asm__ volatile (
		"lea 1f(%%rip), %0\n\t"
		"jmp 2f\n\t"
		
		/* GADGET A: Pivot via smuggled RBP */
		"1: mov %%rbp, %%rsp\n\t"  
		"ret\n\t"               
		
		"2:\n\t"
		"lea 3f(%%rip), %1\n\t"
		"jmp 4f\n\t"
		
		/* GADGET B: Perfect Hardware Restoration */
		"3: pop %%rax\n\t"      // Get orig_rsp
		"pop %%rcx\n\t"         // Get orig_ret
		"pop %%rbp\n\t"         // Get orig_rbp
		"mov %%rax, %%rsp\n\t"  // Snap real stack back
		"push %%rcx\n\t"        // Plant orig_ret back on stack
		"ret\n\t"               // Return gracefully
		"4:"
		: "=r" (pivot_gadget), "=r" (restore_gadget)
	);

	/* 4. Build pure ROP chain on the local fake stack */
	pivot_point[0] = (void *)finish;
	pivot_point[1] = restore_gadget;
	pivot_point[2] = orig_rsp;
	pivot_point[3] = orig_ret;
	pivot_point[4] = orig_rbp;

	/* 5. FORCE the compiler to commit the hijack (Bypasses DSE) */
	((volatile void **)rbp)[0] = pivot_point;
	((volatile void **)rbp)[1] = pivot_gadget;
	
	// End of solution
}

int main() {
  func();
  return 0;
}
