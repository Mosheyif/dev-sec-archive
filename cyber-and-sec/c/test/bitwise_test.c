/*********************************************************
*
* Description: Test runner for bitwise operations
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./bitwise_test
*
*********************************************************/

#include <stdio.h> /* printf */
#include <limits.h> /* INT_MAX , INT_MIN */

#include "bitwise.h"

/*********************************************************/

int main()
{
	unsigned int arr[] = {0, 3, 7, 8, 11, 15, 19, 32, 42};

	print_result("Pow2", Pow2Test());
	print_result("IsPowerOf2Loop", IsPowerOf2LoopTest());
	print_result("IsPowerOf2", IsPowerOf2Test());
	print_result("AddOne", AddOneTest());
	printf("Function: Print3Bits\nResult: ");
	Print3Bits(arr, 9);
	print_result("ByteMirrorLoop", ByteMirrorLoopTest());
	print_result("ByteMirror", ByteMirrorTest());
	print_result("IsTwoAndSixBitsOn", IsTwoAndSixBitsOnTest());
	print_result("IsTwoOrSixBitsOn", IsTwoOrSixBitsOnTest());
	print_result("Swap3rd5th", Swap3rd5thTest());
	print_result("SmallestDivisibleBy16", SmallestDivisibleBy16Test());    
	print_result("SwapNoTmp", 1);      
	print_result("CountSetBitsLoop", CountSetBitsLoopTest());          
	print_result("CountSetBits", CountSetBitsTest());    
	printf("Function: PrintFloatBits\nResult: ");
	PrintFloatBits(10.5);
	
	return 0;
}

/*********************************************************/

void print_result(char* func_name, int result)
{
	printf("Function: %-23s %s\n", func_name, (result ? "Passed" : "Failed"));
}

/*********************************************************/

int Pow2Test()
{
	return (
		Pow2(0, 5) == 0 &&
		Pow2(10, 0) == 10 &&
		Pow2(0, 0) == 0 &&
		Pow2(1, 31) == (long)(1UL << 31) &&
		Pow2(5, 2) == 20 &&
		Pow2(5, 30) == (long)(5UL * (1UL << 30)) &&
		Pow2(UINT_MAX, 31) == (long)((unsigned long)UINT_MAX * (1UL << 31)) /* Maximum input possible */
	);
}

/*********************************************************/

int IsPowerOf2LoopTest()
{
	return (
		IsPowerOf2Loop(0) == 0 &&
		IsPowerOf2Loop(1) == 1 &&
		IsPowerOf2Loop(2) == 1 &&
		IsPowerOf2Loop(16) == 1 &&
		IsPowerOf2Loop(3) == 0 &&
		IsPowerOf2Loop(14) == 0 &&
		IsPowerOf2Loop(1023) == 0 &&
		IsPowerOf2Loop(1U << 31) == 1 &&
		IsPowerOf2Loop(UINT_MAX) == 0
	);
}

/*********************************************************/

int IsPowerOf2Test()
{
	return (
		IsPowerOf2(0) == 0 &&
		IsPowerOf2(1) == 1 &&
		IsPowerOf2(2) == 1 &&
		IsPowerOf2(16) == 1 &&
		IsPowerOf2(3) == 0 &&
		IsPowerOf2(14) == 0 &&
		IsPowerOf2(1023) == 0 &&
		IsPowerOf2(1U << 31) == 1 &&
		IsPowerOf2(UINT_MAX) == 0
	);
}

/*********************************************************/

int AddOneTest()
{
	return (
		AddOne(0) == 1 &&
		AddOne(1) == 2 &&
		AddOne(5) == 6 &&
		AddOne(15) == 16 &&
		AddOne(127) == 128 &&
		AddOne(-5) == -4 &&
		AddOne(-1) == 0 &&
		AddOne(INT_MAX) == INT_MIN &&
		AddOne(INT_MIN) == (INT_MIN + 1)
	);
}

/*********************************************************/

