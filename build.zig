const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "caffeinated",
        .target = target,
        .optimize = optimize,
    });

    // Add all C source files
    exe.addCSourceFiles(.{
        .files = &.{
            "src/main.c",
            "src/nosleep.c",
            "src/animation.c",
            "src/sysinfo.c",
            "src/procman.c",
            "src/nettools.c",
            "src/filetools.c",
            "src/flux.c",
            "src/battery.c",
            "src/clipboard.c",
            "src/utils.c",
            "src/hash.c",
            "src/encoding.c",
            "src/timer.c",
            "src/converters.c",
            "src/text.c",
            "src/git.c",
            "src/network_ext.c",
        },
        .flags = &.{
            "-Wall",
            "-Wextra",
            "-O2",
            "-std=gnu99", // Use gnu99 instead of c99 for POSIX extensions
            "-D_POSIX_C_SOURCE=200809L", // Enable POSIX.1-2008
            "-D_XOPEN_SOURCE=700",        // Enable X/Open 7
            "-D_DEFAULT_SOURCE",          // Enable default features
        },
    });

    // Add include directory
    exe.addIncludePath(b.path("src"));

    // Platform-specific libraries
    if (target.result.os.tag == .windows) {
        exe.linkSystemLibrary("kernel32");
        exe.linkSystemLibrary("ws2_32");
        exe.linkSystemLibrary("iphlpapi");
        exe.linkSystemLibrary("psapi");
        exe.linkSystemLibrary("gdi32");
    } else if (target.result.os.tag == .linux) {
        exe.linkSystemLibrary("X11");
        exe.linkSystemLibrary("Xss");
        exe.linkSystemLibrary("Xrandr");
        exe.linkSystemLibrary("m");
    }

    exe.linkLibC();

    b.installArtifact(exe);

    // Create run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
