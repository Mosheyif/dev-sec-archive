/*********************************************************
*
* Description: A program that allows adding, deleting and 
* authenticating users.
* Author: Moshe Yifrah
* 
* In order to run this project, enter the git home directory and run
* the script, using the following command: 
* ./compiling_necessities/authenticator/run_auth_privileged.sh
* The script adds the SUID bit to the executable.
*
*********************************************************/

#define _POSIX_C_SOURCE 200809L /* Allow fdopen */

#include <stdio.h>     /* fopen, fclose */
#include <crypt.h>     /* crypt_gensalt, crypt_r */
#include <assert.h>    /* assert */
#include <stdlib.h>    /* malloc, calloc, free, size_t, NULL */
#include <fcntl.h>     /* open */
#include <unistd.h>    /* close */
#include <sys/stat.h>  /* O_WRONLY , O_CREAT , O_APPEND, S_IRUSR , S_IWUSR */

#include "authenticator.h"
#include "string.h"    /* Strlen, StrDup */
#include "memFuncs.h"  /* Memset */

#define HASH_ALGO_ID "$6$" /* SHA-512 */
#define PATH_TO_DATA_FILE "/home/moshe/git/system_programming/data/users_data"
#define PATH_TO_TEMP_FILE "/home/moshe/git/system_programming/data/users.tmp"
#define MAX_USERNAME_LEN 32
#define MAX_HASH_LEN 256
#define MAX_LINE_SIZE 512

/*********************************************************/

static int IsUserExist(const char* user_name);
static char* HashPassword(const char* password, char* prefix);
static int StoreUserData(const char* user_name, char* hash);
static int FindUserInFile(const char* user_name, char* hash_buffer, size_t buffer_size);
static int GetUserHash(const char* user_name, char* hash_buffer, size_t buffer_size);
static FILE* OpenSecureFile(const char* filepath, const char* mode);
static int IsAdmin(const char* name);
void GetLineSafe(char *buffer, size_t size);
extern size_t strcspn(const char*, const char*);

/*********************************************************/

int AddUser(const char* user_name, const char* password)
{
	char* hashed_password = NULL;
	
	assert(NULL != user_name);
	assert(NULL != password);
	
	if (IsUserExist(user_name))
	{
		perror("User is already exist\n");
		return 0;
	}
	
	hashed_password = HashPassword(password, HASH_ALGO_ID);
	
	if (NULL == hashed_password)
	{
		perror("Hashing failed\n");
		return 0;
	}
	
	if (!StoreUserData(user_name, hashed_password))
	{
		perror("Storing data failed\n");
		free(hashed_password);
		return 0;
	}
	
	free(hashed_password);
	return 1;
}

/*---------------------------------------------------------*/

static int IsUserExist(const char* user_name)
{
	return FindUserInFile(user_name, NULL, 0);
}

/*---------------------------------------------------------*/

static int FindUserInFile(const char* user_name, char* hash_buffer, size_t buffer_size)
{
	FILE* data_file = NULL; 
	char line[MAX_LINE_SIZE];
	char* hash_start = NULL;
	size_t username_len = 0;
	size_t len = 0;
	int found = 0;
	
	assert(NULL != user_name);
	
	data_file = OpenSecureFile(PATH_TO_DATA_FILE, "r");

	if (NULL == data_file) 
	{
		return 0;
	}
	
	username_len = StrLen(user_name);
	
	while(NULL != fgets(line, sizeof(line), data_file))
	{
		if (0 == StrNCmp(line, user_name, username_len) && ':' == line[username_len])
		{
			if (hash_buffer != NULL) 
			{
				hash_start = StrChr(line, ':') + 1;
				
				len = StrLen(hash_start);
				
				if (len > 0 && '\n' == hash_start[len - 1]) 
				{
					hash_start[len - 1] = '\0';
				}
				
				StrNCpy(hash_buffer, hash_start, buffer_size - 1);
				
				hash_buffer[buffer_size - 1] = '\0';
			}
			
			found = 1;
			break;
		}
	}
	
	fclose(data_file);
	return found;
}

/*---------------------------------------------------------*/

static char* HashPassword(const char* password, char* prefix)
{
	struct crypt_data cdata;
	char* salt = NULL;
	char* hash = NULL;
	
	assert(NULL != password);
	assert(NULL != prefix);
	
	salt = crypt_gensalt(prefix, 0, NULL, 0);
	
	Memset(&cdata, 0, sizeof(cdata));
	
	hash = crypt_r(password, salt, &cdata);

	if (NULL == hash) 
	{
		perror("crypt_r failed");
		return NULL;
	}
	
	return StrDup(hash);
}

