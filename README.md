# Inode-Based File System

A simplified UNIX-style, inode-based virtual file system written in C++. It runs inside a single disk file on the host Linux file system.

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Linux](https://img.shields.io/badge/platform-Linux-FCC624?style=flat-square&logo=linux&logoColor=black)
![Domain](https://img.shields.io/badge/topic-file%20systems-6E40C9?style=flat-square)

## Overview

The program emulates a block device in an ordinary file (the "virtual disk") and builds a small file system on top of it. You can create and mount disks, then create, open, read, write, append, close, delete and list files through an interactive menu. It is loosely modelled on ext2-style structures: a superblock with free lists, an inode table and data blocks addressed through direct and indirect pointers.

### On-disk layout

| Parameter | Value |
| --- | --- |
| Block size | 4 KB (`4096` bytes) |
| Data blocks | 131,072 (512 MB logical disk) |
| Inodes | 78,644 |
| Max open file descriptors | 32 |

The disk file is laid out in this order:

1. **Superblock**: free/used bitmaps for inodes and data blocks, plus the offsets of the regions below.
2. **File name → inode table**: maps each file name to its inode number.
3. **Inode table**: each inode stores the file size and 12 block pointers.
   - 10 direct pointers
   - 1 single-indirect pointer (1,024 block addresses)
   - 1 double-indirect pointer
4. **Data blocks**

Mounting a disk loads the metadata into memory and rebuilds the free-inode and free-block lists. File data is read and written block by block with `fseek`/`fread`/`fwrite`. Unmounting writes the updated superblock, name table and inode table back to the disk file.

## Features

- Create multiple named virtual disks and mount them one at a time.
- Create and delete files. Deleting a file frees its inode and data blocks, including the blocks used for indirect pointers.
- Open files in **read**, **write** or **append** mode, each of which returns a file descriptor.
- Write mode replaces the file's contents. Append mode adds to the end.
- A file that is already open for write or append can't be opened for write or append again. A file can't be deleted while it is open.
- List all files on the disk, and list open files with their descriptors and modes.
- Data persists across unmount and remount.

## Tech stack

- **Language:** C++ (STL containers for in-memory free lists and descriptor tables)
- **I/O:** C stdio (`fopen`, `fseek`, `fread`, `fwrite`) on a regular file acting as the block device
- **Platform:** Linux

## Project structure

```
Inode-based-file-system-Implementation/
├── main.cpp               # Disk layout, block I/O, inode management, menus
└── AOS_Assignment 4.pdf   # Original assignment specification
```

## Getting started

### Prerequisites

- Linux
- `g++` (GCC is required because the code includes `<bits/stdc++.h>`): `sudo apt-get install g++`

### Build and run

```bash
git clone https://github.com/mkk96/Inode-based-file-system-Implementation.git
cd Inode-based-file-system-Implementation
g++ main.cpp -o main
./main
```

Disk files are created in the current working directory under the name you give them.

## Usage

### Disk menu

| Option | Action |
| --- | --- |
| `1` | **Create disk**. Enter a unique disk name on the next line |
| `2` | **Mount disk**. Enter the name of an existing disk; this opens the mount menu |
| `3` | **Exit** |

### Mount menu

| Option | Action |
| --- | --- |
| `1` | **Create file**. Enter a unique file name |
| `2` | **Open file**. Enter the file name, then the mode: `0` read, `1` write, `2` append. Prints the file descriptor |
| `3` | **Read file**. Enter a file descriptor opened in read mode; prints the contents |
| `4` | **Write file**. Enter a file descriptor opened in write mode, then the content |
| `5` | **Append file**. Enter a file descriptor opened in append mode, then the content |
| `6` | **Close file**. Enter the file descriptor |
| `7` | **Delete file**. Enter the file name |
| `8` | **List files** on the mounted disk |
| `9` | **List open files** with descriptors and modes |
| `10` | **Unmount**. Saves metadata and returns to the disk menu |

When writing or appending, type the content over one or more lines and finish with a line that contains only `$`.

### Example session

```text
1            # create disk
mydisk
2            # mount disk
mydisk
1            # create file
notes.txt
2            # open file
notes.txt
1            # write mode -> prints "File: notes.txt, FD: 0"
4            # write file
0
hello from the virtual disk
$
6            # close file
0
2            # reopen in read mode
notes.txt
0
3            # read file
0
10           # unmount
3            # exit
```

## Known limitations

- The read path walks only the 10 direct block pointers, so reading is reliable only for files up to about 40 KB.
- The write path allocates direct and single-indirect blocks. Allocation through the double-indirect pointer is not implemented.
- Only one disk can be mounted at a time. The file system is flat, with no directories.
- The disk file is not pre-allocated to 512 MB. It grows as blocks are written.

## Context

Built for Assignment 4 of the Advanced Operating Systems course at IIIT Hyderabad (M.Tech, 2021). `AOS_Assignment 4.pdf` is the original specification.

---

Made by [Mukul Kumar](https://mkk96.github.io) · [GitHub](https://github.com/mkk96) · [LinkedIn](https://www.linkedin.com/in/mukul-kumar-60322616b/)
