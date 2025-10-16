#include "procman.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

int cmd_ps(void) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to create process snapshot\n");
        return 1;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    printf("%-8s %-40s\n", "PID", "Process Name");
    printf("%-8s %-40s\n", "--------", "----------------------------------------");
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            printf("%-8lu %-40s\n", pe32.th32ProcessID, pe32.szExeFile);
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return 0;
}

int cmd_kill(int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        fprintf(stderr, "Failed to open process %d\n", pid);
        return 1;
    }
    
    if (TerminateProcess(hProcess, 0)) {
        printf("Process %d terminated successfully\n", pid);
        CloseHandle(hProcess);
        return 0;
    } else {
        fprintf(stderr, "Failed to terminate process %d\n", pid);
        CloseHandle(hProcess);
        return 1;
    }
}

#elif defined(__linux__)
#include <dirent.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int cmd_ps(void) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        fprintf(stderr, "Failed to open /proc\n");
        return 1;
    }
    
    printf("%-8s %-40s\n", "PID", "Command");
    printf("%-8s %-40s\n", "--------", "----------------------------------------");
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;
        
        char cmdline_path[300];
        snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s/cmdline", entry->d_name);
        
        FILE *fp = fopen(cmdline_path, "r");
        if (!fp) continue;
        
        char cmdline[256] = {0};
        size_t len = fread(cmdline, 1, sizeof(cmdline) - 1, fp);
        fclose(fp);
        
        // Replace nulls with spaces
        for (size_t i = 0; i < len; i++) {
            if (cmdline[i] == '\0') cmdline[i] = ' ';
        }
        
        if (cmdline[0]) {
            printf("%-8s %-40s\n", entry->d_name, cmdline);
        }
    }
    
    closedir(dir);
    return 0;
}

int cmd_kill(int pid) {
    if (kill(pid, SIGTERM) == 0) {
        printf("Process %d terminated successfully\n", pid);
        return 0;
    } else {
        fprintf(stderr, "Failed to terminate process %d\n", pid);
        return 1;
    }
}

#else
int cmd_ps(void) {
    fprintf(stderr, "Process listing not supported on this platform\n");
    return 1;
}

int cmd_kill(int pid) {
    fprintf(stderr, "Process killing not supported on this platform\n");
    return 1;
}
#endif
