/*********************************************************
*
* Description: Declarations for bitwise operations exercises
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef BITWISE_H
#define BITWISE_H

#include <stddef.h> /* size_t */ 

/* Implemention Functions ********************************/

long Pow2 (unsigned int, unsigned int);
int IsPowerOf2Loop(unsigned int);
int IsPowerOf2(unsigned int);
int AddOne(int);
void Print3Bits(unsigned int*, size_t);
unsigned char ByteMirrorLoop(unsigned char);
unsigned char ByteMirror(unsigned char);
int IsTwoAndSixBitsOn(unsigned char);
int IsTwoOrSixBitsOn(unsigned char);
unsigned char Swap3rd5th(unsigned char);
unsigned int SmallestDivisibleBy16(unsigned int);
void SwapNoTmp(unsigned int*, unsigned int*);
size_t CountSetBitsLoop(int);
size_t CountSetBits(int);
void PrintFloatBits(float);

/* Helping Functions *************************************/

unsigned long Multiply (unsigned long, unsigned long);
unsigned long Add (unsigned long, unsigned long);
int IsBitOn(unsigned char, unsigned char);
unsigned char SwapBits(unsigned char, size_t, size_t);

/* Testing Functions *************************************/

void print_result(char*, int);
int Pow2Test();
int IsPowerOf2LoopTest();
int IsPowerOf2Test();
int AddOneTest();
int ByteMirrorLoopTest();
int ByteMirrorTest();
int IsTwoAndSixBitsOnTest();
int IsTwoOrSixBitsOnTest();
int Swap3rd5thTest();
int SmallestDivisibleBy16Test();
int CountSetBitsLoopTest();
int CountSetBitsTest();

#endif /* BITWISE_H */

/*********************************************************/
