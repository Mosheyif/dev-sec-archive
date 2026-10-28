/*********************************************************
*
* Description: Test runner for the variable-size allocator (VSA)
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./vsa_test
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "vsa.h"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

#define POOL_SIZE 1000

extern vsa_error_t g_vsa_errno;

struct block_header 
{
    size_t block_size;
    
	#ifndef NDEBUG
    size_t pool_id;
	#endif
};

void VSAPrintError(vsa_error_t error);

static void* HelperAlloc(void* pool, size_t size, vsa_error_t expected)
{
    void* block = NULL;
    
    printf("Allocating block (%lu bytes)...\n", (unsigned long)size);
    block = VSAAlloc(pool, size);
    
    if (g_vsa_errno != VSA_SUCCESS) 
    {
        VSAPrintError(g_vsa_errno);
    }
    
    if (g_vsa_errno == expected)
    {
        printf(GREEN "Status: [PASS] | " RESET);
    }
    else
    {
        printf(RED "Status: [FAIL] (Expected %d, got %d) | " RESET, expected, g_vsa_errno);
    }
    
    printf("Max: %lu\n\n", (unsigned long)VSAMaxChunkAvailable(pool));
    g_vsa_errno = VSA_SUCCESS;
    
    return block;
}

static void HelperFree(void* pool, void* block, vsa_error_t expected)
{
    printf("Freeing block...\n");
    VSAFree(block);
    
    if (g_vsa_errno != VSA_SUCCESS) 
    {
        VSAPrintError(g_vsa_errno);
    }
    
    if (g_vsa_errno == expected)
    {
        printf(GREEN "Status: [PASS] | " RESET);
    }
    else
    {
        printf(RED "Status: [FAIL] (Expected %d, got %d) | " RESET, expected, g_vsa_errno);
    }
    
    printf("Max: %lu\n\n", (unsigned long)VSAMaxChunkAvailable(pool));
    g_vsa_errno = VSA_SUCCESS;
}

static void* GetFreshPool(void** raw)
{
    void* pool = NULL;
    *raw = malloc(POOL_SIZE);
    
    if (NULL == *raw)
    {
        printf(RED "MALLOC FAILED IN TEST\n" RESET);
        exit(1);
    }
    
    pool = VSAInit(*raw, POOL_SIZE);
    
    printf("Initializing pool (Total Raw: %d bytes)...\n", POOL_SIZE);
    printf(GREEN "Status: [PASS] | " RESET);
    printf("Usable Max: %lu\n\n", (unsigned long)VSAMaxChunkAvailable(pool));
    
    return pool;
}

static void TestGeneralWorkflow(void)
{
    void* raw = NULL;
    void* pool = NULL;
    void* b1 = NULL;
    void* b2 = NULL;
    void* b3 = NULL;
    void* b4 = NULL;
    
    printf(CYAN "\n=== RUNNING: Test General Workflow ===\n\n" RESET);
    pool = GetFreshPool(&raw);
    
    b1 = HelperAlloc(pool, 50, VSA_SUCCESS);
    b2 = HelperAlloc(pool, 70, VSA_SUCCESS);
    b3 = HelperAlloc(pool, 100, VSA_SUCCESS);
    b4 = HelperAlloc(pool, 10, VSA_SUCCESS);
    
    HelperFree(pool, b1, VSA_SUCCESS);
    HelperFree(pool, b3, VSA_SUCCESS);
    HelperFree(pool, b2, VSA_SUCCESS);
    HelperFree(pool, b4, VSA_SUCCESS);
    
    free(raw);
}

static void TestEdgeCases(void)
{
    void* raw = NULL;
    void* pool = NULL;
    void* block = NULL;
    
    printf(CYAN "\n=== RUNNING: Test Edge Cases ===\n\n" RESET);
    pool = GetFreshPool(&raw);
    
    HelperAlloc(NULL, 50, VSA_ERR_NULL_POOL);
    HelperAlloc(pool, 0, VSA_ERR_BLOCK_SIZE);
    HelperAlloc(pool, POOL_SIZE + 500, VSA_ERR_BLOCK_SIZE);
    HelperFree(pool, NULL, VSA_ERR_NULL_BLOCK);
    
    block = HelperAlloc(pool, 50, VSA_SUCCESS);
    HelperFree(pool, block, VSA_SUCCESS);
    
    printf(YELLOW "[!] Triggering Double Free...\n" RESET);
    HelperFree(pool, block, VSA_ERR_DOUBLE_FREE);
    
    free(raw);
}

static void TestDefragLogic(void)
{
    void* raw = NULL;
    void* pool = NULL;
    void* b1 = NULL;
    void* b2 = NULL;
    void* b3 = NULL;
    void* b4 = NULL;
    
    printf(CYAN "\n=== RUNNING: Test Fragmentation & Defrag ===\n\n" RESET);
    pool = GetFreshPool(&raw);
    
    b1 = HelperAlloc(pool, 100, VSA_SUCCESS);
    b2 = HelperAlloc(pool, 100, VSA_SUCCESS);
    b3 = HelperAlloc(pool, 100, VSA_SUCCESS);
    b4 = HelperAlloc(pool, 100, VSA_SUCCESS);
    
    HelperFree(pool, b1, VSA_SUCCESS);
    HelperFree(pool, b3, VSA_SUCCESS);
    HelperFree(pool, b2, VSA_SUCCESS);
    HelperFree(pool, b4, VSA_SUCCESS);
    
    free(raw);
}

static void TestExhaustion(void)
{
    void* raw = NULL;
    void* pool = NULL;
    void* big_block = NULL;
    size_t max_available = 0;
    
    printf(CYAN "\n=== RUNNING: Test Pool Exhaustion ===\n\n" RESET);
    pool = GetFreshPool(&raw);
    
    max_available = VSAMaxChunkAvailable(pool);
    big_block = HelperAlloc(pool, max_available, VSA_SUCCESS);
    
    HelperAlloc(pool, 8, VSA_ERR_BLOCK_SIZE);
    
    HelperFree(pool, big_block, VSA_SUCCESS);
    
    free(raw);
}

static void TestMaliciousBlock(void)
{
#ifndef NDEBUG
    void* raw = NULL;
    void* pool = NULL;
    void* malicious = NULL;
#endif
    
    printf(CYAN "\n=== RUNNING: Test Malicious Block ===\n\n" RESET);
    
#ifdef NDEBUG
    printf(YELLOW "[i] Release mode detected. Skipping fatal assert test.\n" RESET);
#else
    pool = GetFreshPool(&raw);
    malicious = malloc(20);
    
    if (malicious)
    {
        printf(YELLOW "[!] Triggering fatal assert (Program will abort)...\n" RESET);
        HelperFree(pool, malicious, VSA_SUCCESS); 
        free(malicious);
    }
    free(raw);
#endif
}

int main(void)
{
    TestGeneralWorkflow();
    TestEdgeCases();
    TestDefragLogic();
    TestExhaustion();
    TestMaliciousBlock();
    
    return 0;
}

void VSAPrintError(vsa_error_t error)
{
    switch (error)
    {
        case VSA_SUCCESS:
            break; 
            
        case VSA_ERR_NULL_POOL:
            fprintf(stderr, RED "VSA Error: The memory pool pointer is NULL.\n" RESET);
            break;
            
        case VSA_ERR_POOL_SIZE:
            fprintf(stderr, RED "VSA Error: Invalid pool size requested.\n" RESET);
            break;
            
        case VSA_ERR_DOUBLE_FREE:
            fprintf(stderr, RED "VSA Critical Error: Double free attempt detected!\n" RESET);
            break;
            
        case VSA_ERR_MAGIC_CORRUPTION:
            fprintf(stderr, RED "VSA Fatal Error: Magic number corruption detected! Possible buffer overflow.\n" RESET);
            break;
            
        case VSA_ERR_BLOCK_SIZE:
            fprintf(stderr, RED "VSA Error: Invalid block size requested or encountered.\n" RESET);
            break;
            
        case VSA_ERR_NULL_BLOCK:
            fprintf(stderr, RED "VSA Error: The provided block pointer is NULL.\n" RESET);
            break;
            
        default:
            fprintf(stderr, RED "VSA Unknown Error: An unrecognized error code (%d) was returned.\n" RESET, error);
            break;
    }
}
