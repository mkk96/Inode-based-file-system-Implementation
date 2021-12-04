
# Inode based file system Implementation

Inode based file system(virtual) on top of the Linux file system. A user can perform all the operations which are basically supported by Linux ext2 file system like creating a disk, mounting disk, unmounting the disk, create a file, open file (in read, write or append mode), delete file, close file, list of open files.


## Prerequisites
* The size of virtual disk is 512 MB.
* code is written with respect to linux file system and path hierarchy.
## Run Locally

Clone the project

```bash
  https://github.com/mkk96/Inode-based-file-system-Implementation.git
```

Go to the project directory

```bash
  cd Inode-based-file-system-Implementation
```

Install dependencies

```bash
  sudo apt-get install g++
```

Compile the File

```bash
 g++ main.cpp -o main
```

Run the File

```bash
 ./main
```

## Features

- Disk Menu:
	* Press following key
	    * 1: create disk
		    
            Take unique disk name as input on next line.
        * 2: mount disk
          
            Open the disk for mounting purposes
        * 3: exit
          
            Exit the application.
		
- Mounting Menu:
	* Press following key
        * 1: create file
            
            Take unique file name as input on next line.
        * 2: open file
            
            Take file name as input on next line.
            Then take file mode as input as mentioned below on next line:
                
                 0: read mode
                 1: write mode
                 2: append mode
        * 3: read file
            
            Take input file descriptor of the file which you want to read.
        * 4: write file
            
            Take input file descriptor of the file which you want to write.
            Enter file content that you want to write in the file.
        * 5: append file
            
            Take input file descriptor of the file to which you want to append
            Enter the file content that you want to append to the file.
        * 6: close file
            
            Take the input file descriptor of the file to which you want to close.
        * 7: delete file
            
            Take the input file name which you want to delete.
        * 8: list of files
            
            List all existing files on the disk.
        * 9: list of opened files
            
            List all existing files which are currently open along with their file
            descriptors and mode in which they are open.
        * 10: unmount
            
            Unmount/close the disk which is current mount and return to the previous menu.
