/*********************************************************
*
* Description: A program that navigates in an ext2 file system
* Author: Moshe Yifrah
*
*********************************************************/

#define _XOPEN_SOURCE 500 /* Enables pread in strict ANSI mode */

#include <stdlib.h>    /* malloc, calloc, free, size_t, NULL , strtol */
#include <stdint.h>    /* uint32_t, uint16_t */
#include <unistd.h>    /* read, pread, pwrite, close, lseek, getuid, getgid */
#include <fcntl.h>     /* open, O_RDWR */
#include <sys/types.h> /* off_t, uid_t, gid_t , mode_t */
#include <sys/stat.h>  /* S_ISDIR, S_ISREG, S_IRUSR, S_IRGRP, S_IROTH, S_IXUSR, S_IXGRP, S_IXOTH */
#include <assert.h>    /* assert */
#include <ctype.h>     /* isspace */

#include "ext2.h"
#include "ext2_linux.h"/* ext2 linux structs and macros */
#include "string.h"    /* StrDup, StrTok, StrLen, StrNCmp */

#define SUPERBLOCK_OFFSET 1024
#define EXT2_MID_BLOCK_SIZE 2048
#define EXT2_VALID_PERM_MASK 07777
#define OCTAL_BASE 8
#define ALL_PERM (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX)

/*********************************************************/

static off_t GetInodeTable(ext2_t* ext2_fs, uint32_t inode_num);
static int GetInode(ext2_t* ext2_fs, uint32_t inode_num, struct ext2_inode* inode);
static int SearchDir(ext2_t* ext2_fs, struct ext2_inode* inode, const char* name);
static int SearchBlock(uint32_t block_size, void* block_buf, const char* name);
static uint32_t Min(uint32_t x, uint32_t y);
static int CheckAccess(struct ext2_inode* inode, int check_execute);
static int IsDiskOwner(int disk_fd);
static off_t GetInodeOffset(ext2_t* ext2_fs, uint32_t inode_num);
static mode_t GetNumericPerm(const char* perm, mode_t old_perm);
static int IsOctal(char c);
static mode_t GetOctalPerm(const char* perm);
static mode_t GetSymbolicPerm(const char* perm, const mode_t old_perm);
static int ResolveClause(char* clause, mode_t* result);
static mode_t ParseWho(char** ptr);
static mode_t ParsePerms(char** ptr);

/*********************************************************/

struct ext2_handler
{
	off_t gdt_base_offset;
	uint32_t block_size;
	uint32_t inodes_per_group;
	uint32_t inodes_count;
	uint16_t inode_size;
	int fd;
};

ext2_error_t g_ext2_error = EXT2_SUCCESS;

/*********************************************************/

void* FindFileInFS(const char* dev_path, const char* file_path)
{
	ext2_t* ext2_fs = NULL;
	void* buffer = NULL;
	uint32_t inode_num = 0;
	size_t file_size = 0;
	
	assert(NULL != dev_path);
	assert(NULL != file_path);
	
	g_ext2_error = EXT2_SUCCESS;

	ext2_fs = OpenFS(dev_path);
	
	if (NULL == ext2_fs)
	{
		if (EXT2_SUCCESS == g_ext2_error)
		{
			g_ext2_error = EXT2_ERR_OPEN_DEVICE;
		}
		
		return NULL;
	}
	
	inode_num = GetFileInode(ext2_fs, file_path);
	
	if (0 == inode_num)
	{
		if (EXT2_SUCCESS == g_ext2_error)
		{
			g_ext2_error = EXT2_ERR_FILE_NOT_FOUND;
		}
		
		CloseFS(ext2_fs);
		return NULL;
	}
	
	file_size = GetFileSize(ext2_fs, inode_num);
	
	if (0 == file_size)
	{
		g_ext2_error = EXT2_ERR_EMPTY_FILE;
		CloseFS(ext2_fs);
		return NULL;
	}
	
	buffer = calloc (file_size + 1, 1);
	
	if (NULL == buffer)
	{
		g_ext2_error = EXT2_ERR_ALLOC_BUFFER;
		CloseFS(ext2_fs);
		return NULL;
	}
	
	if(NULL == ReadData(ext2_fs, inode_num, buffer, file_size))
	{
		if (EXT2_SUCCESS == g_ext2_error)
		{
			g_ext2_error = EXT2_ERR_READ_DISK;
		}
		
		free(buffer);
		CloseFS(ext2_fs);
		return NULL;
	}
	
	if (0 != CloseFS(ext2_fs))
	{
		g_ext2_error = EXT2_ERR_CLOSE_DEVICE;
		free(buffer);
		return NULL;
	}
	
	return buffer;
}

