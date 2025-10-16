#include "timer.h"
#include <stdio.h>
#include <time.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

static volatile int timer_running = 1;

void timer_signal_handler(int signum) {
    (void)signum;
    timer_running = 0;
}

int cmd_timer(int seconds) {
    printf("Timer: %d seconds\n", seconds);
    printf("Press Ctrl+C to cancel\n\n");
    
    signal(SIGINT, timer_signal_handler);
    
    int remaining = seconds;
    while (remaining > 0 && timer_running) {
        int hours = remaining / 3600;
        int mins = (remaining % 3600) / 60;
        int secs = remaining % 60;
        
        printf("\r\033[KTime remaining: %02d:%02d:%02d", hours, mins, secs);
        fflush(stdout);
        
        sleep_ms(1000);
        remaining--;
    }
    
    if (timer_running) {
        printf("\r\033[K");
        printf("⏰ Timer complete! ⏰\n");
        
        // Beep
        printf("\a");
        fflush(stdout);
    } else {
        printf("\r\033[KTimer cancelled\n");
    }
    
    timer_running = 1;
    signal(SIGINT, SIG_DFL);
    return 0;
}

int cmd_stopwatch(void) {
    printf("Stopwatch started\n");
    printf("Press Ctrl+C to stop\n\n");
    
    signal(SIGINT, timer_signal_handler);
    
    time_t start = time(NULL);
    
    while (timer_running) {
        time_t elapsed = time(NULL) - start;
        int hours = elapsed / 3600;
        int mins = (elapsed % 3600) / 60;
        int secs = elapsed % 60;
        
        printf("\r\033[KElapsed: %02d:%02d:%02d", hours, mins, secs);
        fflush(stdout);
        
        sleep_ms(1000);
    }
    
    printf("\n\nStopwatch stopped\n");
    
    timer_running = 1;
    signal(SIGINT, SIG_DFL);
    return 0;
}

int cmd_pomodoro(void) {
    const int WORK_TIME = 25 * 60;  // 25 minutes
    const int BREAK_TIME = 5 * 60;   // 5 minutes
    
    printf("🍅 Pomodoro Timer 🍅\n");
    printf("Work: 25 minutes, Break: 5 minutes\n");
    printf("Press Ctrl+C to stop\n\n");
    
    signal(SIGINT, timer_signal_handler);
    
    int pomodoro_count = 0;
    
    while (timer_running) {
        // Work session
        pomodoro_count++;
        printf("Pomodoro #%d - Work time!\n", pomodoro_count);
        
        int remaining = WORK_TIME;
        while (remaining > 0 && timer_running) {
            int mins = remaining / 60;
            int secs = remaining % 60;
            
            printf("\r\033[K🍅 Work: %02d:%02d", mins, secs);
            fflush(stdout);
            
            sleep_ms(1000);
            remaining--;
        }
        
        if (!timer_running) break;
        
        printf("\r\033[K✅ Work session complete! Take a break.\n");
        printf("\a");
        
        // Break session
        remaining = BREAK_TIME;
        while (remaining > 0 && timer_running) {
            int mins = remaining / 60;
            int secs = remaining % 60;
            
            printf("\r\033[K☕ Break: %02d:%02d", mins, secs);
            fflush(stdout);
            
            sleep_ms(1000);
            remaining--;
        }
        
        if (!timer_running) break;
        
        printf("\r\033[K⏰ Break over! Ready for another pomodoro?\n");
        printf("\a");
        sleep_ms(2000);
    }
    
    printf("\n\nPomodoro session ended. Completed: %d pomodoros\n", pomodoro_count);
    
    timer_running = 1;
    signal(SIGINT, SIG_DFL);
    return 0;
}
