#include "flux.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>

typedef struct {
    WORD Red[256];
    WORD Green[256];
    WORD Blue[256];
} GAMMA_RAMP;

void set_gamma_ramp(int temperature) {
    HDC hdc = GetDC(NULL);
    if (hdc == NULL) {
        fprintf(stderr, "Failed to get device context\n");
        return;
    }
    
    GAMMA_RAMP ramp;
    
    // Calculate color temperature adjustment
    // Temperature range: 1000K (very warm) to 6500K (daylight)
    double temp = temperature;
    double red = 1.0, green = 1.0, blue = 1.0;
    
    if (temp < 6600) {
        // Calculate red
        if (temp > 6600) {
            red = 1.0;
        } else if (temp < 1000) {
            red = 1.0;
        } else {
            red = 1.0;
        }
        
        // Calculate green
        if (temp <= 6600) {
            green = temp / 100.0 - 2.0;
            green = -155.25485562709179 - 0.44596950469579133 * green + 104.49216199393888 * log(green);
            green = green / 255.0;
            if (green < 0) green = 0;
            if (green > 1) green = 1;
        } else {
            green = 1.0;
        }
        
        // Calculate blue
        if (temp >= 6600) {
            blue = 1.0;
        } else if (temp <= 2000) {
            blue = 0.0;
        } else {
            blue = temp / 100.0 - 10.0;
            blue = -254.76935184120902 + 0.8274096064007395 * blue + 115.67994401066147 * log(blue);
            blue = blue / 255.0;
            if (blue < 0) blue = 0;
            if (blue > 1) blue = 1;
        }
    } else {
        red = 1.0;
        green = 1.0;
        blue = 1.0;
    }
    
    // Build gamma ramp
    for (int i = 0; i < 256; i++) {
        ramp.Red[i] = (WORD)(i * 256 * red);
        ramp.Green[i] = (WORD)(i * 256 * green);
        ramp.Blue[i] = (WORD)(i * 256 * blue);
    }
    
    SetDeviceGammaRamp(hdc, &ramp);
    ReleaseDC(NULL, hdc);
}

void reset_gamma_ramp(void) {
    HDC hdc = GetDC(NULL);
    if (hdc == NULL) return;
    
    GAMMA_RAMP ramp;
    for (int i = 0; i < 256; i++) {
        ramp.Red[i] = i * 256;
        ramp.Green[i] = i * 256;
        ramp.Blue[i] = i * 256;
    }
    
    SetDeviceGammaRamp(hdc, &ramp);
    ReleaseDC(NULL, hdc);
}

#elif defined(__linux__)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static Display *display = NULL;

void set_gamma_ramp(int temperature) {
    if (display == NULL) {
        display = XOpenDisplay(NULL);
        if (display == NULL) {
            fprintf(stderr, "Cannot open X display\n");
            return;
        }
    }
    
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    
    XRRScreenResources *res = XRRGetScreenResources(display, root);
    if (res == NULL) {
        fprintf(stderr, "Failed to get screen resources\n");
        return;
    }
    
    // Calculate color temperature
    double temp = temperature;
    double red = 1.0, green = 1.0, blue = 1.0;
    
    if (temp < 6600) {
        // Red (always 1.0 for temps below 6600K)
        red = 1.0;
        
        // Green
        green = temp / 100.0;
        if (green > 60.0) {
            green = 1.2969 - 0.1332 * log(green - 60.0);
        } else {
            green = 0.39 * log(green) - 0.634;
        }
        if (green < 0) green = 0;
        if (green > 1) green = 1;
        
        // Blue
        if (temp <= 2000) {
            blue = 0.0;
        } else if (temp < 6600) {
            blue = (temp / 100.0 - 10.0);
            blue = 0.543 * log(blue) - 1.186;
            if (blue < 0) blue = 0;
            if (blue > 1) blue = 1;
        } else {
            blue = 1.0;
        }
    } else {
        red = 1.0;
        green = 1.0;
        blue = 1.0;
    }
    
    // Apply to all outputs
    for (int i = 0; i < res->ncrtc; i++) {
        XRRCrtcGamma *gamma = XRRGetCrtcGamma(display, res->crtcs[i]);
        if (gamma == NULL) continue;
        
        for (int j = 0; j < gamma->size; j++) {
            double value = (double)j / (gamma->size - 1);
            gamma->red[j] = (unsigned short)(value * 65535 * red);
            gamma->green[j] = (unsigned short)(value * 65535 * green);
            gamma->blue[j] = (unsigned short)(value * 65535 * blue);
        }
        
        XRRSetCrtcGamma(display, res->crtcs[i], gamma);
        XRRFreeGamma(gamma);
    }
    
    XRRFreeScreenResources(res);
    XFlush(display);
}

