/*********************************************************
*
* Description: Test runner for recursion exercises
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./recursion_test
*
*********************************************************/

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "recursion.h" /* Header file */
#include "stack.h" /* Stack header file */

#define SIGN(x) (((x) > 0) - ((x) < 0))

int FibonacciTest(void);
int FlipListTest(void);
int SortStackTest(void);
int SortStackTestStrings(void);
int RStrLenTest(void);
int RStrCmpTest(void);
int RStrCpyTest(void);
int RStrCatTest(void);
int RStrStrTest(void);


void print_result(char* func_name, int result)
{
	printf("Function: %-15s %s\n", func_name, (result ? "Passed" : "Failed"));
}

int main()
{
	printf("--- Running Recursion Tests ---\n\n");
	print_result("Fibonaccis", FibonacciTest());
	print_result("FlipList", FlipListTest());
	print_result("SortStack", SortStackTest());
	print_result("SortStackStrings", SortStackTestStrings());
	print_result("RStrLen", RStrLenTest());
	print_result("RStrCmp", RStrCmpTest());
	print_result("RStrCpy", RStrCpyTest());
	print_result("RStrCat", RStrCatTest());
	print_result("RStrStr", RStrStrTest());
	printf("\n------------------------------------\n");

	return 0;
}

int FibonacciTest(void)
{	
	return (
		Fibonacci(0) == 0 &&
		FibonacciRecursion(0) == 0 &&
		Fibonacci(1) == 1 &&
		FibonacciRecursion(1) == 1 &&
		Fibonacci(2) == 1 &&
		FibonacciRecursion(2) == 1 &&
		Fibonacci(3) == 2 &&
		FibonacciRecursion(3) == 2 &&
		Fibonacci(10) == 55 &&
		FibonacciRecursion(10) == 55 &&
		Fibonacci(20) == FibonacciRecursion(20) &&
		Fibonacci(46) == 1836311903
	);
}


int FlipListTest(void)
{
	node_t single_node = {1, NULL};
	node_t double_a = {1, NULL};
	node_t double_b = {2, NULL};
	node_t triple_a = {1, NULL};
	node_t triple_b = {2, NULL};
	node_t triple_c = {3, NULL};
	double_a.next = &double_b;
	triple_a.next = &triple_b;
	triple_b.next = &triple_c;

	return (
		FlipList(NULL) == NULL &&

		FlipList(&single_node) == &single_node &&
		single_node.next == NULL &&

		FlipList(&double_a) == &double_b &&
		double_b.next == &double_a &&
		double_a.next == NULL &&

		FlipList(&triple_a) == &triple_c &&
		triple_c.next == &triple_b &&
		triple_b.next == &triple_a &&
		triple_a.next == NULL
	);
}

int CompareInts(const void *data1, const void *data2)
{
	return (*(int*)data1 - *(int*)data2);
}

int SortStackTest(void)
{
	int a = 53, b = -17, c = 0; 
	int *pa = &a, *pb = &b, *pc = &c;
	int val1 = 0, val2 = 0, val3 = 0;

	stack_t *s_random = StackCreate(5, sizeof(void*)); 

	StackPush(s_random, &pc);
	StackPush(s_random, &pb);
	StackPush(s_random, &pa);

	SortStack(s_random, CompareInts);

	if (StackIsEmpty(s_random)) 
	{
		printf("\n[DEBUG] Error: Stack is completely empty after sort!\n");
		StackDestroy(s_random);
		
		return 0;
	}

	val1 = **(int**)StackPop(s_random);
	val2 = **(int**)StackPop(s_random);
	val3 = **(int**)StackPop(s_random);

	printf("\n--- Debug Info ---");
	printf("\nActual   (Top to Bottom): %d, %d, %d", val1, val2, val3);
	printf("\n------------------\n");

	StackDestroy(s_random);

	return (val1 == 53 && val2 == 0 && val3 == -17);
}

int CompareStrings(const void *data1, const void *data2)
{
	return strcmp((char*)data1, (char*)data2);
}

