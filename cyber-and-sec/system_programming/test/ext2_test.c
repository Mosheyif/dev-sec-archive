/*********************************************************
*
* Description: Test runner for ext2 filesystem parser utilities
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: sudo ./ext2_test <device>
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "string.h"
#include "ext2.h"

#ifndef S_ISVTX
#define S_ISVTX 01000
#endif

char* ModeToString(mode_t mode, char* str);

int main(int argc, char *argv[])
{
    char *content = NULL;
    const char *disk_path = NULL;
    const char *file_path = NULL;
    
    ext2_t *ext2_fs = NULL;
    char input_path[256];
    char perm_buf[32];
    char perm_str[12];
    int action = 0;
    int inode_num = 0;
    size_t file_size = 0;
    void *read_buf = NULL;
    size_t buff_len = 0;
    int c;

    if (argc != 3) 
    {
        printf("Usage: %s <disk_image_path> <file_path_inside_fs>\n", argv[0]);
        printf("Example: %s test_disk.img /dir1/secret.txt\n", argv[0]);
        return 1;
    }

    disk_path = argv[1];
    file_path = argv[2];

    printf("========================================\n");
    printf("[*] Mounting disk: %s\n", disk_path);
    printf("[*] Searching for: %s\n", file_path);
    printf("========================================\n");

    content = (char*) FindFileInFS(disk_path, file_path);
    
    if (NULL != content) 
    {
        printf("\n[+] Success! File content:\n");
        printf("----------------------------------------\n");
        printf("%s\n", content);
        printf("----------------------------------------\n");
        free(content);
    } 
    else 
    {
        PrintExt2Error();
    }

    printf("\n========================================\n");
    printf("[*] Entering Interactive Mode\n");
    printf("========================================\n");

    ext2_fs = OpenFS(disk_path);
    if (NULL == ext2_fs)
    {
        printf("[-] Fatal Error: Could not open file system for interactive mode.\n");
        PrintExt2Error();
        return 1;
    }

    while (1)
    {
        printf("\nEnter full file path (or type 'exit' to quit): ");
        if (scanf("%255s", input_path) != 1) break;
        
        if (StrCmp(input_path, "exit") == 0 || StrCmp(input_path, "quit") == 0)
        {
            break;
        }

        inode_num = GetFileInode(ext2_fs, input_path);
        
        if (0 == inode_num)
        {
            PrintExt2Error();
            continue;
        }

        printf("\nFile found! (Inode: %d)\n", inode_num);
        printf("Current Permissions: %s\n", ModeToString(GetInodePerm(ext2_fs, inode_num), perm_str));
        printf("Select action:\n");
        printf("1. Read file content\n");
        printf("2. Change permissions (chmod)\n");
        printf("3. Cancel / Search another file\n");
        printf("Choice: ");
        
        if (scanf("%d", &action) != 1) 
        {
            while (getchar() != '\n');
            continue;
        }

        if (1 == action)
        {
            file_size = GetFileSize(ext2_fs, inode_num);
            if (0 == file_size && EXT2_SUCCESS != g_ext2_error)
            {
                PrintExt2Error();
                continue;
            }
            
            read_buf = calloc(file_size + 1, 1);
            if (NULL == read_buf)
            {
                g_ext2_error = EXT2_ERR_ALLOC_BUFFER;
                PrintExt2Error();
                continue;
            }
            
            if (NULL != ReadData(ext2_fs, inode_num, read_buf, file_size))
            {
                printf("\n--- File Content ---\n%s\n--------------------\n", (char*)read_buf);
            }
            else
            {
                PrintExt2Error();
            }
            free(read_buf);
        }
        else if (2 == action)
        {
            printf("Enter new permissions: ");
            
			while ((c = getchar()) != '\n' && c != EOF);
            
            if (NULL != fgets(perm_buf, sizeof(perm_buf), stdin))
            {
            	buff_len = StrLen(perm_buf);
            	
            	if (buff_len > 0 && perm_buf[buff_len - 1] == '\n')
				{
					perm_buf[buff_len - 1] = '\0';
				}
            	
                if (0 == Chmod(ext2_fs, inode_num, perm_buf))
                {
                    mode_t updated_perm = GetInodePerm(ext2_fs, inode_num);
                    
                    printf("[+] Permissions for '%s' updated successfully!\n", input_path);
                    printf("[+] New Permissions: %s (%04o)\n", 
                           ModeToString(updated_perm, perm_str), 
                           (unsigned int)(updated_perm & 07777));
                }
                else
                {
                    PrintExt2Error();
                }
            }
        }
    }

    printf("\n[*] Exiting and closing file system...\n");
    CloseFS(ext2_fs);

    return 0;
}

/*-----------------------------------------------------------*/

