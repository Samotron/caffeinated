#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "nosleep.h"
#include "animation.h"
#include "sysinfo.h"
#include "procman.h"
#include "nettools.h"
#include "filetools.h"
#include "flux.h"
#include "battery.h"
#include "clipboard.h"
#include "utils.h"
#include "hash.h"
#include "encoding.h"
#include "timer.h"
#include "converters.h"
#include "text.h"
#include "git.h"
#include "network_ext.h"

static volatile int running = 1;

void signal_handler(int signum) {
    (void)signum;
    running = 0;
}

void print_usage(const char *progname) {
    printf("Caffeinated - Cross-Platform CLI Power Tools\n\n");
    printf("Usage: %s [command] [options]\n\n", progname);
    
    printf("System Monitoring:\n");
    printf("  sysinfo              Display system information\n");
    printf("  monitor              Real-time system resource monitor\n");
    printf("  ps                   List running processes\n");
    printf("  kill <pid>           Kill a process by PID\n");
    printf("  battery              Show battery status\n");
    printf("  battery monitor      Monitor battery in real-time\n");
    printf("\n");
    
    printf("Network Tools:\n");
    printf("  ping <host>          Ping a host\n");
    printf("  portcheck <port>     Check if a local port is open\n");
    printf("  netstat              Show network connections\n");
    printf("\n");
    
    printf("File Operations:\n");
    printf("  diskusage [path]     Show disk usage\n");
    printf("  finddup <path>       Find duplicate files\n");
    printf("  findlarge <path> <mb> Find files larger than size\n");
    printf("  hash <file> [algo]   Calculate file hash (md5)\n");
    printf("\n");
    
    printf("Display & Utilities:\n");
    printf("  flux <temp>          Set color temperature\n");
    printf("  flux auto            Auto-adjust by time of day\n");
    printf("  flux reset           Reset color temperature\n");
    printf("  nosleep              Keep system awake\n");
    printf("\n");
    
    printf("Encoding:\n");
    printf("  base64 encode <text> Encode text to base64\n");
    printf("  base64 decode <text> Decode base64 to text\n");
    printf("  uuid [count]         Generate UUIDs\n");
    printf("\n");
    
    printf("Productivity:\n");
    printf("  timer <seconds>      Countdown timer\n");
    printf("  stopwatch            Stopwatch\n");
    printf("  pomodoro             Pomodoro timer (25/5)\n");
    printf("\n");
    
    printf("Converters:\n");
    printf("  convert <val> <from> <to>  Unit converter\n");
    printf("  calc <expression>    Simple calculator\n");
    printf("\n");
    
    printf("Text Tools:\n");
    printf("  lorem <words>        Generate lorem ipsum\n");
    printf("  case <text> <type>   Convert case (upper/lower/title/camel/snake)\n");
    printf("\n");
    
    printf("Developer Tools:\n");
    printf("  gitstats [path]      Git repository statistics\n");
    printf("\n");
    
    printf("Other:\n");
    printf("  clipboard get        Get clipboard content\n");
    printf("  clipboard set <text> Set clipboard content\n");
    printf("  env [var]            Environment variables\n");
    printf("  envinspect [file]    Inspect .env file and compare with system\n");
    printf("  passgen <length>     Generate password\n");
    printf("  help                 Show this help\n");
    
    printf("\nPress Ctrl+C to stop long-running commands.\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "nosleep") == 0) {
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

        if (nosleep_init() != 0) {
            fprintf(stderr, "Failed to initialize nosleep functionality\n");
            return 1;
        }

        printf("Caffeinated activated! Your system will stay awake.\n");
        printf("Press Ctrl+C to stop...\n\n");

        animation_init();

        while (running) {
            nosleep_tick();
            animation_tick();
        }

        animation_cleanup();
        nosleep_cleanup();
        printf("\n\nCaffeinated deactivated. System can sleep normally now.\n");
        return 0;
    }

    if (strcmp(argv[1], "sysinfo") == 0) {
        return cmd_sysinfo();
    }

    if (strcmp(argv[1], "monitor") == 0) {
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        return cmd_monitor(&running);
    }

    if (strcmp(argv[1], "ps") == 0) {
        return cmd_ps();
    }

    if (strcmp(argv[1], "kill") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s kill <pid>\n", argv[0]);
            return 1;
        }
        return cmd_kill(atoi(argv[2]));
    }

    if (strcmp(argv[1], "ping") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s ping <host>\n", argv[0]);
            return 1;
        }
        return cmd_ping(argv[2]);
    }

    if (strcmp(argv[1], "portcheck") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s portcheck <port>\n", argv[0]);
            return 1;
        }
        return cmd_portcheck(atoi(argv[2]));
    }

    if (strcmp(argv[1], "diskusage") == 0) {
        const char *path = argc > 2 ? argv[2] : ".";
        return cmd_diskusage(path);
    }

    if (strcmp(argv[1], "finddup") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s finddup <path>\n", argv[0]);
            return 1;
        }
        return cmd_finddup(argv[2]);
    }

    if (strcmp(argv[1], "findlarge") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s findlarge <path> <size_in_mb>\n", argv[0]);
            fprintf(stderr, "Example: %s findlarge /home 100\n", argv[0]);
            return 1;
        }
        return cmd_findlarge(argv[2], atoll(argv[3]));
    }

    if (strcmp(argv[1], "flux") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s flux <temp|auto|reset>\n", argv[0]);
            fprintf(stderr, "Examples:\n");
            fprintf(stderr, "  %s flux 3400      # Set warm temperature\n", argv[0]);
            fprintf(stderr, "  %s flux 6500      # Set daylight temperature\n", argv[0]);
            fprintf(stderr, "  %s flux auto      # Auto-adjust by time\n", argv[0]);
            fprintf(stderr, "  %s flux reset     # Reset to normal\n", argv[0]);
            return 1;
        }

        if (strcmp(argv[2], "reset") == 0) {
            return cmd_flux_reset();
        } else if (strcmp(argv[2], "auto") == 0) {
            signal(SIGINT, signal_handler);
            signal(SIGTERM, signal_handler);
            return cmd_flux_auto(&running);
        } else {
            return cmd_flux(atoi(argv[2]));
        }
    }

    if (strcmp(argv[1], "battery") == 0) {
        if (argc > 2 && strcmp(argv[2], "monitor") == 0) {
            signal(SIGINT, signal_handler);
            signal(SIGTERM, signal_handler);
            return cmd_battery_monitor(&running);
        }
        return cmd_battery();
    }

    if (strcmp(argv[1], "clipboard") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s clipboard <get|set> [text]\n", argv[0]);
            return 1;
        }
        
        if (strcmp(argv[2], "get") == 0) {
            return cmd_clipboard_get();
        } else if (strcmp(argv[2], "set") == 0) {
            if (argc < 4) {
                fprintf(stderr, "Usage: %s clipboard set <text>\n", argv[0]);
                return 1;
            }
            // Concatenate all remaining args
            char text[4096] = "";
            for (int i = 3; i < argc; i++) {
                if (i > 3) strcat(text, " ");
                strcat(text, argv[i]);
            }
            return cmd_clipboard_set(text);
        } else {
            fprintf(stderr, "Unknown clipboard command: %s\n", argv[2]);
            return 1;
        }
    }

    if (strcmp(argv[1], "env") == 0) {
        const char *var = argc > 2 ? argv[2] : NULL;
        return cmd_env(var);
    }

    if (strcmp(argv[1], "envinspect") == 0) {
        const char *file = argc > 2 ? argv[2] : NULL;
        return cmd_env_inspect(file);
    }

    if (strcmp(argv[1], "passgen") == 0) {
        int length = 16;
        if (argc > 2) {
            length = atoi(argv[2]);
        }
        return cmd_passgen(length, PASS_ALL);
    }

    // New commands
    if (strcmp(argv[1], "hash") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s hash <file> [algorithm]\n", argv[0]);
            return 1;
        }
        const char *algo = argc > 3 ? argv[3] : "md5";
        return cmd_hash_file(argv[2], algo);
    }

    if (strcmp(argv[1], "base64") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s base64 <encode|decode> <text>\n", argv[0]);
            return 1;
        }
        if (strcmp(argv[2], "encode") == 0) {
            return cmd_base64_encode(argv[3], 0);
        } else if (strcmp(argv[2], "decode") == 0) {
            return cmd_base64_decode(argv[3], 0);
        }
        return 1;
    }

    if (strcmp(argv[1], "uuid") == 0) {
        int count = argc > 2 ? atoi(argv[2]) : 1;
        return cmd_uuid_generate(count);
    }

    if (strcmp(argv[1], "timer") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s timer <seconds>\n", argv[0]);
            return 1;
        }
        return cmd_timer(atoi(argv[2]));
    }

    if (strcmp(argv[1], "stopwatch") == 0) {
        return cmd_stopwatch();
    }

    if (strcmp(argv[1], "pomodoro") == 0) {
        return cmd_pomodoro();
    }

    if (strcmp(argv[1], "convert") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Usage: %s convert <value> <from> <to>\n", argv[0]);
            fprintf(stderr, "Example: %s convert 100 f c\n", argv[0]);
            return 1;
        }
        return cmd_convert_unit(argv[2], argv[3], argv[4]);
    }

    if (strcmp(argv[1], "calc") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s calc <expression>\n", argv[0]);
            fprintf(stderr, "Example: %s calc \"10 + 5\"\n", argv[0]);
            return 1;
        }
        return cmd_calc(argv[2]);
    }

    if (strcmp(argv[1], "lorem") == 0) {
        int words = argc > 2 ? atoi(argv[2]) : 50;
        return cmd_lorem(words);
    }

    if (strcmp(argv[1], "case") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s case <text> <type>\n", argv[0]);
            fprintf(stderr, "Types: upper, lower, title, camel, snake\n");
            return 1;
        }
        return cmd_case_convert(argv[2], argv[3]);
    }

    if (strcmp(argv[1], "gitstats") == 0) {
        const char *path = argc > 2 ? argv[2] : ".";
        return cmd_git_stats(path);
    }

    if (strcmp(argv[1], "netstat") == 0) {
        return cmd_netstat();
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}
