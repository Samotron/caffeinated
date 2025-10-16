# Caffeinated ☕

A powerful cross-platform C-based CLI toolkit packed with 40+ system administration, development, and productivity tools.

## Features Summary

**System Monitoring** • **Network Tools** • **File Operations** • **Display Utilities** • **Encoding & Security** • **Productivity Timers** • **Converters & Calculator** • **Text Tools** • **Developer Tools**

Over 40 commands in ~3,300 lines of C code, fully cross-platform (Windows & Linux).

## Quick Start

```bash
make                                    # Build
./caffeinated help                      # Show all commands
./caffeinated sysinfo                   # System info
./caffeinated timer 300                 # 5 min timer
./caffeinated flux auto                 # Auto color temp
./caffeinated uuid 3                    # Generate UUIDs
./caffeinated convert 100 f c           # Convert units
```

## All Commands

### System Monitoring
- `sysinfo` - Display OS, CPU, memory information
- `monitor` - Real-time CPU/memory monitor
- `ps` - List running processes
- `kill <pid>` - Terminate process
- `battery` - Battery status
- `battery monitor` - Real-time battery monitoring

### Network Tools
- `ping <host>` - ICMP ping with latency
- `portcheck <port>` - Check if port is open
- `netstat` - Show network connections

### File Operations
- `diskusage [path]` - Disk usage statistics
- `finddup <path>` - Find duplicate files
- `findlarge <path> <mb>` - Find large files
- `hash <file>` - Calculate MD5 hash

### Display & Utilities
- `flux <temp>` - Set color temperature (1000K-10000K)
- `flux auto` - Auto-adjust by time of day
- `flux reset` - Reset to normal
- `nosleep` - Keep system awake with dancing llama

### Encoding & Security
- `base64 encode/decode <text>` - Base64 encoding/decoding
- `uuid [count]` - Generate UUIDs (v4)
- `passgen <length>` - Generate secure passwords

### Productivity
- `timer <seconds>` - Countdown timer with alarm
- `stopwatch` - Track elapsed time
- `pomodoro` - Pomodoro timer (25 min / 5 min)

### Converters
- `convert <val> <from> <to>` - Unit converter
  - Temperature: c, f, k
  - Length: m, km, cm, mi, ft, in
  - Data: b, kb, mb, gb, tb
- `calc <expression>` - Calculator (+, -, *, /, %, ^)

### Text Tools
- `lorem <words>` - Generate lorem ipsum
- `case <text> <type>` - Convert case (upper/lower/title/camel/snake)

### Developer Tools
- `gitstats [path]` - Git repository statistics
- `clipboard get/set` - Clipboard operations
- `env [var]` - View environment variables

## Building

### Using Zig (Recommended - Easy Cross-Compilation)

**Install Zig:**
```bash
# Linux/macOS
brew install zig  # or download from https://ziglang.org/download/

# Or use version manager
curl -sSL https://raw.githubusercontent.com/tristanisham/zvm/master/install.sh | bash
zvm install 0.13.0
```

**Build for your platform:**
```bash
zig build                          # Build for current platform
zig build run                      # Build and run
```

**Cross-compile:**
```bash
# Linux x86_64
zig build -Dtarget=x86_64-linux-gnu -Doptimize=ReleaseFast

# Windows x86_64
zig build -Dtarget=x86_64-windows-gnu -Doptimize=ReleaseFast

# macOS (requires macOS SDK headers)
zig build -Dtarget=x86_64-macos -Doptimize=ReleaseFast
```

Binaries are output to `zig-out/bin/`

**Note:** Linux builds require X11 headers installed on the build machine:
```bash
sudo apt-get install libx11-dev libxss-dev libxrandr-dev
```

### Using Make (Traditional)

**Linux:**
```bash
make
```
Requirements: `gcc`, `libx11-dev`, `libxss-dev`, `libxrandr-dev`, `xclip`

```bash
sudo apt-get install libx11-dev libxss-dev libxrandr-dev xclip
```

**Windows:**
```bash
make
```
Requirements: MinGW or similar GCC toolchain

