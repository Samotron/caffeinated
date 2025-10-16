#include "nosleep.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>

int nosleep_init(void) {
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    return 0;
}

void nosleep_tick(void) {
    // On Windows, SetThreadExecutionState is continuous, just sleep a bit
    Sleep(1000);
}

void nosleep_cleanup(void) {
    SetThreadExecutionState(ES_CONTINUOUS);
}

#elif defined(__linux__)
#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>
#include <unistd.h>

static Display *display = NULL;

int nosleep_init(void) {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Warning: Cannot open X display. Sleep prevention may not work.\n");
        fprintf(stderr, "Try running in a terminal with X11 support.\n");
        return -1;
    }
    return 0;
}

void nosleep_tick(void) {
    if (display != NULL) {
        // Reset screensaver timer by simulating user activity
        XResetScreenSaver(display);
        XFlush(display);
    }
    sleep(1);
}

void nosleep_cleanup(void) {
    if (display != NULL) {
        XCloseDisplay(display);
        display = NULL;
    }
}

#else
#error "Unsupported platform"
#endif
