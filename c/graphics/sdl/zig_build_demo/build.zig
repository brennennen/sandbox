const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    b.install_path = ".build";

    const demo = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    demo.addCSourceFile(.{ .file = b.path("src/main.c"), .flags = &.{"-std=c11"} });

    if (target.result.os.tag == .windows) {
        const sdl_path = ".vendor/SDL3-3.4.0/x86_64-w64-mingw32";
        demo.addIncludePath(b.path(sdl_path ++ "/include"));
        demo.addObjectFile(b.path(sdl_path ++ "/lib/libSDL3.dll.a"));
        const install_dll = b.addInstallFileWithDir(
            b.path(sdl_path ++ "/bin/SDL3.dll"),
            .bin,
            "SDL3.dll",
        );
        b.getInstallStep().dependOn(&install_dll.step);
    } else {
        // todo:
        //demo.addLibraryPath(b.path(".vendor/SDL3/lib"));
        //demo.linkSystemLibrary("SDL3", .{});
    }

    const exe = b.addExecutable(.{
        .name = "demo",
        .root_module = demo,
    });

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    const run_step = b.step("run", "Run the engine");
    run_step.dependOn(&run_cmd.step);
}
