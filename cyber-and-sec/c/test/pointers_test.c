/*********************************************************
*
* Description: Test runner for pointer exercises
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./pointers_test
*
*********************************************************/

#include <stdio.h>
#include <string.h>

size_t josephus (size_t*, size_t);
void SumRows(size_t, size_t, int**, int*);
int SumRowsTest();
int JosephusTest();
void print_result(char*, int);


int SumRowsTest()
{
    int mat[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int* mat_ptrs[2] = {0}; 
    int res[2] = {0};
    
    
    mat_ptrs[0] = mat[0];
    mat_ptrs[1] = mat[1];
    SumRows(2, 3, mat_ptrs, res);
    
    return (
        res[0] == 6 &&
        res[1] == 15
    );
}

int JosephusTest()
{
    size_t arr5[5];
    size_t arr100[100];
    size_t arr41[41];
    
    return (
        josephus(arr5, 5) == 3 &&
        josephus(arr100, 100) == 73 &&
        josephus(arr41, 41) == 19
    );
}

void print_result(char* func_name, int result)
{
    printf("Function: %-15s %s\n", func_name, (result ? "Passed" : "Failed"));
}

int main()
{
    print_result("SumRows", SumRowsTest());
    print_result("Josephus", JosephusTest());
    printf("\n");
    
    return 0;
}
