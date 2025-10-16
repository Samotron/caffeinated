# Zig Build Migration Guide

Caffeinated now uses **Zig** as the primary build system for easy cross-compilation!

## Why Zig?

- **Zero dependencies**: No need to install MinGW, cross-compilers, or separate toolchains
- **True cross-compilation**: Build for Windows from Linux, Linux from Windows, etc.
- **Built-in C compiler**: Zig includes a full C/C++ compiler (uses Clang/LLVM)
- **Faster CI/CD**: Single setup step instead of multiple toolchain installations
- **Reproducible builds**: Same Zig version = same binary output

## Quick Commands

```bash
# Build for current platform
zig build

# Build optimized release
zig build -Doptimize=ReleaseFast

# Cross-compile to Windows
zig build -Dtarget=x86_64-windows-gnu -Doptimize=ReleaseFast

# Cross-compile to Linux
zig build -Dtarget=x86_64-linux-gnu -Doptimize=ReleaseFast

# Build and run
zig build run -- help

# Clean build artifacts
rm -rf zig-out zig-cache
```

## Output Location

- Built binaries: `zig-out/bin/`
- Build cache: `zig-cache/` and `.zig-cache/`

## GitHub Actions Changes

The workflow now:
1. Installs Zig once (via `goto-bus-stop/setup-zig@v2`)
2. Builds for both Linux and Windows from a single Ubuntu runner
3. No longer needs MinGW or separate Windows build steps

## Traditional Make Still Works

The `Makefile` is still available for those who prefer GCC/Make workflows:

```bash
make              # Build with GCC
make clean        # Clean build artifacts
```

## Zig Version

Recommended version: **0.13.0** (specified in `.zigversion`)

## Installation

### Linux/macOS
```bash
# Homebrew
brew install zig

# Or use ZVM (Zig Version Manager)
curl -sSL https://raw.githubusercontent.com/tristanisham/zvm/master/install.sh | bash
zvm install 0.13.0
```

### Windows
```powershell
# Chocolatey
choco install zig

# Or download from https://ziglang.org/download/
```

## Benefits for Contributors

1. **Easier cross-platform testing**: Build Windows binaries on Linux without dual-boot
2. **Consistent tooling**: Everyone uses the same compiler version
3. **Faster builds**: Zig's incremental compilation is very fast
4. **Better errors**: Clear, helpful error messages

## Technical Details

- The `build.zig` file defines all source files and dependencies
- Uses GNU99 C standard with POSIX extensions for Linux compatibility
- Automatically links platform-specific libraries (X11 for Linux, Win32 for Windows)
- Supports all Zig optimization levels: Debug, ReleaseSafe, ReleaseFast, ReleaseSmall

## Migrating Custom Builds

If you have custom build scripts using Make, they can easily be converted to Zig:

**Makefile flag** → **Zig equivalent**
- `-O2` → `-Doptimize=ReleaseFast`
- `-g` → `-Doptimize=Debug`
- `-DFOO=bar` → Add to `.flags` in `build.zig`
- Cross-compile → `-Dtarget=<arch>-<os>`

## Resources

- [Zig Build System](https://ziglang.org/learn/build-system/)
- [Zig Download](https://ziglang.org/download/)
- [Cross-compilation targets](https://ziglang.org/documentation/master/#Targets)
