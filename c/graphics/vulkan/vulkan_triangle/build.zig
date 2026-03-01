const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    b.install_path = ".build";

    const vulkan_triangle = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const c_flags = &.{"-std=c23"};

    vulkan_triangle.addCSourceFile(.{ .file = b.path(".vendor/volk/volk.c"), .flags = c_flags });
    vulkan_triangle.addCSourceFile(.{ .file = b.path("src/main.c"), .flags = c_flags });
    vulkan_triangle.addCSourceFile(.{ .file = b.path("src/logger.c"), .flags = c_flags });
    vulkan_triangle.addCSourceFile(.{ .file = b.path("src/vk_backend.c"), .flags = c_flags });
    vulkan_triangle.addCSourceFile(.{ .file = b.path("src/sdl_backend.c"), .flags = c_flags });

    vulkan_triangle.addIncludePath(b.path(".vendor/volk"));
    vulkan_triangle.addIncludePath(b.path(".vendor/Vulkan-Headers/include"));
    vulkan_triangle.addIncludePath(b.path("src"));

    if (target.result.os.tag == .windows) {
        vulkan_triangle.addIncludePath(b.path(".vendor/Vulkan/include"));
        const sdl_path = ".vendor/SDL3-3.4.0/x86_64-w64-mingw32";
        vulkan_triangle.addIncludePath(b.path(sdl_path ++ "/include"));
        vulkan_triangle.addObjectFile(b.path(sdl_path ++ "/lib/libSDL3.dll.a"));
        const install_dll = b.addInstallFileWithDir(
            b.path(sdl_path ++ "/bin/SDL3.dll"),
            .bin,
            "SDL3.dll",
        );
        b.getInstallStep().dependOn(&install_dll.step);
    } else {
        // todo: linux support
    }

    const shaders = [_][2][]const u8{
        .{ "shaders/triangle.vert", "shaders/triangle.vert.spv" },
        .{ "shaders/triangle.frag", "shaders/triangle.frag.spv" },
    };
    for (shaders) |s| {
        const cmd = b.addSystemCommand(&.{"glslc"});
        cmd.addFileArg(b.path(s[0]));
        cmd.addArgs(&.{"-o"});
        const out_file = cmd.addOutputFileArg(s[1]);
        const install_shader = b.addInstallFileWithDir(out_file, .bin, s[1]);
        b.getInstallStep().dependOn(&install_shader.step);
    }

    const exe = b.addExecutable(.{
        .name = "vulkan_triangle",
        .root_module = vulkan_triangle,
    });

    b.installArtifact(exe);
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    run_cmd.cwd = b.path("zig-out/bin");
    const run_step = b.step("run", "Start vulkan_triangle");
    run_step.dependOn(&run_cmd.step);
}
