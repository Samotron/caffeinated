#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

// Environment variables
int cmd_env(const char *var) {
    if (var == NULL) {
        // List all environment variables
        printf("=== Environment Variables ===\n\n");
        
#ifdef _WIN32
        LPTCH env = GetEnvironmentStrings();
        if (env == NULL) {
            fprintf(stderr, "Failed to get environment variables\n");
            return 1;
        }
        
        LPTSTR ptr = env;
        while (*ptr) {
            printf("%s\n", ptr);
            ptr += strlen(ptr) + 1;
        }
        FreeEnvironmentStrings(env);
#else
        extern char **environ;
        for (char **env_ptr = environ; *env_ptr != NULL; env_ptr++) {
            printf("%s\n", *env_ptr);
        }
#endif
        return 0;
    } else {
        // Get specific variable
        char *value = getenv(var);
        if (value) {
            printf("%s=%s\n", var, value);
            return 0;
        } else {
            fprintf(stderr, "Environment variable '%s' not found\n", var);
            return 1;
        }
    }
}

// Enhanced .env file inspector
typedef struct EnvVar {
    char *name;
    char *value;
    int is_system;  // 1 if from system env, 0 if from .env file
    struct EnvVar *next;
} EnvVar;

void free_env_vars(EnvVar *head) {
    while (head) {
        EnvVar *temp = head;
        head = head->next;
        free(temp->name);
        free(temp->value);
        free(temp);
    }
}

void add_env_var(EnvVar **head, const char *name, const char *value, int is_system) {
    EnvVar *new_var = malloc(sizeof(EnvVar));
    if (!new_var) return;
    
    new_var->name = strdup(name);
    new_var->value = strdup(value);
    new_var->is_system = is_system;
    new_var->next = *head;
    *head = new_var;
}

EnvVar* find_env_var(EnvVar *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

void trim_whitespace(char *str) {
    if (!str) return;
    
    // Trim leading
    char *start = str;
    while (*start && isspace(*start)) start++;
    
    // Trim trailing
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    *(end + 1) = '\0';
    
    // Move trimmed string to beginning
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int parse_env_file(const char *filename, EnvVar **vars) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    }
    
    char line[1024];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Skip empty lines and comments
        trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        
        // Parse NAME=VALUE
        char *equals = strchr(line, '=');
        if (!equals) {
            continue;
        }
        
        *equals = '\0';
        char *name = line;
        char *value = equals + 1;
        
        trim_whitespace(name);
        trim_whitespace(value);
        
        // Remove quotes from value if present
        if ((value[0] == '"' || value[0] == '\'') && 
            value[strlen(value)-1] == value[0]) {
            value[strlen(value)-1] = '\0';
            value++;
        }
        
        add_env_var(vars, name, value, 0);
    }
    
    fclose(fp);
    return 0;
}

