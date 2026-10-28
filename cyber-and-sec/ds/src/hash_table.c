/*********************************************************
*
* Description: Implemention of a hash table using a singly 
* linked list
* Author: Moshe Yifrah
*
*********************************************************/

#include <stdlib.h>      /* malloc, free, size_t, NULL */
#include <assert.h>      /* assert */

#include "hash_table.h"  /* Hash table header file */
#include "slist.h"       /* List header file */

/*********************************************************/

static void CleanBuckets(list_t** buckets, size_t count);
static int KeyCmp(void* node_data, void* user_data);
static list_iter_t FindIterInBucket(const hash_table_t* table, void* key, size_t* bucket_index);
static int FreeData(void* data, void* params);
static int IsEndOfList(const list_t* list, list_iter_t iter);
static int ActionFuncHelper(void* node_data, void* user_data);

/*********************************************************/

struct hash_table
{
	list_t** list;
	size_t num_of_buckets;
	CmpFunc cmp_func;
	HashFunc hash_func;
};

typedef struct user_data
{
	void* key;
	void* value;
} data_t;

typedef struct key_cmp_helper
{
	void* key;
	CmpFunc cmp_func;
} key_cmp_t;

typedef struct action_func_helper
{
	void* params;
	ActionFunc action_func;
} action_func_helper_t;

/*********************************************************/

hash_table_t* HTCreate(size_t num_of_buckets, HashFunc hash_func, CmpFunc cmp_func)
{
	hash_table_t* table = NULL;
	size_t bucket = 0;
	
	assert(0 < num_of_buckets);
	assert(NULL != hash_func);
	assert(NULL != cmp_func);
	
	table = (hash_table_t*) malloc (sizeof(hash_table_t));
	
	if (NULL == table)
	{
		return NULL;
	}
	
	table -> list = (list_t**) malloc (sizeof(list_t*) * num_of_buckets);
	
	if (NULL == table -> list)
	{
		free(table);
		return NULL;
	}
	
	for(; bucket < num_of_buckets; ++bucket)
	{ 
		table -> list [bucket] = ListCreate();
		
		if(NULL == table -> list [bucket])
		{
			CleanBuckets(table -> list, bucket);
			free(table);
			return NULL;
		}
	}
	
	table -> num_of_buckets = num_of_buckets;
	table -> cmp_func = cmp_func;
	table -> hash_func = hash_func;
	
	return table;
}

/*---------------------------------------------------------*/

static void CleanBuckets(list_t** buckets, size_t count)
{
	while (0 < count)
		{
			--count;
			ListForEach(buckets[count], FreeData, NULL);
			ListDestroy(buckets [count]);
		}
		
		free(buckets);
}

/*---------------------------------------------------------*/

static int FreeData(void* data, void* params)
{
	(void) params;
	free(data);
	
	return 0;
}

/*********************************************************/

void HTDestroy(hash_table_t* table)
{
	assert(NULL != table);
	
	CleanBuckets(table -> list, table -> num_of_buckets);
	
	free(table);
}

/*********************************************************/

void HTInsert(hash_table_t* table, void* key, void* value)
{
	size_t bucket_index = 0;
	data_t* new_data = NULL;
	list_iter_t existing_data = NULL;
	
	existing_data = FindIterInBucket(table, key, &bucket_index);
	
	if (!IsEndOfList(table -> list[bucket_index], existing_data))
	{
		((data_t*) ListGetData(existing_data)) -> value = value;
		return;
	} 
	
	new_data = (data_t*) malloc (sizeof(data_t));
	
	if (NULL == new_data)
	{
		return;
	}
	
	new_data -> key = key;
	new_data -> value = value;
	
	ListInsert (ListBegin (table -> list [bucket_index]), new_data);
}

/*---------------------------------------------------------*/

static int IsEndOfList(const list_t* list, list_iter_t iter)
{
	return ListIsSameIterator(iter, ListEnd(list));
}

/*********************************************************/

void HTRemove(hash_table_t* table, void* key)
{
	size_t bucket_index = 0;
	list_iter_t iter = NULL;
	
	iter = FindIterInBucket(table, key, &bucket_index);
	
	if (IsEndOfList(table -> list[bucket_index], iter))
	{
		return;
	}
	
	free(ListGetData(iter));
	ListRemove(iter);
}

/*********************************************************/

void* HTFind(const hash_table_t* table, void* key)
{
	size_t bucket_index = 0;
	list_iter_t found_iter = NULL;
	data_t* data = NULL;

	found_iter = FindIterInBucket(table, key, &bucket_index);

	if (IsEndOfList(table -> list[bucket_index], found_iter))
	{
		return NULL;
	}

	data = (data_t*) ListGetData(found_iter);

	return data -> value;
}

/*---------------------------------------------------------*/

static list_iter_t FindIterInBucket(const hash_table_t* table, void* key, size_t* bucket_index)
{
	key_cmp_t key_cmp;
	
	assert(NULL != table);
	assert(NULL != table -> list);
	assert(NULL != key);
	
	*bucket_index = table -> hash_func(key) % table -> num_of_buckets;
	
	assert(NULL != table -> list [*bucket_index]);
	
	key_cmp.key = key;
	key_cmp.cmp_func = table -> cmp_func;
	
	return ListFind(table -> list [*bucket_index], &key_cmp, KeyCmp);
}

/*---------------------------------------------------------*/

static int KeyCmp(void* node_data, void* user_data)
{
	data_t* list_element = (data_t*) node_data;
	key_cmp_t* user_element = (key_cmp_t*) user_data;
	
	return user_element -> cmp_func (list_element -> key, user_element -> key);
}

/*********************************************************/

size_t HTSize(const hash_table_t* table)
{
	size_t bucket = 0;
	size_t size = 0;
	
	assert(NULL != table);
	assert(NULL != table -> list);
	
	for (; bucket < table -> num_of_buckets; ++bucket)
	{
		size += ListCount(table -> list [bucket]);
	}
	
	return size;
}

/*********************************************************/

int HTIsEmpty(const hash_table_t* table)
{
	size_t bucket = 0;
	
	assert(NULL != table);
	assert(NULL != table -> list);
	
	for (; bucket < table -> num_of_buckets; ++bucket)
	{
		if (!ListIsEmpty(table -> list [bucket]))
		{
			return 0;
		}
	}
	
	return 1;
}

/*********************************************************/

int HTForEach(const hash_table_t* table, ActionFunc action_func, void* params)
{
	size_t bucket = 0;
	action_func_helper_t data;
	
	assert(NULL != table);
	assert(NULL != table -> list);
	assert(NULL != action_func);
	
	data.params = params;
	data.action_func = action_func;
	
	for (; bucket < table -> num_of_buckets; ++bucket)
	{
		if (0 < ListForEach(table -> list [bucket], ActionFuncHelper, &data))
		{
			return 1;
		}
	}
	
	return 0;
}

/*---------------------------------------------------------*/

static int ActionFuncHelper(void* node_data, void* user_data)
{
	data_t* list_element = (data_t*) node_data;
	action_func_helper_t* user_element = (action_func_helper_t*) user_data;
	
	return user_element -> action_func (list_element -> value, user_element -> params);
}

/*********************************************************/
