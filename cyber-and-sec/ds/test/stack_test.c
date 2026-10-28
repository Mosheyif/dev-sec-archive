/*********************************************************
*
* Description: Test runner for the stack data structure
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./stack_test
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

#define TEST(name, condition) \
    do { \
        if (condition) { \
            printf("[V] PASS: %s\n", name); \
        } else { \
            printf("[X] FAIL: %s\n", name); \
        } \
    } while(0)

int main() 
{
    size_t capacity = 5;
    size_t element_size = sizeof(int);
    stack_t* stack = NULL;
    int val1 = 10, val2 = 20, val3 = 30, val4 = 40, val5 = 50;
    int* peeked = NULL;
    int* popped = NULL;

    stack = StackCreate(capacity, element_size);

    printf("--- Starting Stack Implementation Tests ---\n\n");

    TEST("StackCreate - Allocation successful", stack != NULL);
    TEST("StackIsEmpty - Initially empty", StackIsEmpty(stack) == 1);
    TEST("StackSize - Initially 0", StackSize(stack) == 0);
    TEST("StackCapacity - Matches initial setup", StackCapacity(stack) == 5);

    printf("\n--- Testing Push & Peek ---\n");
    
    StackPush(stack, &val1);
    TEST("StackPush - Size increased to 1", StackSize(stack) == 1);
    TEST("StackIsEmpty - Not empty after push", StackIsEmpty(stack) == 0);

    peeked = (int*)StackPeek(stack);
    TEST("StackPeek - Matches pushed value (10)", *peeked == 10);

    printf("\n--- Testing Edge Case: Full Capacity ---\n");
    StackPush(stack, &val2);
    StackPush(stack, &val3);
    StackPush(stack, &val4);
    StackPush(stack, &val5);
    TEST("StackPush - Reached full capacity (5)", StackSize(stack) == capacity);
    
    peeked = (int*)StackPeek(stack);
    TEST("StackPeek - Matches top element (50)", *peeked == 50);

    printf("\n--- Testing Pop ---\n");
    
    popped = (int*)StackPop(stack);
    TEST("StackPop - Returns last pushed element (50)", *popped == 50);
    TEST("StackSize - Decreased after pop (4)", StackSize(stack) == 4);

    printf("\n--- Testing Edge Case: Pop to Empty ---\n");
    StackPop(stack);
    StackPop(stack);
    StackPop(stack);
    
    popped = (int*)StackPop(stack);
    TEST("StackPop - Returns first element (10)", *popped == 10);
    TEST("StackSize - Size is 0 after popping all", StackSize(stack) == 0);
    TEST("StackIsEmpty - Empty after popping all", StackIsEmpty(stack) == 1);

    StackDestroy(stack);
    printf("\n--- Tests finished successfully. Don't forget Valgrind! ---\n");
    
    return 0;
}