void reset_gamma_ramp(void) {
    if (display == NULL) {
        display = XOpenDisplay(NULL);
        if (display == NULL) return;
    }
    
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    
    XRRScreenResources *res = XRRGetScreenResources(display, root);
    if (res == NULL) return;
    
    for (int i = 0; i < res->ncrtc; i++) {
        XRRCrtcGamma *gamma = XRRGetCrtcGamma(display, res->crtcs[i]);
        if (gamma == NULL) continue;
        
        for (int j = 0; j < gamma->size; j++) {
            double value = (double)j / (gamma->size - 1);
            gamma->red[j] = (unsigned short)(value * 65535);
            gamma->green[j] = (unsigned short)(value * 65535);
            gamma->blue[j] = (unsigned short)(value * 65535);
        }
        
        XRRSetCrtcGamma(display, res->crtcs[i], gamma);
        XRRFreeGamma(gamma);
    }
    
    XRRFreeScreenResources(res);
    XFlush(display);
}

void cleanup_display(void) {
    if (display != NULL) {
        XCloseDisplay(display);
        display = NULL;
    }
}

#else
void set_gamma_ramp(int temperature) {
    fprintf(stderr, "Gamma adjustment not supported on this platform\n");
}
void reset_gamma_ramp(void) {}
void cleanup_display(void) {}
#endif

int get_temp_for_time(void) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int hour = local->tm_hour;
    
    // Daylight: 6500K (6AM-6PM)
    // Sunset transition: 6PM-10PM (6500K -> 3400K)
    // Night: 3400K (10PM-6AM)
    
    if (hour >= 6 && hour < 18) {
        // Daytime - neutral
        return 6500;
    } else if (hour >= 18 && hour < 22) {
        // Sunset transition
        int mins_since_18 = (hour - 18) * 60 + local->tm_min;
        int total_mins = 4 * 60; // 4 hours
        double progress = (double)mins_since_18 / total_mins;
        // Interpolate from 6500K to 3400K
        return (int)(6500 - progress * (6500 - 3400));
    } else {
        // Night - warm
        return 3400;
    }
}

const char* get_time_period(void) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int hour = local->tm_hour;
    
    if (hour >= 6 && hour < 18) return "Daytime";
    else if (hour >= 18 && hour < 22) return "Sunset";
    else return "Night";
}

int cmd_flux(int temp) {
    if (temp < 1000 || temp > 10000) {
        fprintf(stderr, "Temperature must be between 1000K and 10000K\n");
        fprintf(stderr, "Common values:\n");
        fprintf(stderr, "  6500K - Daylight (neutral)\n");
        fprintf(stderr, "  4500K - Warm daylight\n");
        fprintf(stderr, "  3400K - Warm/sunset\n");
        fprintf(stderr, "  2700K - Very warm/incandescent\n");
        return 1;
    }
    
    printf("Setting color temperature to %dK...\n", temp);
    set_gamma_ramp(temp);
    printf("Color temperature set!\n");
    printf("To reset: ./caffeinated flux reset\n");
    
    return 0;
}

int cmd_flux_auto(volatile int *running) {
    printf("Auto color temperature adjustment (f.lux mode)\n");
    printf("Press Ctrl+C to stop and reset...\n\n");
    
    while (*running) {
        int temp = get_temp_for_time();
        const char *period = get_time_period();
        
        // Clear line and print status
        printf("\r\033[K"); // Clear line
        printf("Period: %s | Temperature: %dK", period, temp);
        fflush(stdout);
        
        set_gamma_ramp(temp);
        
#ifdef _WIN32
        Sleep(60000); // Update every minute
#else
        sleep(60);
#endif
    }
    
    printf("\n\nResetting color temperature...\n");
    reset_gamma_ramp();
    
#ifdef __linux__
    cleanup_display();
#endif
    
    return 0;
}

int cmd_flux_reset(void) {
    printf("Resetting color temperature to normal...\n");
    reset_gamma_ramp();
    
#ifdef __linux__
    cleanup_display();
#endif
    
    printf("Color temperature reset!\n");
    return 0;
}