int cmd_env_inspect(const char *env_file) {
    EnvVar *vars = NULL;
    EnvVar *system_vars = NULL;
    
    // Load system environment variables
#ifdef _WIN32
    LPTCH env = GetEnvironmentStrings();
    if (env) {
        LPTSTR ptr = env;
        while (*ptr) {
            char *equals = strchr(ptr, '=');
            if (equals) {
                *equals = '\0';
                add_env_var(&system_vars, ptr, equals + 1, 1);
                *equals = '=';
            }
            ptr += strlen(ptr) + 1;
        }
        FreeEnvironmentStrings(env);
    }
#else
    extern char **environ;
    for (char **env_ptr = environ; *env_ptr != NULL; env_ptr++) {
        char *str = strdup(*env_ptr);
        char *equals = strchr(str, '=');
        if (equals) {
            *equals = '\0';
            add_env_var(&system_vars, str, equals + 1, 1);
        }
        free(str);
    }
#endif
    
    // Try to load .env file
    const char *filename = env_file ? env_file : ".env";
    int env_file_loaded = 0;
    
    if (parse_env_file(filename, &vars) == 0) {
        env_file_loaded = 1;
    }
    
    printf("=== Environment Inspector ===\n\n");
    
    if (env_file_loaded) {
        printf("📄 Loaded .env file: %s\n\n", filename);
        
        // Show .env variables and compare with system
        printf("Variables from %s:\n", filename);
        printf("%-30s %-40s %s\n", "Name", "Value", "Status");
        printf("%-30s %-40s %s\n", "----", "-----", "------");
        
        int count = 0;
        EnvVar *current = vars;
        while (current) {
            EnvVar *sys_var = find_env_var(system_vars, current->name);
            
            char truncated_value[41];
            if (strlen(current->value) > 40) {
                snprintf(truncated_value, sizeof(truncated_value), "%.37s...", current->value);
            } else {
                strncpy(truncated_value, current->value, sizeof(truncated_value));
                truncated_value[40] = '\0';
            }
            
            const char *status;
            if (!sys_var) {
                status = "❌ Not in system";
            } else if (strcmp(sys_var->value, current->value) == 0) {
                status = "✅ Matches system";
            } else {
                status = "⚠️  Different in system";
            }
            
            printf("%-30s %-40s %s\n", current->name, truncated_value, status);
            
            // Show difference if values don't match
            if (sys_var && strcmp(sys_var->value, current->value) != 0) {
                printf("  └─ System value: %s\n", sys_var->value);
            }
            
            count++;
            current = current->next;
        }
        
        printf("\nTotal .env variables: %d\n", count);
        
        // Show system-only variables that aren't in .env
        printf("\n--- Key System Variables Not in .env ---\n");
        printf("%-30s %s\n", "Name", "Value");
        printf("%-30s %s\n", "----", "-----");
        
        const char *important_vars[] = {
            "PATH", "HOME", "USER", "SHELL", "LANG", "PWD", 
            "TERM", "EDITOR", "HOSTNAME", "TMPDIR", "TMP", "TEMP",
            NULL
        };
        
        for (int i = 0; important_vars[i]; i++) {
            EnvVar *sys_var = find_env_var(system_vars, important_vars[i]);
            if (sys_var && !find_env_var(vars, important_vars[i])) {
                char truncated[81];
                if (strlen(sys_var->value) > 80) {
                    snprintf(truncated, sizeof(truncated), "%.77s...", sys_var->value);
                } else {
                    strncpy(truncated, sys_var->value, sizeof(truncated));
                    truncated[80] = '\0';
                }
                printf("%-30s %s\n", sys_var->name, truncated);
            }
        }
        
    } else {
        printf("⚠️  No .env file found: %s\n", filename);
        printf("Looking for .env file in current directory...\n\n");
        
        // Show subset of system variables
        printf("--- System Environment Variables (Key Variables) ---\n");
        printf("%-30s %s\n", "Name", "Value");
        printf("%-30s %s\n", "----", "-----");
        
        const char *important_vars[] = {
            "PATH", "HOME", "USER", "SHELL", "LANG", "PWD", 
            "TERM", "EDITOR", "HOSTNAME", "LOGNAME",
            NULL
        };
        
        for (int i = 0; important_vars[i]; i++) {
            char *value = getenv(important_vars[i]);
            if (value) {
                char truncated[81];
                if (strlen(value) > 80) {
                    snprintf(truncated, sizeof(truncated), "%.77s...", value);
                } else {
                    strncpy(truncated, value, sizeof(truncated));
                    truncated[80] = '\0';
                }
                printf("%-30s %s\n", important_vars[i], truncated);
            }
        }
        
        printf("\nTip: Create a .env file to track project-specific variables\n");
        printf("Example .env format:\n");
        printf("  DATABASE_URL=postgresql://localhost/mydb\n");
        printf("  API_KEY=your_secret_key_here\n");
        printf("  DEBUG=true\n");
    }
    
    free_env_vars(vars);
    free_env_vars(system_vars);
    
    return 0;
}

