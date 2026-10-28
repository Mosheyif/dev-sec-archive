/*********************************************************
*
* Description: Test runner for custom memory utility functions
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./memFuncs_test
*
*********************************************************/

#include <stdio.h>
#include <string.h>

#include "memFuncs.h"

int main()
{
    printf("\n--- Starting Memfuncs Tests ---\n\n");

    print_result("Memset", MemsetTest());
    print_result("Memcpy", MemcpyTest());
    print_result("Memmove", MemmoveTest());
    print_result("IsLittleEndian", IsLittleEndianTest() == IS_LITTLE_ENDIAN);

    printf("\n--- Tests Completed ---\n\n");

    return 0;
}

void print_result(char* func_name, int result)
{
    printf("Function: %-23s %s\n", func_name, (result ? "Passed" : "Failed"));
}

int MemsetTest()
{
    char m_buf1[20] = {0}, s_buf1[20] = {0};
    char m_buf2[30] = {0}, s_buf2[30] = {0};
    
    return (
        /* Basic test */
        (Memset(m_buf1, 'A', 15), memset(s_buf1, 'A', 15), 
         memcmp(m_buf1, s_buf1, 20) == 0) &&
         
        /* Unaligned test (starting from index 3) */
        (Memset(m_buf2 + 3, 'B', 22), memset(s_buf2 + 3, 'B', 22), 
         memcmp(m_buf2, s_buf2, 30) == 0) &&
         
        /* Zero bytes copy */
        (Memset(m_buf1, 'C', 0), memset(s_buf1, 'C', 0), 
         memcmp(m_buf1, s_buf1, 20) == 0)
    );
}

int MemcpyTest()
{
    char src[] = "Infinity Labs R&D Testing";
    char m_dst1[40] = {0}, s_dst1[40] = {0};
    char m_dst2[40] = {0}, s_dst2[40] = {0};
    
    return (
        /* Basic aligned copy */
        (Memcpy(m_dst1, src, sizeof(src)), memcpy(s_dst1, src, sizeof(src)), 
         memcmp(m_dst1, s_dst1, 40) == 0) &&
         
        /* Unaligned destination copy */
        (Memcpy(m_dst2 + 1, src, sizeof(src)), memcpy(s_dst2 + 1, src, sizeof(src)), 
         memcmp(m_dst2, s_dst2, 40) == 0)
    );
}

int MemmoveTest()
{
    char m_buf1[] = "1234567890";
    char s_buf1[] = "1234567890";
    char m_buf2[] = "1234567890";
    char s_buf2[] = "1234567890";
    
    return (
        /* Overlap right (dst > src) -> the reverse copy test */
        (Memmove(m_buf1 + 3, m_buf1, 5), memmove(s_buf1 + 3, s_buf1, 5), 
         memcmp(m_buf1, s_buf1, sizeof(m_buf1)) == 0) &&
         
        /* Overlap left (dst < src) -> the forward copy test */
        (Memmove(m_buf2, m_buf2 + 3, 5), memmove(s_buf2, s_buf2 + 3, 5), 
         memcmp(m_buf2, s_buf2, sizeof(m_buf2)) == 0)
    );
}

int IsLittleEndianTest()
{
    return (IsLittleEndian() == 1);
}
