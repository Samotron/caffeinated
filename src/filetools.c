#include "filetools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>

void format_size(unsigned long long size, char *buffer, size_t buffer_size) {
    if (size >= 1024ULL * 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
    } else if (size >= 1024ULL * 1024) {
        snprintf(buffer, buffer_size, "%.2f MB", size / (1024.0 * 1024.0));
    } else if (size >= 1024ULL) {
        snprintf(buffer, buffer_size, "%.2f KB", size / 1024.0);
    } else {
        snprintf(buffer, buffer_size, "%llu B", size);
    }
}

unsigned long long get_dir_size(const char *path) {
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char search_path[MAX_PATH];
    unsigned long long total_size = 0;
    
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    hFind = FindFirstFile(search_path, &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            char subdir[MAX_PATH];
            snprintf(subdir, sizeof(subdir), "%s\\%s", path, findData.cFileName);
            total_size += get_dir_size(subdir);
        } else {
            ULARGE_INTEGER filesize;
            filesize.LowPart = findData.nFileSizeLow;
            filesize.HighPart = findData.nFileSizeHigh;
            total_size += filesize.QuadPart;
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
    return total_size;
}

int cmd_diskusage(const char *path) {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    
    if (GetDiskFreeSpaceEx(path, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        char total_str[50], used_str[50], free_str[50];
        unsigned long long used = totalBytes.QuadPart - totalFreeBytes.QuadPart;
        
        format_size(totalBytes.QuadPart, total_str, sizeof(total_str));
        format_size(used, used_str, sizeof(used_str));
        format_size(totalFreeBytes.QuadPart, free_str, sizeof(free_str));
        
        printf("Disk Usage for %s:\n", path);
        printf("Total: %s\n", total_str);
        printf("Used: %s\n", used_str);
        printf("Free: %s\n", free_str);
        printf("Usage: %.1f%%\n", (used / (double)totalBytes.QuadPart) * 100);
        return 0;
    }
    
    fprintf(stderr, "Failed to get disk usage for %s\n", path);
    return 1;
}

int cmd_finddup(const char *path) {
    printf("Finding duplicates in: %s\n", path);
    printf("Note: Basic implementation - checking file sizes only\n\n");
    
    // This is a simplified version
    printf("Duplicate detection not fully implemented on Windows yet\n");
    return 0;
}

#elif defined(__linux__)
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

void format_size(unsigned long long size, char *buffer, size_t buffer_size) {
    if (size >= 1024ULL * 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
    } else if (size >= 1024ULL * 1024) {
        snprintf(buffer, buffer_size, "%.2f MB", size / (1024.0 * 1024.0));
    } else if (size >= 1024ULL) {
        snprintf(buffer, buffer_size, "%.2f KB", size / 1024.0);
    } else {
        snprintf(buffer, buffer_size, "%llu B", size);
    }
}

unsigned long long get_dir_size(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return 0;
    
    unsigned long long total_size = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (lstat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                total_size += get_dir_size(full_path);
            } else if (S_ISREG(st.st_mode)) {
                total_size += st.st_size;
            }
        }
    }
    
    closedir(dir);
    return total_size;
}

int cmd_diskusage(const char *path) {
    struct statvfs stat;
    
    if (statvfs(path, &stat) != 0) {
        fprintf(stderr, "Failed to get disk usage for %s\n", path);
        return 1;
    }
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long free = stat.f_bfree * stat.f_frsize;
    unsigned long long avail = stat.f_bavail * stat.f_frsize;
    unsigned long long used = total - free;
    
    char total_str[50], used_str[50], free_str[50], avail_str[50];
    format_size(total, total_str, sizeof(total_str));
    format_size(used, used_str, sizeof(used_str));
    format_size(free, free_str, sizeof(free_str));
    format_size(avail, avail_str, sizeof(avail_str));
    
    printf("Disk Usage for %s:\n", path);
    printf("Total: %s\n", total_str);
    printf("Used: %s\n", used_str);
    printf("Free: %s\n", free_str);
    printf("Available: %s\n", avail_str);
    printf("Usage: %.1f%%\n", (used / (double)total) * 100);
    
    return 0;
}

typedef struct FileEntry {
    char path[1024];
    unsigned long long size;
    struct FileEntry *next;
} FileEntry;

void scan_directory(const char *path, FileEntry **files) {
    DIR *dir = opendir(path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (lstat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                scan_directory(full_path, files);
            } else if (S_ISREG(st.st_mode)) {
                FileEntry *new_entry = malloc(sizeof(FileEntry));
                strncpy(new_entry->path, full_path, sizeof(new_entry->path) - 1);
                new_entry->size = st.st_size;
                new_entry->next = *files;
                *files = new_entry;
            }
        }
    }
    
    closedir(dir);
}

int cmd_finddup(const char *path) {
    printf("Finding duplicates in: %s\n", path);
    printf("Scanning files (by size)...\n\n");
    
    FileEntry *files = NULL;
    scan_directory(path, &files);
    
    int duplicates_found = 0;
    FileEntry *current = files;
    
    while (current != NULL) {
        FileEntry *compare = current->next;
        int found_dup = 0;
        
        while (compare != NULL) {
            if (current->size == compare->size && current->size > 0) {
                if (!found_dup) {
                    printf("Duplicate files (size: %llu bytes):\n", current->size);
                    printf("  %s\n", current->path);
                    found_dup = 1;
                    duplicates_found++;
                }
                printf("  %s\n", compare->path);
            }
            compare = compare->next;
        }
        
        if (found_dup) printf("\n");
        current = current->next;
    }
    
    // Free memory
    while (files != NULL) {
        FileEntry *temp = files;
        files = files->next;
        free(temp);
    }
    
    if (duplicates_found == 0) {
        printf("No duplicate files found (by size)\n");
    } else {
        printf("Found %d sets of potential duplicates\n", duplicates_found);
        printf("Note: This checks file sizes only. Use hash comparison for accuracy.\n");
    }
    
    return 0;
}

#else
int cmd_diskusage(const char *path) {
    fprintf(stderr, "Disk usage not supported on this platform\n");
    return 1;
}

int cmd_finddup(const char *path) {
    fprintf(stderr, "Duplicate finder not supported on this platform\n");
    return 1;
}
#endif
