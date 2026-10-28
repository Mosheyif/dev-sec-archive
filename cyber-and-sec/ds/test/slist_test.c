/*********************************************************
*
* Description: Test runner for the singly linked list implementation
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./slist_test
*
*********************************************************/

#include <stdio.h>
#include "slist.h"

int DataCmp (void * data1, void* data2)
{
	return (data1 == data2)? 0 : 1;
}

int AddOne(void* data, void* params)
{
    (void)params;
    *(int*)data += 1;
    return 0;
}

int LPrint(void* element, void* params)
{
	(void) params;
	printf("Data: %d\n", *((int*)element));
	
	return 0;
}

int main(void)
{
    list_t* my_list = NULL;
    list_iter_t iter = NULL;
    list_iter_t iter_next = NULL;
    list_iter_t iter_find = NULL;
    size_t count = 0;
    
    int data1 = 10;
    int data2 = 20;
    int data_updated = 99;
    int data_inserted = 0;

    int val1 = 100;
    int val2 = 200;
    int val3 = 300;
    int expected_val = 0;
    list_iter_t iter_100 = NULL;
    list_iter_t iter_200 = NULL;
    list_iter_t iter_300 = NULL;
    list_iter_t iter_after_remove = NULL;
    action_function list_print = LPrint;

    printf("=== Starting SList Tests ===\n\n");

    printf("[Test 1] Testing ListCreate...\n");
    my_list = ListCreate();
    if (NULL == my_list)
    {
        printf("[-] FAIL: ListCreate returned NULL. Memory allocation failed.\n");
        return 1;
    }
    printf("[+] PASS: ListCreate returned a valid pointer.\n");	

    printf("\n[Test 2] Testing ListIsEmpty (Empty State)...\n");
    if (1 == ListIsEmpty(my_list))
    {
        printf("[+] PASS: List is correctly identified as empty.\n");
    }
    else
    {
        printf("[-] FAIL: List should be empty but returned false.\n");
    }

    printf("\n[Test 3] Testing ListInsert & ListGetData...\n");
    iter = ListBegin(my_list);
    iter = ListInsert(iter, &data1);
    
    data_inserted = *((int*)ListGetData(iter));
    if (data_inserted == data1)
    {
        printf("[+] PASS: Inserted %d and retrieved it successfully.\n", data_inserted);
        if (0 != ListForEach(my_list, list_print, NULL))
        {
        	printf("Printing failed\n");
        }
    }
    else
    {
        printf("[-] FAIL: Data mismatch. Expected %d, got %d.\n", data1, data_inserted);
    }

    printf("\n[Test 4] Testing ListIsEmpty (Non-Empty State)...\n");
    if (0 == ListIsEmpty(my_list))
    {
        printf("[+] PASS: List is correctly identified as not empty.\n");
    }
    else
    {
        printf("[-] FAIL: List should not be empty now.\n");
    }

    printf("\n[Test 5] Testing ListNext & Multiple Inserts...\n");
    iter = ListInsert(iter, &data2); 
    
    iter_next = ListNext(iter);
    if (*((int*)ListGetData(iter_next)) == data1)
    {
        printf("[+] PASS: ListNext successfully moved to the next element (%d).\n", data1);
        
        if (0 != ListForEach(my_list, list_print, NULL))
        {
        	printf("Printing failed\n");
        }        
    }
    else
    {
        printf("[-] FAIL: ListNext points to the wrong data.\n");
    }

    printf("\n[Test 6] Testing ListSetData...\n");
    ListSetData(iter_next, &data_updated);
    if (*((int*)ListGetData(iter_next)) == data_updated)
    {
        printf("[+] PASS: ListSetData updated the value to %d.\n", data_updated);
        if (0 != ListForEach(my_list, list_print, NULL))
        {
        	printf("Printing failed\n");
        }        
    }
    else
    {
        printf("[-] FAIL: ListSetData failed to update the value.\n");
    }

    printf("\n[Test 7] Testing ListEnd & ListIsSameIterator...\n");
    iter_next = ListNext(iter_next); 
    
    if (1 == ListIsSameIterator(iter_next, ListEnd(my_list)))
    {
        printf("[+] PASS: ListIsSameIterator correctly identified the end of the list.\n");
    }
    else
    {
        printf("[-] FAIL: Iterator comparison with ListEnd failed.\n");
    }

	printf("\n[Test 8] Testing ListRemove (Happy Flow)...\n");
    iter_100 = ListInsert(ListBegin(my_list), &val1);
    iter_200 = ListInsert(ListNext(iter_100), &val2);
    iter_300 = ListInsert(ListNext(iter_200), &val3);
    
    iter_after_remove = ListRemove(iter_200);
    
    if (*((int*)ListGetData(iter_after_remove)) == val3)
    {
        printf("[+] PASS: The list correctly skipped the removed node (100 -> 300).\n");
        if (0 != ListForEach(my_list, list_print, NULL))
        {
        	printf("Printing failed\n");
        }        
    }
    else
    {
        printf("[-] FAIL: ListRemove failed. Expected 300, got %d.\n", 
               *((int*)ListGetData(iter_after_remove)));
    }

    printf("\n[Test 9] Testing ListFind...\n");
    iter_find = ListFind(my_list, &val1, DataCmp);
    if (1 == ListIsSameIterator(iter_find, iter_100) && *((int*)ListGetData(iter_find)) == val1)
    {
        printf("[+] PASS: ListFind successfully located %d.\n", val1);
    }
    else
    {
        printf("[-] FAIL: ListFind failed to locate the correct node.\n");
    }

    printf("\n[Test 10] Testing ListCount...\n");
    count = ListCount(my_list);
    if (4 == count)
    {
        printf("[+] PASS: ListCount returned %lu nodes.\n", (unsigned long)count);
    }
    else
    {
        printf("[-] FAIL: ListCount expected 4, got %lu.\n", (unsigned long)count);
    }

	printf("\n[Test 11] Testing ListForEach...\n");
    
    expected_val = val1 + 1; 
    
    ListForEach(my_list, AddOne, NULL);
    
    if (*((int*)ListGetData(iter_find)) == expected_val)
    {
        printf("[+] PASS: ListForEach successfully modified elements.\n");
    }
    else
    {
        printf("[-] FAIL: ListForEach failed. Expected %d, got %d.\n", 
               expected_val, *((int*)ListGetData(iter_find)));
    }

    printf("\n[Test 12] Testing ListDestroy...\n");
    ListDestroy(my_list); 
    printf("[+] PASS: ListDestroy completed without crashing.\n");

    printf("\n=== All basic tests finished ===\n");

    return 0;
}
