#include "animation.h"
#include <stdio.h>
#include <time.h>

static const char *llama_frames[] = {
    "    \\  /\n"
    "     \\/  \n"
    "    (oo) \n"
    "   /|  |\\ \n"
    "  / |  | \\\n"
    "    ^  ^  ",

    "    \\  /\n"
    "     \\/  \n"
    "    (oo) \n"
    "   /|  |\\ \n"
    "    |  |  \n"
    "    ^  ^  ",

    "    \\  /\n"
    "     \\/  \n"
    "    (oo) \n"
    "    |  |  \n"
    "   /|  |\\ \n"
    "    ^  ^  ",

    "    \\  /\n"
    "     \\/  \n"
    "    (oo) \n"
    "    |  |  \n"
    "    |  |  \n"
    "    ^  ^  "
};

static const int num_frames = 4;
static int current_frame = 0;
static time_t start_time;
static int tick_count = 0;

void animation_init(void) {
    current_frame = 0;
    tick_count = 0;
    start_time = time(NULL);
}

void animation_tick(void) {
    // Clear previous frame (move cursor up 6 lines and clear)
    if (tick_count > 0) {
        printf("\033[6A"); // Move cursor up 6 lines
    }
    
    // Print current frame
    printf("%s", llama_frames[current_frame]);
    
    // Print status
    int elapsed = (int)(time(NULL) - start_time);
    int hours = elapsed / 3600;
    int minutes = (elapsed % 3600) / 60;
    int seconds = elapsed % 60;
    printf("\r\nAwake for: %02d:%02d:%02d", hours, minutes, seconds);
    
    fflush(stdout);
    
    current_frame = (current_frame + 1) % num_frames;
    tick_count++;
}

void animation_cleanup(void) {
    printf("\n");
}
