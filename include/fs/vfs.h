#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <list.h>

#define FS_TYPE_NTFS "NTFS"
#define FS_TYPE_EXT4 "EXT4"
#define FS_TYPE_FAT32 "FAT32"
#define FS_TYPE_HFS "HFS"
#define FS_TYPE_XFS "XFS"

struct vfs_File_t;
struct vfs_Directory_t;
struct vfs_FileSystem_t;
struct vfs_FileStream_t;

struct vfs_File_t {
    char* name;
    char* path;
    size_t size;
    struct {
        bool readable:1;   // Indicates if the file is readable
        bool writable:1;  // Indicates if the file is writable
        bool executable:1; // Indicates if the file is executable
        bool hidden:1;    // Indicates if the file is hidden
        bool system:1;    // Indicates if the file is a system file
        bool reserved:3; // Other flags for future use
    } permissions; // Placeholder for file permissions structure
};

struct vfs_Directory_t {
    char* name;
    char* path;
    List files; // List of files in the directory
};

struct vfs_FileSystem_t {
    char* name; // Name of the file system
    char* type; // Type of the file system (e.g., ext4, ntfs)
    size_t total_size; // Total size of the file system
    size_t free_size; // Free size available in the file system
    List directories; // List of directories in the file system
};

struct vfs_FileStream_t {
    struct vfs_File_t* file; // Pointer to the file being accessed
    size_t position; // Current position in the file stream
    size_t length; // Length of the file stream
    bool is_open; // Indicates if the file stream is open
};

#ifdef __cplusplus
}
#endif