int ByteMirrorLoopTest()
{
	return (
		ByteMirrorLoop(0)   == 0   &&
		ByteMirrorLoop(1)   == 128 &&
		ByteMirrorLoop(128) == 1   &&
		ByteMirrorLoop(170) == 85  &&
		ByteMirrorLoop(240) == 15  &&
		ByteMirrorLoop(198) == 99  &&
		ByteMirrorLoop(129) == 129 &&
		ByteMirrorLoop(255) == 255
	);
}

/*********************************************************/

int ByteMirrorTest()
{
	return (
		ByteMirror(0)   == 0   &&
		ByteMirror(1)   == 128 &&
		ByteMirror(128) == 1   &&
		ByteMirror(170) == 85  &&
		ByteMirror(240) == 15  &&
		ByteMirror(198) == 99  &&
		ByteMirror(129) == 129 &&
		ByteMirror(255) == 255
	);
}

/*********************************************************/

int IsTwoAndSixBitsOnTest()
{
	return (
		IsTwoAndSixBitsOn(0x22) == 1 &&
		IsTwoAndSixBitsOn(0xFF) == 1 &&
		IsTwoAndSixBitsOn(0x02) == 0 &&
		IsTwoAndSixBitsOn(0x20) == 0 &&
		IsTwoAndSixBitsOn(0x00) == 0 &&
		IsTwoAndSixBitsOn(0xDD) == 0
	);
}

/*********************************************************/

int IsTwoOrSixBitsOnTest()
{
	return (
		IsTwoOrSixBitsOn(0x22) == 1 &&
		IsTwoOrSixBitsOn(0x02) == 1 &&
		IsTwoOrSixBitsOn(0x20) == 1 &&
		IsTwoOrSixBitsOn(0xFF) == 1 &&
		IsTwoOrSixBitsOn(0x00) == 0 &&
		IsTwoOrSixBitsOn(0xDD) == 0
	);
}

/*********************************************************/

int Swap3rd5thTest()
{
	return (
		Swap3rd5th(0x00) == 0x00 &&
		Swap3rd5th(0x08) == 0x20 &&
		Swap3rd5th(0x20) == 0x08 &&
		Swap3rd5th(0x28) == 0x28 &&
		Swap3rd5th(0xFF) == 0xFF &&
		Swap3rd5th(0xF7) == 0xDF &&
		Swap3rd5th(0xDF) == 0xF7 &&
		Swap3rd5th(0x18) == 0x30 &&
		Swap3rd5th(0x30) == 0x18
	);
}

/*********************************************************/

int SmallestDivisibleBy16Test()
{
	return (
		SmallestDivisibleBy16(0) == 0 &&
		SmallestDivisibleBy16(15) == 0 &&
		SmallestDivisibleBy16(16) == 16 &&
		SmallestDivisibleBy16(17) == 16 &&
		SmallestDivisibleBy16(31) == 16 &&
		SmallestDivisibleBy16(32) == 32 &&
		SmallestDivisibleBy16(255) == 240 &&
		SmallestDivisibleBy16(256) == 256
	);
}

/*********************************************************/

int CountSetBitsLoopTest()
{
	return (
		CountSetBitsLoop(0) == 0 &&
		CountSetBitsLoop(1) == 1 &&
		CountSetBitsLoop(2) == 1 &&
		CountSetBitsLoop(3) == 2 &&
		CountSetBitsLoop(15) == 4 &&
		CountSetBitsLoop(0xFF) == 8 &&
		CountSetBitsLoop(0x0F0F0F0F) == 16 &&
		CountSetBitsLoop(~0) == (sizeof(int) * 8) &&
		CountSetBitsLoop(-1) == (sizeof(int) * 8) &&
		CountSetBitsLoop(0x80000000) == 1
	);
}

/*********************************************************/

int CountSetBitsTest()
{
	return (
		CountSetBits(0) == 0 &&
		CountSetBits(1) == 1 &&
		CountSetBits(2) == 1 &&
		CountSetBits(3) == 2 &&
		CountSetBits(15) == 4 &&
		CountSetBits(0xFF) == 8 &&
		CountSetBits(0x0F0F0F0F) == 16 &&
		CountSetBits(~0) == (sizeof(int) * 8) &&
		CountSetBits(-1) == (sizeof(int) * 8) &&
		CountSetBits(0x80000000) == 1
	);
}

/*********************************************************/

















