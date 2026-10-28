Reviewer: Roman

1. ext2_t* OpenFS(const char* dev_path);

Open the device file at dev_path in Read-Only mode and get the File Descriptor (FD).

Allocate memory for the management struct (ext2_t).

Seek to byte 1024 on the disk and read the Superblock into the struct.

Calculate the real block size from the Superblock.

Seek to the block following the Superblock and read the Group Descriptor Table (GDT) into the struct.

Return the pointer to the management struct.

2. int CloseFS(ext2_t* ext2_handler);

Check if the ext2_handler is valid (not NULL).

Close the File Descriptor to release the hardware resource.

Free the ext2_handler itself.

Return status.

3. int GetFileInode(ext2_t* ext2_handler, const char* file_path);

Start at the Root Directory, which is always Inode 2.

Split the file_path string by the / delimiter.

Loop: For each part of the path:

Read the data blocks of the current directory inode.

Parse the blocks as ext2_dir_entry structures.

Compare the name in the directory entry with the current path part.

If it matches, extract the new Inode Number from that entry.

Continue the loop until you reach the final file.

Return the final Inode Number (or 0/-1 if the path is invalid/not found).

4. void* ReadData(int inode, void* buffer, size_t size);

Calculate the offset of the given inode.

Read the ext2_inode structure.

Look at the block array inside the inode to find the physical data block numbers.

Seek to those data blocks on the disk and read the raw bytes.

Copy up to size bytes into the provided buffer.

Return the buffer.

5. size_t GetFileSize(int inode, ext2_t* ext2_handler);

Seek to the offset of the given inode and read the ext2_inode structure.

Extract the value from the i_size field.

Return that size.
