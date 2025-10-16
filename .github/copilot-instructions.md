# Copilot Instructions for Caffeinated

## Project Overview

Caffeinated is a cross-platform CLI power tools suite written in **pure C** (C99 standard). It provides 41+ system administration, development, and productivity commands in a single binary that runs on both Windows and Linux.

**Core Philosophy**: Minimalist, efficient, cross-platform, and dependency-free (except system libraries).

## Project Structure

```
caffeinated/
├── src/
│   ├── main.c           # Command dispatcher and entry point
│   ├── *.c/*.h          # Modular feature implementations
│   └── [19 modules]     # Each module = one feature category
├── Makefile             # Cross-platform build system
├── README.md            # User documentation
├── FEATURES.md          # Complete feature list
├── PLAN.md              # Development roadmap
└── PROJECT_SUMMARY.md   # Comprehensive overview
```

## Architecture & Design Patterns

### 1. Modular Design
- Each feature category is a separate module (e.g., `sysinfo.c`, `nettools.c`, `timer.c`)
- Each module has a corresponding header file
- Clean separation of concerns
- Easy to add new features by creating new modules

### 2. Cross-Platform Support
- Use preprocessor directives for platform-specific code:
  ```c
  #ifdef _WIN32
      // Windows implementation using Win32 API
  #elif defined(__linux__)
      // Linux implementation using POSIX/proc/sys
  #else
      #error "Unsupported platform"
  #endif
  ```
- Windows: Win32 API, GDI32, iphlpapi, ws2_32, Toolhelp32
- Linux: /proc, /sys, X11, XRandR, POSIX APIs

### 3. Command Pattern
- All commands follow the pattern: `int cmd_<feature>(args...)`
- Main dispatcher in `main.c` routes to appropriate command handlers
- Consistent error handling (return 0 for success, 1 for error)

## Coding Standards

### Style Guidelines
1. **Naming Conventions**:
   - Functions: `snake_case` (e.g., `cmd_battery_monitor`)
   - Structs: `PascalCase` (e.g., `EnvVar`, `FileInfo`)
   - Constants/Defines: `UPPER_SNAKE_CASE` (e.g., `PASS_ALL`)
   - Variables: `snake_case`

2. **Code Organization**:
   - Keep functions focused and under 100 lines when possible
   - Group related functions together
   - Use helper functions for repeated logic
   - Add comments for complex algorithms

3. **Error Handling**:
   - Always check return values from system calls
   - Use descriptive error messages with `fprintf(stderr, ...)`
   - Return appropriate error codes (0=success, 1=error)
   - Clean up resources (free memory, close files) on error paths

4. **Memory Management**:
   - Always `free()` what you `malloc()`
   - Use `strdup()` for string copies when appropriate
   - Check for NULL after allocation
   - Avoid memory leaks - use tools like valgrind for verification

### Platform-Specific Best Practices

**Windows**:
- Use `Sleep(milliseconds)` for delays
- Use `\` for paths (or use `/` which also works)
- Link necessary libraries in Makefile: `-lkernel32 -lws2_32 -lgdi32` etc.

**Linux**:
- Use `sleep(seconds)` or `usleep(microseconds)` for delays
- Use `/` for paths
- Link necessary libraries: `-lX11 -lXss -lXrandr -lm`

## Adding New Features

### Step-by-Step Guide

1. **Create Module Files**:
   ```bash
   # Create header file
   touch src/myfeature.h
   
   # Create implementation file
   touch src/myfeature.c
   ```

2. **Define Interface in Header** (`myfeature.h`):
   ```c
   #ifndef MYFEATURE_H
   #define MYFEATURE_H
   
   int cmd_myfeature(const char *arg);
   
   #endif
   ```

3. **Implement in Source** (`myfeature.c`):
   ```c
   #include "myfeature.h"
   #include <stdio.h>
   
   #ifdef _WIN32
   // Windows implementation
   #elif defined(__linux__)
   // Linux implementation
   #endif
   
   int cmd_myfeature(const char *arg) {
       // Implementation
       return 0;
   }
   ```

4. **Update Makefile**:
   ```makefile
   SOURCES = ... src/myfeature.c
   ```

5. **Add to Main Dispatcher** (`main.c`):
   ```c
   #include "myfeature.h"
   
   // In print_usage():
   printf("  myfeature <arg>      Description\n");
   
   // In main():
   if (strcmp(argv[1], "myfeature") == 0) {
       return cmd_myfeature(argv[2]);
   }
   ```

6. **Update Documentation**:
   - Add to README.md
   - Add to FEATURES.md
   - Update help text

## Testing Guidelines

### Manual Testing
- Test on both Windows and Linux if possible
- Test edge cases (empty input, NULL pointers, large values)
- Test error conditions (file not found, permission denied, etc.)
- Use valgrind on Linux to check for memory leaks:
  ```bash
  valgrind --leak-check=full ./caffeinated <command>
  ```

### Build Testing
```bash
# Clean build
make clean && make

# Test help
./caffeinated help

# Test specific command
./caffeinated <your_command>
```

## Common Patterns

### 1. Signal Handling for Long-Running Commands
```c
static volatile int running = 1;