int SortStackTestStrings(void)
{
	char *a = "Apple";
	char *b = "Cherry";
	char *c = "Banana";

	char *val1 = NULL, *val2 = NULL, *val3 = NULL;

	stack_t *s_strings = StackCreate(5, sizeof(void*));

	StackPush(s_strings, &a);
	StackPush(s_strings, &b);
	StackPush(s_strings, &c);

	SortStack(s_strings, CompareStrings);

	if (StackIsEmpty(s_strings))
	{
		printf("\n[DEBUG] Error: Stack is completely empty after sort!\n");
		StackDestroy(s_strings);
		return 0;
	}

	val1 = *(char**)StackPop(s_strings);
	val2 = *(char**)StackPop(s_strings);
	val3 = *(char**)StackPop(s_strings);

	printf("\n--- String Debug Info ---");
	printf("\nExpected (Top to Bottom): Cherry, Banana, Apple");
	printf("\nActual   (Top to Bottom): %s, %s, %s", val1, val2, val3);
	printf("\n-------------------------\n");

	StackDestroy(s_strings);

	return (strcmp(val1, "Cherry") == 0 && 
			strcmp(val2, "Banana") == 0 && 
			strcmp(val3, "Apple") == 0);
}

int RStrLenTest(void)
{
	const char *reg = "InfinityLabs";
	const char *empty = "";
	const char *single = "A";

	return (
		RStrLen(reg) == strlen(reg) &&
		RStrLen(empty) == strlen(empty) &&
		RStrLen(single) == strlen(single)
	);
}

int RStrCmpTest(void)
{
	const char *s1 = "Infinity";
	const char *s2 = "Infinity";
	const char *s3 = "InfinitZ";
	const char *s4 = "Inf";
	const char *s5 = "";

	return (
		SIGN(RStrCmp(s1, s2)) == SIGN(strcmp(s1, s2)) &&

		SIGN(RStrCmp(s1, s3)) == SIGN(strcmp(s1, s3)) &&

		SIGN(RStrCmp(s1, s4)) == SIGN(strcmp(s1, s4)) &&
		SIGN(RStrCmp(s4, s1)) == SIGN(strcmp(s4, s1)) &&

		SIGN(RStrCmp(s1, s5)) == SIGN(strcmp(s1, s5)) &&
		SIGN(RStrCmp(s5, s1)) == SIGN(strcmp(s5, s1)) &&
		SIGN(RStrCmp(s5, s5)) == SIGN(strcmp(s5, s5))
	);
}

int RStrCpyTest(void)
{
	const char *src_reg = "MatrixTerminal";
	const char *src_empty = "";

	char my_dest1[50] = "XXXXXXXXX";
	char std_dest1[50] = "XXXXXXXXX";
	char my_dest2[50] = "YYYYY";
	char std_dest2[50] = "YYYYY";

	RStrCpy(my_dest1, src_reg);
	strcpy(std_dest1, src_reg);

	RStrCpy(my_dest2, src_empty);
	strcpy(std_dest2, src_empty);

	return (
		strcmp(my_dest1, std_dest1) == 0 &&
		strcmp(my_dest2, std_dest2) == 0
	);
}

int RStrCatTest(void)
{
    const char *append_str = "World!";
    const char *empty_str = "";
    
    char my_dest1[50] = "Hello ";
    char std_dest1[50] = "Hello ";
    
    char my_dest2[50] = "Infinity";
    char std_dest2[50] = "Infinity";

    RStrCat(my_dest1, append_str);
    strcat(std_dest1, append_str);

    RStrCat(my_dest2, empty_str);
    strcat(std_dest2, empty_str);

    return (
        strcmp(my_dest1, std_dest1) == 0 &&
        strcmp(my_dest2, std_dest2) == 0
    );
}

int RStrStrTest(void)
{
    const char *haystack = "MatrixTerminal";
    const char *empty = "";

    return (
        RStrStr(haystack, "Term") == strstr(haystack, "Term") &&
        RStrStr(haystack, "Mat") == strstr(haystack, "Mat") &&
        RStrStr(haystack, "nal") == strstr(haystack, "nal") &&
        RStrStr(haystack, "Neo") == strstr(haystack, "Neo") &&
        RStrStr(haystack, empty) == strstr(haystack, empty) &&
        RStrStr(empty, "Matrix") == strstr(empty, "Matrix") &&
        RStrStr(empty, empty) == strstr(empty, empty)
    );
}








