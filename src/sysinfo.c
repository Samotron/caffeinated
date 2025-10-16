#include "sysinfo.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

void get_os_info(void) {
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    printf("OS: Windows\n");
}

void get_cpu_info(void) {
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    printf("CPU Cores: %u\n", siSysInfo.dwNumberOfProcessors);
}

void get_memory_info(void) {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    
    printf("Total RAM: %.2f GB\n", statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
    printf("Available RAM: %.2f GB\n", statex.ullAvailPhys / (1024.0 * 1024.0 * 1024.0));
    printf("Memory Usage: %lu%%\n", statex.dwMemoryLoad);
}

void get_cpu_usage(double *usage) {
    static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    static int initialized = 0;
    
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));

    GetSystemTimes(&ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));
    
    if (initialized) {
        double percent = (sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart);
        percent /= (now.QuadPart - lastCPU.QuadPart);
        percent *= 100;
        *usage = percent;
    } else {
        *usage = 0.0;
        initialized = 1;
    }
    
    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;
}

#elif defined(__linux__)
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void get_os_info(void) {
    struct utsname uts;
    if (uname(&uts) == 0) {
        printf("OS: %s %s\n", uts.sysname, uts.release);
        printf("Hostname: %s\n", uts.nodename);
    }
}

void get_cpu_info(void) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return;
    
    char line[256];
    char model[256] = "Unknown";
    int cores = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                sscanf(colon + 2, "%[^\n]", model);
            }
        }
        if (strncmp(line, "processor", 9) == 0) {
            cores++;
        }
    }
    fclose(fp);
    
    printf("CPU: %s\n", model);
    printf("CPU Cores: %d\n", cores);
}

void get_memory_info(void) {
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        double total = si.totalram * si.mem_unit / (1024.0 * 1024.0 * 1024.0);
        double free = si.freeram * si.mem_unit / (1024.0 * 1024.0 * 1024.0);
        double used = total - free;
        
        printf("Total RAM: %.2f GB\n", total);
        printf("Used RAM: %.2f GB\n", used);
        printf("Free RAM: %.2f GB\n", free);
        printf("Memory Usage: %.1f%%\n", (used / total) * 100);
    }
}

void get_cpu_usage(double *usage) {
    static unsigned long long lastTotalUser = 0, lastTotalUserLow = 0, lastTotalSys = 0, lastTotalIdle = 0;
    static int initialized = 0;
    
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        *usage = 0.0;
        return;
    }
    
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;
    fscanf(fp, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow, &totalSys, &totalIdle);
    fclose(fp);
    
    if (initialized) {
        if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
            totalSys < lastTotalSys || totalIdle < lastTotalIdle) {
            *usage = 0.0;
        } else {
            total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) + (totalSys - lastTotalSys);
            double totalTime = total + (totalIdle - lastTotalIdle);
            *usage = (total / totalTime) * 100.0;
        }
    } else {
        *usage = 0.0;
        initialized = 1;
    }
    
    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;
}

#else
void get_os_info(void) { printf("OS: Unknown\n"); }
void get_cpu_info(void) { printf("CPU: Unknown\n"); }
void get_memory_info(void) { printf("Memory: Unknown\n"); }
void get_cpu_usage(double *usage) { *usage = 0.0; }
#endif

int cmd_sysinfo(void) {
    printf("=== System Information ===\n\n");
    get_os_info();
    printf("\n");
    get_cpu_info();
    printf("\n");
    get_memory_info();
    return 0;
}

int cmd_monitor(volatile int *running) {
    printf("=== System Monitor ===\n");
    printf("Press Ctrl+C to exit\n\n");
    
    double cpu_usage;
    
    // Initialize CPU usage calculation
    get_cpu_usage(&cpu_usage);
    
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    
    while (*running) {
        get_cpu_usage(&cpu_usage);
        
        // Clear screen and move to top
        printf("\033[2J\033[H");
        printf("=== System Monitor === (Ctrl+C to exit)\n\n");
        
        printf("CPU Usage: %.1f%%\n", cpu_usage);
        
        // Draw CPU bar
        int bar_width = 50;
        int filled = (int)(cpu_usage / 100.0 * bar_width);
        printf("[");
        for (int i = 0; i < bar_width; i++) {
            if (i < filled) printf("=");
            else printf(" ");
        }
        printf("]\n\n");
        
        get_memory_info();
        
        fflush(stdout);
        
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }
    
    return 0;
}