## Usage Examples

```bash
# System monitoring
./caffeinated sysinfo
./caffeinated monitor
./caffeinated battery
./caffeinated netstat
./caffeinated ps | grep firefox

# File operations
./caffeinated diskusage /home
./caffeinated findlarge /home 100
./caffeinated hash README.md
./caffeinated finddup ~/Downloads

# Productivity
./caffeinated timer 300          # 5 minute timer
./caffeinated stopwatch
./caffeinated pomodoro           # 25/5 pomodoro

# Encoding & security
./caffeinated base64 encode "Hello World"
./caffeinated base64 decode "SGVsbG8gV29ybGQ="
./caffeinated uuid 5             # Generate 5 UUIDs
./caffeinated passgen 24         # 24-char password

# Converters
./caffeinated convert 100 f c    # 100°F to Celsius
./caffeinated convert 1024 mb gb # MB to GB
./caffeinated convert 5 mi km    # Miles to kilometers
./caffeinated calc "42 * 1.5"    # Calculator

# Text tools
./caffeinated lorem 100          # 100 words lorem ipsum
./caffeinated case "hello world" title
./caffeinated case "HelloWorld" snake
./caffeinated clipboard get
./caffeinated clipboard set "Copied text!"

# Developer tools
./caffeinated gitstats           # Git repo stats
./caffeinated env PATH           # View PATH variable
./caffeinated env                # List all variables

# Display utilities
./caffeinated flux 3400          # Warm color temp
./caffeinated flux auto          # Auto by time
./caffeinated nosleep            # Keep awake
```

## Color Temperature (Flux)

Reduce eye strain by adjusting screen warmth:

- **6500K** - Daylight (neutral white) - default
- **4500K** - Warm daylight
- **3400K** - Sunset/evening (recommended)
- **2700K** - Incandescent bulb (very warm)

Auto mode schedule:
- 6AM-6PM: 6500K (daylight)
- 6PM-10PM: Gradual transition
- 10PM-6AM: 3400K (warm/night)

## Platform Support

- **Linux**: Full support (X11 required for flux/nosleep)
- **Windows**: Full support

## Architecture

19 modular components, 3,300+ lines of C:

- `main.c` - Command dispatcher
- `sysinfo.c` - System info & monitoring
- `procman.c` - Process management
- `nettools.c` - Ping, port checking
- `network_ext.c` - Netstat
- `filetools.c` - File operations
- `flux.c` - Color temperature
- `battery.c` - Battery monitoring
- `nosleep.c` - Sleep prevention
- `animation.c` - ASCII art
- `clipboard.c` - Clipboard ops
- `hash.c` - MD5 hashing
- `encoding.c` - Base64, UUID
- `timer.c` - Timers & pomodoro
- `converters.c` - Unit conversion
- `text.c` - Text generation
- `git.c` - Git statistics
- `utils.c` - Utilities

Each module provides cross-platform implementations using preprocessor directives.

## License

Open source - see repository for details.

## Environment Inspector

The `envinspect` command is a powerful tool for managing environment variables in development:

```bash
# Inspect .env file in current directory
./caffeinated envinspect

# Inspect specific env file
./caffeinated envinspect production.env
```

**Features:**
- ✅ Parses `.env` files with support for comments and quoted values
- ✅ Compares .env variables with system environment
- ✅ Shows which variables match, differ, or are missing
- ✅ Displays actual system values for comparison
- ✅ Lists important system variables not in .env
- ✅ Helpful for debugging environment configuration issues

**Status Indicators:**
- ✅ **Matches system** - Variable exists in both with same value
- ⚠️ **Different in system** - Variable exists but values differ
- ❌ **Not in system** - Variable only in .env file

**Example .env format:**
```bash
# Database
DATABASE_URL=postgresql://localhost:5432/mydb
DB_USER=admin

# API Keys
API_KEY=your_secret_key_here
STRIPE_KEY="pk_test_123"  # Quotes are supported

# Settings
DEBUG=true
PORT=3000
```