/*********************************************************/

ext2_t* OpenFS(const char* dev_path)
{
	ext2_t* ext2_fs = NULL;
	struct ext2_super_block super_block;
	int fd = 0;
	
	assert(NULL != dev_path);
	
	fd = open(dev_path, O_RDWR);
	
	if (-1 == fd)
	{
		return NULL;
	}
	
	ext2_fs = (ext2_t*) malloc (sizeof(ext2_t));
	
	if (NULL == ext2_fs)
	{
		close(fd);
		return NULL;
	}
	
	ext2_fs -> fd = fd;
	
	lseek(fd, SUPERBLOCK_OFFSET, SEEK_SET);
	
	if (sizeof(struct ext2_super_block) != read (fd, &super_block, sizeof(struct ext2_super_block)))
	{
		free(ext2_fs);
		close(fd);
		return NULL;
	}
	
	if (EXT2_SUPER_MAGIC != super_block.s_magic)
	{
		g_ext2_error = EXT2_ERR_WRONG_FS_MAGIC;
		free(ext2_fs);
		ext2_fs = NULL;
		close(fd);
		return NULL;
	}
	
	ext2_fs -> block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
	ext2_fs -> inodes_per_group = super_block.s_inodes_per_group;
	ext2_fs -> inodes_count = super_block.s_inodes_count;
	ext2_fs -> inode_size = (EXT2_GOOD_OLD_REV == super_block.s_rev_level)?
		EXT2_GOOD_OLD_INODE_SIZE : super_block.s_inode_size;
	ext2_fs -> gdt_base_offset = (EXT2_MIN_BLOCK_SIZE == ext2_fs -> block_size)?
		EXT2_MID_BLOCK_SIZE : ext2_fs -> block_size;
	
	return ext2_fs;
}

/*********************************************************/

int CloseFS(ext2_t* ext2_fs)
{
	int fd = 0;
	
	assert(NULL != ext2_fs);
	
	fd = ext2_fs -> fd;
	
	free(ext2_fs);
	ext2_fs = NULL;
	
	return close(fd);
}

/*********************************************************/

size_t GetFileSize(ext2_t* ext2_fs, uint32_t inode_num)
{
	struct ext2_inode inode;
	
	assert(NULL != ext2_fs);
	assert(EXT2_ROOT_INO <= inode_num && inode_num <= ext2_fs -> inodes_count);
	
	if (0 != GetInode(ext2_fs, inode_num, &inode))
	{
		return 0;
	}
	
	return inode.i_size;
}

/*********************************************************/

int GetFileInode(ext2_t* ext2_fs, const char* file_path)
{
	struct ext2_inode inode;
	char* path = NULL;
	char* name = NULL;
	int next_inode = 0;

	assert(NULL != ext2_fs);
	assert(NULL != file_path);

	if (0 != GetInode(ext2_fs, EXT2_ROOT_INO, &inode))
	{
		return 0;
	}

	if (!CheckAccess(&inode, 1))
	{
		g_ext2_error = EXT2_ERR_ROOT_ACCESS_DENIED;
		return 0;
	}

	path = StrDup(file_path);

	if (NULL == path)
	{
		return 0;
	}

	name = StrTok(path, "/");

	while (NULL != name)
	{
		if (!(S_ISDIR(inode.i_mode)))
		{
			g_ext2_error = EXT2_ERR_FILE_NOT_FOUND;
			free(path);
			return 0;
		}

		next_inode = SearchDir(ext2_fs, &inode, name);

		if (0 == next_inode)
		{
			g_ext2_error = EXT2_ERR_FILE_NOT_FOUND;
			free(path);
			return 0;
		}

		if (0 != GetInode(ext2_fs, next_inode, &inode))
		{
			g_ext2_error = EXT2_ERR_FILE_NOT_FOUND;
			free(path);
			return 0;
		}

		name = StrTok(NULL, "/");

		if (NULL != name)
		{
			if (!CheckAccess(&inode, 1))
			{
				g_ext2_error = EXT2_ERR_TRAVERSAL_DENIED;
				free(path);
				return 0;
			}
		}
	}

	free(path);

	if (!S_ISREG(inode.i_mode)) 
	{
		g_ext2_error = EXT2_ERR_NOT_REGULAR_FILE;
		return 0;
	}

	return next_inode;
}

