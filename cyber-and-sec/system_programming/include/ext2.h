/*********************************************************
*
* Description: Declarations for ext2 filesystem parser utilities
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef EXT2_H
#define EXT2_H

/* Includes ******************************************************/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t */

/* Declarations ************************************************/

typedef struct ext2_handler ext2_t;

/* Error Handling **********************************************/
	
typedef enum ext2_error {
	EXT2_SUCCESS = 0,
	EXT2_ERR_OPEN_DEVICE,
	EXT2_ERR_FILE_NOT_FOUND,
	EXT2_ERR_EMPTY_FILE,
	EXT2_ERR_ALLOC_BUFFER,
	EXT2_ERR_READ_DISK,
	EXT2_ERR_WRITE_DISK,
	EXT2_ERR_CLOSE_DEVICE,
	EXT2_ERR_ROOT_ACCESS_DENIED,
	EXT2_ERR_TRAVERSAL_DENIED,
	EXT2_ERR_READ_ACCESS_DENIED,
	EXT2_ERR_NOT_REGULAR_FILE,
	EXT2_ERR_WRONG_FS_MAGIC,
	EXT2_ERR_CHMOD_EPERM,
	EXT2_ERR_INVALID_PERM,
	EXT2_ERR_CHMOD_MISSING_OPERAND
} ext2_error_t;

extern ext2_error_t g_ext2_error;

void PrintExt2Error(void);

/* Implementation Functions ***********************************/

void* FindFileInFS(const char* dev_path, const char* file_path);

/* -----------------------------------------------------------*/

ext2_t* OpenFS(const char* dev_path);

/* -----------------------------------------------------------*/

int CloseFS(ext2_t* ext2_fs);

/* -----------------------------------------------------------*/
 
int GetFileInode(ext2_t* ext2_fs, const char* file_path);

/* -----------------------------------------------------------*/

size_t GetFileSize(ext2_t* ext2_fs, uint32_t inode_num);

/* -----------------------------------------------------------*/
 
void* ReadData(ext2_t* ext2_fs, uint32_t inode_num, void* buffer, size_t size);

/* -----------------------------------------------------------*/

int Chmod(ext2_t* ext2_fs, uint32_t inode_num, const char* perm); 

/* -----------------------------------------------------------*/

mode_t GetInodePerm(ext2_t* ext2_fs, uint32_t inode_num);

/* -----------------------------------------------------------*/

#endif /* EXT2_H */