void PrintExt2Error(void)
{
    switch (g_ext2_error)
    {
        case EXT2_SUCCESS:
            break;
        
        case EXT2_ERR_OPEN_DEVICE:
            printf("[-] Error: Cannot open the device\n");
            break;
            
        case EXT2_ERR_FILE_NOT_FOUND:
            printf("[-] Error: Invalid file or file not found\n");
            break;
            
        case EXT2_ERR_EMPTY_FILE:
            printf("[-] Error: File is empty or error reading size\n");
            break;
            
        case EXT2_ERR_ALLOC_BUFFER:
            printf("[-] Error: Memory allocation failed for buffer\n");
            break;
            
        case EXT2_ERR_READ_DISK:
            printf("[-] Error: Failed reading data from disk\n");
            break;

        case EXT2_ERR_WRITE_DISK:
            printf("[-] Error: Failed writing data to disk (pwrite failed)\n");
            break;
            
        case EXT2_ERR_CLOSE_DEVICE:
            printf("[-] Error: Failed to close the device\n");
            break;
            
        case EXT2_ERR_ROOT_ACCESS_DENIED:
            printf("[-] Error: Permission denied. Cannot access root directory.\n");
            break;
            
        case EXT2_ERR_TRAVERSAL_DENIED:
            printf("[-] Error: Permission denied (Traversal).\n");
            break;
            
        case EXT2_ERR_READ_ACCESS_DENIED:
            printf("[-] Error: Access denied. No read permissions for target.\n");
            break;
            
        case EXT2_ERR_NOT_REGULAR_FILE:
            printf("[-] Error: Target is not a regular file.\n");
            break;
            
        case EXT2_ERR_WRONG_FS_MAGIC:
            printf("[-] Error: Wrong fs type.\n");
            break;

        case EXT2_ERR_CHMOD_EPERM:
            printf("[-] Error: Operation not permitted. You must be the owner to chmod.\n");
            break;
            
        case EXT2_ERR_INVALID_PERM:
            printf("[-] Error: Invalid mode. Insert valid octal or symbolic permissions.\n");
            break;
            
        case EXT2_ERR_CHMOD_MISSING_OPERAND:
            printf("[-] Error: Missing permissions operand.\n");
            break;
            
        default:
            printf("[-] Error: Unknown system error.\n");
            break;
    }

    g_ext2_error = EXT2_SUCCESS;
}



char* ModeToString(mode_t mode, char* str)
{
    StrCpy(str, "----------");

    if (S_ISDIR(mode))  str[0] = 'd';
    if (S_ISLNK(mode))  str[0] = 'l';

    /* User (Owner) */
    if (mode & S_IRUSR) str[1] = 'r';
    if (mode & S_IWUSR) str[2] = 'w';
    if (mode & S_IXUSR) str[3] = (mode & S_ISUID) ? 's' : 'x';
    else if (mode & S_ISUID) str[3] = 'S';

    /* Group */
    if (mode & S_IRGRP) str[4] = 'r';
    if (mode & S_IWGRP) str[5] = 'w';
    if (mode & S_IXGRP) str[6] = (mode & S_ISGID) ? 's' : 'x';
    else if (mode & S_ISGID) str[6] = 'S';

    /* Other */
    if (mode & S_IROTH) str[7] = 'r';
    if (mode & S_IWOTH) str[8] = 'w';
    if (mode & S_IXOTH) str[9] = (mode & S_ISVTX) ? 't' : 'x';
    else if (mode & S_ISVTX) str[9] = 'T';

    return str;
}