/*---------------------------------------------------------*/

static int SearchDir(ext2_t* ext2_fs, struct ext2_inode* inode, const char* name)
{
	void* block_buf = NULL;
	int block_index = 0;
	int block_result = 0;
	
	assert(NULL != ext2_fs);
	assert(NULL != inode);
	assert(NULL != name);
	
	block_buf = malloc(ext2_fs -> block_size);
	
	if (NULL == block_buf)
	{
		return 0;
	}
	
	for (; block_index < EXT2_NDIR_BLOCKS; ++block_index)
	{
		/* Last block */
		
		if(0 == inode -> i_block[block_index])
		{
			break;
		}
	
		/* Read error */
		
		if (ext2_fs -> block_size != pread(ext2_fs -> fd, block_buf, ext2_fs -> block_size,
			 ext2_fs -> block_size * inode -> i_block[block_index]))
		{
			break;
		}
		
		block_result = SearchBlock(ext2_fs -> block_size, block_buf, name);
		
		/* File inode found */ 
		
		if (0 < block_result)
		{
			break;
		}
	
	}
	
	free(block_buf);
	block_buf = NULL;
	
	return block_result;
}

/*---------------------------------------------------------*/

static int SearchBlock(uint32_t block_size, void* block_buf, const char* name)
{
	struct ext2_dir_entry_2* dir_entry = NULL;
	
	assert(NULL != block_buf);
	assert(NULL != name);
	assert(EXT2_MIN_BLOCK_SIZE <= block_size && EXT2_MAX_BLOCK_SIZE >= block_size);
	
	dir_entry = (struct ext2_dir_entry_2*) block_buf;
	
	while ((char*) dir_entry < (char*) block_buf + block_size)
	{
		if(0 != dir_entry -> inode && dir_entry -> name_len == StrLen(name) &&
			StrNCmp(dir_entry -> name, name, dir_entry -> name_len) == 0)
		{
			return (int) (dir_entry -> inode);
		}
		
		/* Preventing infinity loop */
		
		if (0 == dir_entry -> rec_len)
		{
			break;
		}
		
		dir_entry = (struct ext2_dir_entry_2*) ((char*) dir_entry + dir_entry -> rec_len);
	}

	return 0;
}

/*---------------------------------------------------------*/

static off_t GetInodeTable(ext2_t* ext2_fs, uint32_t inode_num)
{
	struct ext2_group_desc gdt;
	uint32_t group_num = 0;
	off_t offset = 0;
	
	assert(NULL != ext2_fs);
	assert(EXT2_ROOT_INO <= inode_num && inode_num <= ext2_fs -> inodes_count);
	
	group_num = (inode_num - 1) / ext2_fs -> inodes_per_group;
	offset = ext2_fs -> gdt_base_offset + (group_num * sizeof(struct ext2_group_desc));
	
	if (sizeof(struct ext2_group_desc) != pread(ext2_fs -> fd, &gdt, sizeof(struct ext2_group_desc), offset))
	{
		return 0;
	}
	
	return (off_t) gdt.bg_inode_table * ext2_fs -> block_size;
	
}

/*---------------------------------------------------------*/

static int GetInode(ext2_t* ext2_fs, uint32_t inode_num, struct ext2_inode* inode)
{
	off_t inode_offset = 0;
	
	assert(NULL != ext2_fs);
	assert(NULL != inode);
	assert(EXT2_ROOT_INO <= inode_num && inode_num <= ext2_fs -> inodes_count);
	
	inode_offset = GetInodeOffset(ext2_fs, inode_num);
	
	if (sizeof(struct ext2_inode) != pread(ext2_fs->fd, inode, sizeof(struct ext2_inode), inode_offset))
	{
		g_ext2_error = EXT2_ERR_FILE_NOT_FOUND;
		return 1;
	}
	
	return 0;
}

/*---------------------------------------------------------*/

static off_t GetInodeOffset(ext2_t* ext2_fs, uint32_t inode_num)
{
	off_t inode_table_offset = 0;
	uint32_t index = 0;

	assert(NULL != ext2_fs);
	assert(EXT2_ROOT_INO <= inode_num && inode_num <= ext2_fs -> inodes_count);	

	inode_table_offset = GetInodeTable(ext2_fs, inode_num);
	index = (inode_num - 1) % ext2_fs -> inodes_per_group;
	
	return (off_t) (inode_table_offset + (index * ext2_fs->inode_size));
}

/*********************************************************/

