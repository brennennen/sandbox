const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    b.install_path = ".build";

    const game_engine = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const c_flags = &.{"-std=c23"};

    game_engine.addCSourceFile(.{ .file = b.path(".vendor/volk/volk.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/main.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/core/logger.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/core/math/mat4_math.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/core/camera.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/core/game_engine.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/graphics.c"), .flags = c_flags });

    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/debug/debug_grid.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/vulkan/vk_devices.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/vulkan/vk_backend.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/vulkan/vk_resources.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/vulkan/vk_swapchain.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/vulkan/vk_gpu_allocator.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/vulkan/vk_pipeline.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/graphics/vulkan/vk_commands.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/assets/image.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/assets/obj.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/modules/assets/gltf.c"), .flags = c_flags });
    //game_engine.addCSourceFile(.{ .file = b.path("src/platform/platform.c"), .flags = c_flags });
    game_engine.addCSourceFile(.{ .file = b.path("src/platform/sdl/sdl_backend.c"), .flags = c_flags });

    game_engine.addIncludePath(b.path(".vendor/volk"));
    game_engine.addIncludePath(b.path(".vendor/Vulkan-Headers/include"));
    game_engine.addIncludePath(b.path(".vendor/VulkanMemoryAllocator/include"));
    game_engine.addIncludePath(b.path(".vendor/stb/"));
    game_engine.addIncludePath(b.path(".vendor/"));
    game_engine.addIncludePath(b.path("src"));
    game_engine.addIncludePath(b.path("src/modules"));
    game_engine.addIncludePath(b.path("src/modules/graphics/include"));

    if (target.result.os.tag == .windows) {
        game_engine.addIncludePath(b.path(".vendor/Vulkan/include"));
        const sdl_path = ".vendor/SDL3-3.4.0/x86_64-w64-mingw32";
        game_engine.addIncludePath(b.path(sdl_path ++ "/include"));
        game_engine.addObjectFile(b.path(sdl_path ++ "/lib/libSDL3.dll.a"));
        const install_dll = b.addInstallFileWithDir(
            b.path(sdl_path ++ "/bin/SDL3.dll"),
            .bin,
            "SDL3.dll",
        );
        b.getInstallStep().dependOn(&install_dll.step);
    } else {
        // todo:
        //game_engine.addLibraryPath(b.path(".vendor/SDL3/lib"));
        //game_engine.linkSystemLibrary("SDL3", .{});
    }

    const shaders = [_][2][]const u8{
        .{ "shaders/triangle.vert", "shaders/triangle.vert.spv" },
        .{ "shaders/triangle.frag", "shaders/triangle.frag.spv" },
        .{ "shaders/debug_wireframe.frag", "shaders/debug_wireframe.frag.spv" },
        .{ "shaders/debug_lighting_only.frag", "shaders/debug_lighting_only.frag.spv" },
        .{ "shaders/debug_albedo.frag", "shaders/debug_albedo.frag.spv" },
        .{ "shaders/debug_normals.frag", "shaders/debug_normals.frag.spv" },
        .{ "shaders/unlit.vert", "shaders/unlit.vert.spv" },
        .{ "shaders/unlit.frag", "shaders/unlit.frag.spv" },
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
        .name = "game_engine",
        .root_module = game_engine,
    });

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    run_cmd.cwd = b.path("zig-out/bin");

    const run_step = b.step("run", "Run the engine");
    run_step.dependOn(&run_cmd.step);
}