/*---------------------------------------------------------*/

static int StoreUserData(const char* user_name, char* hash)
{
	FILE* data_file = NULL;
	
	assert(NULL != user_name);
	assert(NULL != hash);
	
	data_file = OpenSecureFile(PATH_TO_DATA_FILE, "a");

	if (NULL == data_file) 
	{
		perror("fdopen failed");
		return 0;
	}
	
	fprintf(data_file, "%s:%s\n", user_name, hash);

	fclose(data_file);
	
	return 1;
}

/*********************************************************/

int DeleteUser(const char* user_name, const char* password)
{
	FILE* data_file = NULL;
	FILE* temp_file = NULL;
	char username_to_del[MAX_USERNAME_LEN];
	char line[MAX_LINE_SIZE];
	size_t username_len = 0;
	int found = 0;

	assert(NULL != user_name);
	assert(NULL != password);
	
	if(0 == Authenticate(user_name, password))
	{
		return 0;
	}
	
	if (IsAdmin(user_name))
	{
		printf("Enter the username to delete: ");
		GetLineSafe(username_to_del, MAX_USERNAME_LEN);
	}
	
	else
	{
		StrNCpy(username_to_del, user_name, MAX_USERNAME_LEN);
	}

	data_file = OpenSecureFile(PATH_TO_DATA_FILE, "r");
	temp_file = OpenSecureFile(PATH_TO_TEMP_FILE, "w");

	if (NULL == data_file || NULL == temp_file)
	{
		if (data_file) 
		{
			fclose(data_file);
		}
		
		if (temp_file) 
		{
			fclose(temp_file);
		}
		
		return 0;
	}

	username_len = StrLen(username_to_del);

	while (NULL != fgets(line, sizeof(line), data_file))
	{
		if (0 == StrNCmp(line, username_to_del, username_len) && ':' == line[username_len])
		{
			found = 1;
		}
		else
		{
			fputs(line, temp_file);
		}
	}

	fclose(data_file);
	fclose(temp_file);

	if (found)
	{
		rename(PATH_TO_TEMP_FILE, PATH_TO_DATA_FILE);
		return 1;
	}
	
	else
	{
		remove(PATH_TO_TEMP_FILE);
		return 0;
	}
}

/*********************************************************/

int Authenticate(const char* user_name, const char* password)
{
	char stored_hash[MAX_HASH_LEN];
	struct crypt_data cdata;
	char* recieved_hash = NULL;
	
	assert(NULL != user_name);
	assert(NULL != password);
	
	if (0 == GetUserHash(user_name, stored_hash, MAX_HASH_LEN))
	{
		return 0;
	}
	
	Memset(&cdata, 0, sizeof(cdata));
	recieved_hash = crypt_r(password, stored_hash, &cdata);

	if (NULL != recieved_hash && 0 == StrCmp(recieved_hash, stored_hash))
	{
		return 1;
	}

	return 0;
}

/*---------------------------------------------------------*/

static int GetUserHash(const char* user_name, char* hash_buffer, size_t buffer_size)
{
	return FindUserInFile(user_name, hash_buffer, buffer_size);
}

/*---------------------------------------------------------*/

static FILE* OpenSecureFile(const char* filepath, const char* mode)
{
	int flags = 0;
	int fd = 0;

	if (0 == StrCmp(mode, "a"))
	{
		flags = O_WRONLY | O_CREAT | O_APPEND;
	}
	
	else if (0 == StrCmp(mode, "w"))
	{
		flags = O_WRONLY | O_CREAT | O_TRUNC;
	}
	
	else
	{
		flags = O_RDONLY;
	}

	fd = open(filepath, flags, S_IRUSR | S_IWUSR);

	if (-1 == fd)
	{
		return NULL;
	}

	return fdopen(fd, mode);
}

/*---------------------------------------------------------*/

static int IsAdmin(const char* name)
{
	return (0 == StrCmp("Moshe", name) || 0 == StrCmp("Root", name));
}

/*---------------------------------------------------------*/

void GetLineSafe(char *buffer, size_t size) 
{
	if (fgets(buffer, size, stdin)) 
	{
		buffer[strcspn(buffer, "\n")] = 0;
	}
}

/*********************************************************/