void* ReadData(ext2_t* ext2_fs, uint32_t inode_num, void* buffer, size_t size)
{
	struct ext2_inode inode;
	size_t bytes_left = size;
	uint32_t read_size = 0;
	int block_index = 0;
	
	assert(NULL != ext2_fs);
	assert(EXT2_ROOT_INO <= inode_num && inode_num <= ext2_fs -> inodes_count);
	assert(NULL != buffer);
	assert(0 < size);
	
	if (0 != GetInode(ext2_fs, inode_num, &inode))
	{
		return NULL;
	}
	
	if (!CheckAccess(&inode, 0))
	{
		g_ext2_error = EXT2_ERR_READ_ACCESS_DENIED;
		return NULL;
	}
		
	for (; block_index < EXT2_NDIR_BLOCKS && bytes_left > 0; ++block_index)
	{
		/* Last block */
		
		if(0 == inode.i_block[block_index])
		{
			break;
		}
	
		read_size = Min(ext2_fs -> block_size, bytes_left);
		
		/* Read error */
		
		if (read_size != pread(ext2_fs -> fd, (char*) buffer + (block_index * ext2_fs -> block_size),
			read_size, ext2_fs -> block_size * inode.i_block[block_index]))
		{
			break;
		}
		
		bytes_left -= read_size;
	}
	
	return buffer;
}

/*---------------------------------------------------------*/

static uint32_t Min(uint32_t x, uint32_t y)
{
	return ((x > y)? y : x);
}

/*---------------------------------------------------------*/

static int CheckAccess(struct ext2_inode* inode, int check_execute)
{
	uid_t current_uid = getuid();
	gid_t current_gid = getgid();
	uint16_t mode = 0;
	
	assert(NULL != inode);
	
	mode = inode -> i_mode;

	/* Root  */
	if (0 == current_uid)
	{
		return 1;
	}

	/* Owner */
	if (current_uid == inode -> i_uid) 
	{
		return check_execute ? (mode & S_IXUSR) : (mode & S_IRUSR);
	}
	
	/* Group */
	else if (current_gid == inode -> i_gid) 
	{
		return check_execute ? (mode & S_IXGRP) : (mode & S_IRGRP);
	}
	
	/* Others */
	else 
	{
		return check_execute ? (mode & S_IXOTH) : (mode & S_IROTH);
	}
}

/*********************************************************/

int Chmod(ext2_t* ext2_fs, uint32_t inode_num, const char* perm)
{
	struct ext2_inode inode;
	off_t inode_offset = 0;
	mode_t new_perm = 0;
	uid_t current_uid = 0;
	
	assert(NULL != ext2_fs);
	assert(EXT2_ROOT_INO <= inode_num && inode_num <= ext2_fs -> inodes_count);
	assert(NULL != perm);
	
	if ('\0' == *perm || isspace((unsigned char)*perm))
	{
		g_ext2_error = EXT2_ERR_CHMOD_MISSING_OPERAND;
		return 1;
	}
	
	if (0 != GetInode(ext2_fs, inode_num, &inode))
	{
		g_ext2_error = EXT2_ERR_FILE_NOT_FOUND;
		return 1;
	}
	
	current_uid = getuid();
	
	if (0 != current_uid && inode.i_uid != current_uid && !IsDiskOwner(ext2_fs -> fd))
	{
		g_ext2_error = EXT2_ERR_CHMOD_EPERM;
		return 1;
	}
	
	new_perm = GetNumericPerm(perm, inode.i_mode & EXT2_VALID_PERM_MASK);
	
	if ((mode_t) -1 == new_perm)
	{
		g_ext2_error = EXT2_ERR_INVALID_PERM;
		return 1;
	}
	
	inode.i_mode = (inode.i_mode & S_IFMT) | (new_perm & EXT2_VALID_PERM_MASK);
	
	inode_offset = GetInodeOffset(ext2_fs, inode_num);
	
	if ((ssize_t)sizeof(inode) != pwrite(ext2_fs->fd, &inode, sizeof(inode), inode_offset))
	{
		g_ext2_error = EXT2_ERR_WRITE_DISK;
		return 1;
	}
	
	return 0;
}

/*---------------------------------------------------------*/

static int IsDiskOwner(int disk_fd) 
{
	struct stat disk_stat;

	if (-1 == fstat(disk_fd, &disk_stat)) 
	{
		return 0;
	}

	return (getuid() == disk_stat.st_uid); 
}

/*---------------------------------------------------------*/

