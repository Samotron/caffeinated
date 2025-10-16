# Caffeinated

A cross-platform C-based CLI power tools suite for system administration.

## Implemented Features

### ✅ System Monitoring & Info
- **sysinfo** - Display OS, CPU, hostname, and memory information
- **monitor** - Real-time CPU and memory usage with visual bars
- **ps** - List all running processes with PIDs and command names
- **kill** - Terminate processes by PID
- **battery** - Battery status, level, and health monitoring
- **battery monitor** - Real-time battery monitoring with visual indicator

### ✅ Network Tools
- **ping** - ICMP ping with latency measurements (sudo required on Linux)
- **portcheck** - Check if local ports are in use

### ✅ File Operations
- **diskusage** - Display disk space usage with human-readable sizes
- **finddup** - Find duplicate files by size in directory trees
- **findlarge** - Find files larger than specified size (in MB)

### ✅ Display & Utilities
- **flux** - Screen color temperature adjustment (like f.lux/Redshift)
  - Manual temperature setting (1000K-10000K)
  - Auto mode: adjusts by time of day (6500K daytime → 3400K night)
  - Smooth transitions during sunset hours
  - Reduces blue light and eye strain at night
- **nosleep** - Keep system awake with animated dancing llama and timer

### ✅ Other Tools
- **clipboard get/set** - Read from and write to system clipboard
- **env** - List all environment variables or query specific ones
- **passgen** - Generate secure random passwords with configurable length

## Architecture

Modular design with cross-platform support:
- Platform detection via preprocessor directives (#ifdef _WIN32, __linux__)
- Windows: Win32 API (Toolhelp32, ICMP, SetThreadExecutionState, Gamma Ramp, Clipboard API)
- Linux: /proc filesystem, X11, XRandR, raw sockets, /sys/class/power_supply, xclip

## Components

All features implemented across 11 modules:
- `main.c` - Command dispatcher and argument parsing
- `nosleep.c` - Sleep prevention via X11/Win32 APIs
- `animation.c` - ASCII art dancing llama animation
- `sysinfo.c` - System information and real-time monitoring
- `procman.c` - Process listing and management
- `nettools.c` - Network utilities (ping, port checking)
- `filetools.c` - File operations (disk usage, duplicates)
- `flux.c` - Color temperature control with time-based automation
- `battery.c` - Battery status and monitoring for laptops
- `clipboard.c` - Cross-platform clipboard access
- `utils.c` - Utility functions (env vars, password gen, large file finder)

## Future Enhancement Ideas

### Advanced Monitoring
- CPU/GPU temperature sensors
- Network bandwidth monitor with graphs
- Disk I/O statistics
- Per-process resource usage

### Enhanced Network Tools
- Port scanner (range scanning)
- DNS lookup and reverse DNS
- Network interface info and stats
- Speed test utility
- TCP/UDP connection viewer

### Enhanced Display Features
- Per-monitor color temperature control
- Saved color profiles
- Location-based sunset calculation (GPS/timezone)
- Brightness control
- Blue light filter percentage

### File Tools Enhancements
- Hash-based duplicate detection (MD5/SHA256/CRC32)
- Bulk file renaming with patterns
- Directory tree visualization
- File integrity checker
- Symlink resolver

### System Utilities
- Service/daemon manager
- Scheduled tasks viewer/editor
- System uptime and reboot history
- Log file viewer with filtering
- Startup programs manager

### Security Tools
- Password strength analyzer
- Password manager (encrypted storage)
- File encryption/decryption (AES)
- Permission checker and fixer
- Hash calculator for files
- Basic firewall rule viewer

### Performance Tools
- CPU/disk/memory benchmark
- Process priority manager
- Cache cleaner
- Memory optimizer
- Boot time analyzer