void signal_handler(int signum) {
    (void)signum;
    running = 0;
}

int cmd_something(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    while (running) {
        // Do work
    }
    
    // Cleanup
    return 0;
}
```

### 2. Cross-Platform Sleep
```c
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif
```

### 3. String Trimming
```c
void trim_whitespace(char *str) {
    if (!str) return;
    
    // Trim leading
    char *start = str;
    while (*start && isspace(*start)) start++;
    
    // Trim trailing
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    *(end + 1) = '\0';
    
    // Move to beginning
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}
```

### 4. Linked List for Dynamic Data
```c
typedef struct Node {
    char *data;
    struct Node *next;
} Node;

void free_list(Node *head) {
    while (head) {
        Node *temp = head;
        head = head->next;
        free(temp->data);
        free(temp);
    }
}
```

## Current Modules Reference

1. **main.c** - Command dispatcher
2. **nosleep.c** - Prevent system sleep (X11/Win32)
3. **animation.c** - ASCII art dancing llama
4. **sysinfo.c** - System info and monitoring (/proc, Win32)
5. **procman.c** - Process management (Toolhelp32, /proc)
6. **nettools.c** - Ping, port checking (ICMP, sockets)
7. **network_ext.c** - Netstat (/proc/net/tcp, iphlpapi)
8. **filetools.c** - File operations (disk usage, duplicates)
9. **flux.c** - Color temperature (XRandR, Gamma Ramp)
10. **battery.c** - Battery monitoring (/sys, PowerStatus)
11. **clipboard.c** - Clipboard (xclip/xsel, Win32)
12. **utils.c** - Utilities (env, passgen, findlarge, envinspect)
13. **hash.c** - MD5 hashing (custom implementation)
14. **encoding.c** - Base64, UUID generation
15. **timer.c** - Timer, stopwatch, pomodoro
16. **converters.c** - Unit conversion, calculator
17. **text.c** - Lorem ipsum, case conversion
18. **git.c** - Git statistics (uses git commands)

## Performance Considerations

- **Minimize system calls** in loops
- **Buffer I/O** when reading large files
- **Use stack allocation** for small, fixed-size buffers
- **Avoid repeated string operations** - cache results
- **Use appropriate data structures** - linked lists for dynamic data, arrays for known sizes

## Security Considerations

1. **Input Validation**:
   - Always validate user input
   - Check array bounds
   - Validate file paths (prevent directory traversal)

2. **Buffer Safety**:
   - Use `strncpy` instead of `strcpy`
   - Use `snprintf` instead of `sprintf`
   - Check buffer sizes before writing

3. **Environment Variables**:
   - Be cautious with user-controlled env vars
   - Sanitize paths from environment

## Documentation Standards

### Code Comments
- Document complex algorithms
- Explain platform-specific quirks
- Note any limitations or TODOs
- Use clear, concise language

### User-Facing Documentation
- Update README.md with new features
- Add usage examples
- Document platform-specific requirements
- Keep FEATURES.md in sync

## Troubleshooting Common Issues

### Build Failures
- **Missing headers**: Install required dev packages (libx11-dev, etc.)
- **Linking errors**: Check Makefile LDFLAGS for correct libraries
- **Platform detection**: Ensure `#ifdef` directives are correct

### Runtime Issues
- **Segfaults**: Check for NULL pointers, buffer overflows
- **Permission errors**: Some features need sudo (ping on Linux)
- **Display issues**: Flux requires X11/XRandR on Linux

## Future Development Guidelines

### Potential Enhancements
- SHA256/SHA512 hash support (consider OpenSSL)
- HTTP client (libcurl or manual implementation)
- JSON/XML parsing (consider lightweight libraries)
- More compression utilities
- Database connectivity

### Maintaining Cross-Platform Compatibility
- Test on both platforms regularly
- Document platform differences
- Provide graceful fallbacks when features aren't available
- Keep dependencies minimal

## Quick Reference

### Build Commands
```bash
make                 # Build project
make clean          # Clean build artifacts
make clean && make  # Full rebuild
```

### Testing Commands
```bash
./caffeinated help                    # Show all commands
./caffeinated <cmd> --help           # Command help (where supported)
./caffeinated envinspect             # Useful for debugging env vars
```

### Important Files to Update
When adding features, update:
- [ ] `src/myfeature.c` and `src/myfeature.h`
- [ ] `src/main.c` (include, print_usage, dispatcher)
- [ ] `Makefile` (add to SOURCES)
- [ ] `README.md` (user docs)
- [ ] `FEATURES.md` (feature list)
- [ ] `.github/copilot-instructions.md` (this file, if needed)

## Code Review Checklist

Before submitting changes:
- [ ] Code compiles without warnings on both platforms
- [ ] Memory is properly managed (no leaks)
- [ ] Error cases are handled gracefully
- [ ] Cross-platform compatibility maintained
- [ ] Documentation updated
- [ ] Code follows project style guidelines
- [ ] Tested manually with various inputs
- [ ] Help text updated if new command added

---

**Remember**: Caffeinated is about being useful, efficient, and cross-platform. Keep it simple, keep it fast, keep it working on both Windows and Linux!