static mode_t GetNumericPerm(const char* perm, const mode_t old_perm)
{
	assert(NULL != perm);
	
	if (IsOctal(perm[0]))
	{
		return GetOctalPerm(perm);
	}

	return GetSymbolicPerm(perm, old_perm);
}

/*---------------------------------------------------------*/

static int IsOctal(char c)
{
	return (c >= '0' && c <= '7');
}

/*---------------------------------------------------------*/

static mode_t GetOctalPerm(const char* perm)
{
	long int parsed_perm = 0;
	char* endptr = NULL;

	assert (NULL != perm);

	parsed_perm = strtol(perm, &endptr, OCTAL_BASE);

	if ('\0' == *endptr && parsed_perm >= 0 && parsed_perm <= EXT2_VALID_PERM_MASK)
	{
		return (mode_t) parsed_perm;
	}

	return (mode_t) -1;
}

/*---------------------------------------------------------*/

static mode_t GetSymbolicPerm(const char* perm, const mode_t old_perm)
{
	mode_t result = old_perm;
	char perm_copy[64];
	char* clause = NULL;
	
	assert(NULL != perm);
	
	StrNCpy(perm_copy, perm, sizeof(perm_copy) - 1);
	
	perm_copy[sizeof(perm_copy) - 1] = '\0';
	
	clause = StrTok((char*) perm, ",");
	
	while (NULL != clause)
	{
		if (!ResolveClause(clause, &result))
		{
			return (mode_t) -1;
		}
		
		clause = StrTok(NULL, ",");
	}

	return result;
}

/*---------------------------------------------------------*/

static int ResolveClause(char* clause, mode_t* result)
{
	char* ptr = clause;
	char op = 0;
	mode_t who_mask = 0;
	mode_t perm_bits = 0;
	mode_t change_mask = 0;
	
	ptr = clause;
	who_mask = ParseWho(&ptr);
	op = *ptr++;

	if (op != '+' && op != '-' && op != '=') 
	{
		return 0;
	}

	perm_bits = ParsePerms(&ptr);
	
	if ((mode_t)-1 == perm_bits)
	{
		return 0;
	}

	change_mask = (perm_bits & who_mask);

	if (op == '+')
	{
		*result |= change_mask;
	}
	else if (op == '-')
	{
		*result &= ~change_mask;
	}
	else if (op == '=')
	{
		*result = (*result & ~who_mask) | change_mask;
	}

	return 1;
}

/*---------------------------------------------------------*/

static mode_t ParseWho(char** ptr)
{
	mode_t who_mask = 0;

	while (**ptr && StrChr("ugoa", **ptr))
	{
		switch (**ptr)
		{
			case 'u':
				who_mask |= (S_IRWXU | S_ISUID);
				break;
			case 'g':
				who_mask |= (S_IRWXG | S_ISGID);
				break;
			case 'o':
				who_mask |= (S_IRWXO | S_ISVTX);
				break;
			case 'a':
				who_mask |= ALL_PERM;
				break;
		}
		(*ptr)++;
	}

	if (0 == who_mask)
	{
		who_mask = ALL_PERM;
	}

	return who_mask;
}

/*---------------------------------------------------------*/

static mode_t ParsePerms(char** ptr)
{
	mode_t perm_bits = 0;

	while (**ptr)
	{
		switch (**ptr)
		{
			case 'r':
				perm_bits |= (S_IRUSR | S_IRGRP | S_IROTH);
				break;
			case 'w':
				perm_bits |= (S_IWUSR | S_IWGRP | S_IWOTH);
				break;
			case 'x':
				perm_bits |= (S_IXUSR | S_IXGRP | S_IXOTH);
				break;
			case 's':
				perm_bits |= (S_ISUID | S_ISGID);
				break;
			case 't':
				perm_bits |= S_ISVTX;
				break;
			default:
				return (mode_t) -1;
		}
		(*ptr)++;
	}

	return perm_bits;
}

/*---------------------------------------------------------*/

mode_t GetInodePerm(ext2_t* ext2_fs, uint32_t inode_num)
{
	struct ext2_inode inode;
	
	assert(NULL != ext2_fs);
	assert(EXT2_ROOT_INO <= inode_num && inode_num <= ext2_fs -> inodes_count);
	
	if (0 != GetInode(ext2_fs, inode_num, &inode))
	{
		g_ext2_error = EXT2_ERR_FILE_NOT_FOUND;
		return (mode_t) -1;
	}
	
	return (mode_t) inode.i_mode;
}

/*********************************************************/




















