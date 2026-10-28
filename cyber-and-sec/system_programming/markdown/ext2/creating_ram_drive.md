Reviewer: Roman

Execution Summary: Creating an ext2 RAM Drive

Below are the clean, step-by-step instructions for creating, formatting, and mounting a RAM drive, based on the actions executed in your terminal:

Step 1: Memory Allocation and Block Device Creation

Load the brd module into the Linux Kernel to create a virtual Block Device directly in your RAM. In this example, we created a single drive of approximately 1GB.

sudo modprobe brd rd_size=1045876 rd_nr=1


(This action generates the device file /dev/ram0)

Step 2: Formatting the Drive (Make File System)

Now that we have a raw block device sitting in memory, we need to build the ext2 file system architecture inside it (which creates the necessary Inodes and Superblocks).

sudo mkfs.ext2 /dev/ram0


Step 3: Preparing the Mount Point

Create an empty directory within the Linux directory tree. This folder will act as the "doorway" to access your new RAM drive.

sudo mkdir -p /mnt/fast_ramdisk


Step 4: Mounting the Drive

Attach the newly formatted drive (/dev/ram0) to the directory you just created so you can start writing files to it.

sudo mount /dev/ram0 /mnt/fast_ramdisk/

The correct way to dismantle the drive (before reboot):
You must unmount the drive first. Only after it is detached can you safely remove the module from the Kernel:

# 1. Unmount the drive
sudo umount /mnt/fast_ramdisk

# 2. Remove the module and free up the RAM
sudo modprobe -r brd

