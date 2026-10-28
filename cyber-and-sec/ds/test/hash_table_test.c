/*********************************************************
*
* Description: Test runner for the hash table implementation
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./hash_table_test
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "hash_table.h"

#define DICT_PATH "/usr/share/dict/words"
#define NUM_OF_BUCKETS 100003 
#define MAX_WORD_LEN 100

extern char* StrDup(const char* s);
extern int StrCmp(const char* str1, const char* str2);
extern size_t StrLen(const char* str);

static size_t HashFuncString(void* key)
{
    unsigned char* str = (unsigned char*) key;
    size_t hash = 5381;
    int c = 0;

    while (0 != (c = *str++))
    {
        hash = ((hash << 5) + hash) + c; 
    }

    return hash;
}

static int CmpFuncString(void* list_data, void* user_key)
{
    return StrCmp((const char*)list_data, (const char*)user_key);
}

static int FreeStringPayload(void* value, void* params)
{
    (void)params;
    free(value);
    return 0;
}

int main(void)
{
    hash_table_t* dictionary_table = NULL;
    FILE* file = NULL;
    char buffer[MAX_WORD_LEN] = {0};
    char* word_copy = NULL;
    size_t len = 0;

    dictionary_table = HTCreate(NUM_OF_BUCKETS, HashFuncString, CmpFuncString);
    if (NULL == dictionary_table)
    {
        fprintf(stderr, "Failed to create hash table.\n");
        return 1;
    }

    file = fopen(DICT_PATH, "r");
    if (NULL == file)
    {
        fprintf(stderr, "Could not open dictionary file.\n");
        HTDestroy(dictionary_table);
        return 1;
    }

    while (NULL != fgets(buffer, sizeof(buffer), file))
    {
        size_t len = StrLen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }
        
        word_copy = StrDup(buffer);
        if (NULL != word_copy)
        {
            HTInsert(dictionary_table, word_copy, word_copy);
        }
    }
    fclose(file);

    printf("Dictionary loaded successfully!\n");
    printf("Total words in hash table: %lu\n", HTSize(dictionary_table));
    printf("---------------------------------------------------\n");
    printf("Type a word to check its spelling, or 'exit' to quit.\n");

    while (1)
    {
        printf("\n> ");
        if (NULL == fgets(buffer, sizeof(buffer), stdin))
        {
            break;
        }

        len = StrLen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }

        if (0 == StrCmp(buffer, "exit"))
        {
            break;
        }

        if (NULL != HTFind(dictionary_table, buffer))
        {
            printf("[V] Correct! '%s' is in the dictionary.\n", buffer);
        }
        else
        {
            printf("[X] Misspelled! '%s' was not found.\n", buffer);
        }
    }

    HTForEach(dictionary_table, FreeStringPayload, NULL);
    HTDestroy(dictionary_table);

    return 0;
}
