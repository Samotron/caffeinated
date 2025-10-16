#include "battery.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

int cmd_battery(void) {
    SYSTEM_POWER_STATUS sps;
    
    if (!GetSystemPowerStatus(&sps)) {
        fprintf(stderr, "Failed to get battery status\n");
        return 1;
    }
    
    printf("=== Battery Status ===\n\n");
    
    if (sps.BatteryFlag == 128) {
        printf("No battery detected\n");
        return 0;
    }
    
    printf("AC Power: %s\n", sps.ACLineStatus == 1 ? "Connected" : "Disconnected");
    
    if (sps.BatteryLifePercent <= 100) {
        printf("Battery Level: %d%%\n", sps.BatteryLifePercent);
        
        // Draw battery bar
        int bar_width = 30;
        int filled = (sps.BatteryLifePercent * bar_width) / 100;
        printf("Battery: [");
        for (int i = 0; i < bar_width; i++) {
            if (i < filled) printf("=");
            else printf(" ");
        }
        printf("]\n");
    }
    
    if (sps.BatteryLifeTime != -1) {
        int hours = sps.BatteryLifeTime / 3600;
        int minutes = (sps.BatteryLifeTime % 3600) / 60;
        printf("Time Remaining: %d:%02d\n", hours, minutes);
    }
    
    if (sps.BatteryFlag & 1) printf("Status: High\n");
    else if (sps.BatteryFlag & 2) printf("Status: Low\n");
    else if (sps.BatteryFlag & 4) printf("Status: Critical\n");
    else if (sps.BatteryFlag & 8) printf("Status: Charging\n");
    
    return 0;
}

int cmd_battery_monitor(volatile int *running) {
    printf("Battery Monitor - Press Ctrl+C to exit\n\n");
    
    while (*running) {
        SYSTEM_POWER_STATUS sps;
        if (GetSystemPowerStatus(&sps)) {
            printf("\033[2J\033[H"); // Clear screen
            printf("=== Battery Monitor ===\n\n");
            
            if (sps.BatteryFlag == 128) {
                printf("No battery detected\n");
                break;
            }
            
            printf("AC Power: %s\n", sps.ACLineStatus == 1 ? "Connected" : "Disconnected");
            if (sps.BatteryLifePercent <= 100) {
                printf("Battery Level: %d%%\n\n", sps.BatteryLifePercent);
                
                int bar_width = 50;
                int filled = (sps.BatteryLifePercent * bar_width) / 100;
                printf("[");
                for (int i = 0; i < bar_width; i++) {
                    if (i < filled) printf("=");
                    else printf(" ");
                }
                printf("]\n");
            }
        }
        
        Sleep(5000);
    }
    
    return 0;
}

#elif defined(__linux__)
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

int read_battery_info(char *status, int *capacity, int *energy_now, int *energy_full) {
    DIR *dir = opendir("/sys/class/power_supply");
    if (!dir) return -1;
    
    struct dirent *entry;
    char path[512];
    int found = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "BAT", 3) == 0) {
            // Found a battery
            snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", entry->d_name);
            FILE *fp = fopen(path, "r");
            if (fp) {
                fscanf(fp, "%s", status);
                fclose(fp);
            }
            
            snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", entry->d_name);
            fp = fopen(path, "r");
            if (fp) {
                fscanf(fp, "%d", capacity);
                fclose(fp);
            }
            
            snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_now", entry->d_name);
            fp = fopen(path, "r");
            if (fp) {
                fscanf(fp, "%d", energy_now);
                fclose(fp);
            }
            
            snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_full", entry->d_name);
            fp = fopen(path, "r");
            if (fp) {
                fscanf(fp, "%d", energy_full);
                fclose(fp);
            }
            
            found = 1;
            break;
        }
    }
    
    closedir(dir);
    return found ? 0 : -1;
}

int cmd_battery(void) {
    char status[32] = "Unknown";
    int capacity = 0;
    int energy_now = 0;
    int energy_full = 0;
    
    if (read_battery_info(status, &capacity, &energy_now, &energy_full) != 0) {
        printf("No battery detected or unable to read battery info\n");
        return 1;
    }
    
    printf("=== Battery Status ===\n\n");
    printf("Status: %s\n", status);
    printf("Battery Level: %d%%\n", capacity);
    
    // Draw battery bar
    int bar_width = 30;
    int filled = (capacity * bar_width) / 100;
    printf("Battery: [");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) printf("=");
        else printf(" ");
    }
    printf("]\n");
    
    if (energy_full > 0) {
        printf("Energy: %.2f Wh / %.2f Wh\n", 
               energy_now / 1000000.0, 
               energy_full / 1000000.0);
    }
    
    return 0;
}

int cmd_battery_monitor(volatile int *running) {
    printf("Battery Monitor - Press Ctrl+C to exit\n\n");
    
    while (*running) {
        char status[32] = "Unknown";
        int capacity = 0;
        int energy_now = 0;
        int energy_full = 0;
        
        if (read_battery_info(status, &capacity, &energy_now, &energy_full) != 0) {
            printf("No battery detected\n");
            break;
        }
        
        printf("\033[2J\033[H"); // Clear screen
        printf("=== Battery Monitor ===\n\n");
        printf("Status: %s\n", status);
        printf("Battery Level: %d%%\n\n", capacity);
        
        int bar_width = 50;
        int filled = (capacity * bar_width) / 100;
        printf("[");
        for (int i = 0; i < bar_width; i++) {
            if (i < filled) printf("=");
            else printf(" ");
        }
        printf("]\n");
        
        if (energy_full > 0) {
            printf("\nEnergy: %.2f Wh / %.2f Wh\n", 
                   energy_now / 1000000.0, 
                   energy_full / 1000000.0);
        }
        
        sleep(5);
    }
    
    return 0;
}

#else
int cmd_battery(void) {
    fprintf(stderr, "Battery info not supported on this platform\n");
    return 1;
}

int cmd_battery_monitor(volatile int *running) {
    fprintf(stderr, "Battery monitoring not supported on this platform\n");
    return 1;
}
#endif