// Password generator
int cmd_passgen(int length, int flags) {
    if (length < 4 || length > 128) {
        fprintf(stderr, "Password length must be between 4 and 128\n");
        return 1;
    }
    
    if (flags == 0) {
        flags = PASS_ALL;
    }
    
    const char *alpha = "abcdefghijklmnopqrstuvwxyz";
    const char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *numbers = "0123456789";
    const char *special = "!@#$%^&*()-_=+[]{}|;:,.<>?";
    
    char charset[256] = "";
    int charset_len = 0;
    
    if (flags & PASS_ALPHA) {
        strcat(charset, alpha);
        charset_len += strlen(alpha);
    }
    if (flags & PASS_UPPER) {
        strcat(charset, upper);
        charset_len += strlen(upper);
    }
    if (flags & PASS_NUMBERS) {
        strcat(charset, numbers);
        charset_len += strlen(numbers);
    }
    if (flags & PASS_SPECIAL) {
        strcat(charset, special);
        charset_len += strlen(special);
    }
    
    if (charset_len == 0) {
        fprintf(stderr, "No character sets selected\n");
        return 1;
    }
    
    srand(time(NULL));
    
    char *password = malloc(length + 1);
    if (!password) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % charset_len];
    }
    password[length] = '\0';
    
    printf("Generated password: %s\n", password);
    printf("Length: %d characters\n", length);
    
    free(password);
    return 0;
}

// Find large files
typedef struct FileInfo {
    char path[1024];
    unsigned long long size;
    struct FileInfo *next;
} FileInfo;

void add_file(FileInfo **list, const char *path, unsigned long long size) {
    FileInfo *new_file = malloc(sizeof(FileInfo));
    if (!new_file) return;
    
    strncpy(new_file->path, path, sizeof(new_file->path) - 1);
    new_file->path[sizeof(new_file->path) - 1] = '\0';
    new_file->size = size;
    new_file->next = NULL;
    
    if (*list == NULL || size > (*list)->size) {
        new_file->next = *list;
        *list = new_file;
        return;
    }
    
    FileInfo *current = *list;
    while (current->next != NULL && current->next->size > size) {
        current = current->next;
    }
    
    new_file->next = current->next;
    current->next = new_file;
}

void scan_for_large_files(const char *path, unsigned long long min_size, FileInfo **list, int *count) {
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char search_path[MAX_PATH];
    
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    hFind = FindFirstFile(search_path, &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) return;
    
    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;
        
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s\\%s", path, findData.cFileName);
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_for_large_files(full_path, min_size, list, count);
        } else {
            ULARGE_INTEGER filesize;
            filesize.LowPart = findData.nFileSizeLow;
            filesize.HighPart = findData.nFileSizeHigh;
            
            if (filesize.QuadPart >= min_size) {
                add_file(list, full_path, filesize.QuadPart);
                (*count)++;
            }
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
#else
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
                scan_for_large_files(full_path, min_size, list, count);
            } else if (S_ISREG(st.st_mode)) {
                if ((unsigned long long)st.st_size >= min_size) {
                    add_file(list, full_path, st.st_size);
                    (*count)++;
                }
            }
        }
    }
    
    closedir(dir);
#endif
}

void format_large_size(unsigned long long size, char *buffer, size_t buffer_size) {
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

int cmd_findlarge(const char *path, unsigned long long min_size_mb) {
    unsigned long long min_size = min_size_mb * 1024 * 1024;
    
    printf("Scanning for files larger than %llu MB in: %s\n", min_size_mb, path);
    printf("This may take a while...\n\n");
    
    FileInfo *files = NULL;
    int count = 0;
    
    scan_for_large_files(path, min_size, &files, &count);
    
    if (count == 0) {
        printf("No files found larger than %llu MB\n", min_size_mb);
        return 0;
    }
    
    printf("Found %d large files:\n\n", count);
    printf("%-60s %15s\n", "Path", "Size");
    printf("%-60s %15s\n", "------------------------------------------------------------", "---------------");
    
    int displayed = 0;
    FileInfo *current = files;
    while (current != NULL && displayed < 50) {
        char size_str[50];
        format_large_size(current->size, size_str, sizeof(size_str));
        
        // Truncate path if too long
        char display_path[61];
        if (strlen(current->path) > 60) {
            snprintf(display_path, sizeof(display_path), "...%s", current->path + strlen(current->path) - 57);
        } else {
            strncpy(display_path, current->path, sizeof(display_path) - 1);
            display_path[60] = '\0';
        }
        
        printf("%-60s %15s\n", display_path, size_str);
        
        FileInfo *temp = current;
        current = current->next;
        free(temp);
        displayed++;
    }
    
    if (count > 50) {
        printf("\n... and %d more files\n", count - 50);
    }
    
    // Free remaining files
    while (current != NULL) {
        FileInfo *temp = current;
        current = current->next;
        free(temp);
    }
    
    return 0;
}
