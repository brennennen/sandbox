const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    //b.install_path = ".build";

    const c_flags = &.{"-std=c23"};

    //
    // MARK: Core
    //
    // const core_include_paths = [_][]const u8{
    //     "libs",
    // };
    // const core_module = b.createModule(.{
    //     .target = target,
    //     .optimize = optimize,
    //     .link_libc = true,
    // });
    // const core_sources = [_][]const u8{
    //     "core/logger.c",
    // };
    // for (core_sources) |file| {
    //     core_module.addCSourceFile(.{ .file = b.path(file), .flags = c_flags });
    // }

    // for (core_include_paths) |path| {
    //     core_module.addIncludePath(b.path(path));
    // }
    // const core_lib = b.addLibrary(.{
    //     .linkage = .static,
    //     .name = "engine",
    //     .root_module = core_module,
    // });
    // b.installArtifact(core_lib);

    //
    // MARK: Engine
    //

    const include_paths = [_][]const u8{
        ".vendor/volk",
        ".vendor/Vulkan-Headers/include",
        ".vendor/VulkanMemoryAllocator/include",
        ".vendor/stb",
        ".vendor",
        "libs",
        "engine",
        "engine/modules",
        "engine/modules/graphics/include",
    };

    const engine_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const engine_sources = [_][]const u8{
        ".vendor/volk/volk.c",
        "libs/engine/core/math/mat4_math.c",
        "libs/engine/core/camera.c",
        "libs/engine/core/game_engine.c",
        "libs/engine/core/world.c",
        "libs/engine/core/vfs.c",
        "libs/engine/core/logger.c",
        "libs/engine/modules/graphics/graphics.c",
        "libs/engine/modules/graphics/debug/debug_grid.c",
        "libs/engine/modules/graphics/vulkan/vk_devices.c",
        "libs/engine/modules/graphics/vulkan/vk_backend.c",
        "libs/engine/modules/graphics/vulkan/vk_resources.c",
        "libs/engine/modules/graphics/vulkan/vk_swapchain.c",
        "libs/engine/modules/graphics/vulkan/vk_gpu_allocator.c",
        "libs/engine/modules/graphics/vulkan/vk_pipeline.c",
        "libs/engine/modules/graphics/vulkan/vk_commands.c",
        "libs/engine/modules/assets/image.c",
        "libs/engine/modules/assets/obj.c",
        "libs/engine/modules/assets/gltf.c",
        "libs/engine/platform/sdl/sdl_backend.c",
    };

    for (engine_sources) |file| {
        engine_mod.addCSourceFile(.{ .file = b.path(file), .flags = c_flags });
    }

    for (include_paths) |path| {
        engine_mod.addIncludePath(b.path(path));
    }

    if (target.result.os.tag == .windows) {
        engine_mod.addIncludePath(b.path(".vendor/Vulkan/include"));
        const sdl_path = ".vendor/SDL3-3.4.0/x86_64-w64-mingw32";
        engine_mod.addIncludePath(b.path(sdl_path ++ "/include"));
    }

    const engine_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "engine",
        .root_module = engine_mod,
    });
    b.installArtifact(engine_lib);

    if (target.result.os.tag == .windows) {
        const install_dll = b.addInstallFileWithDir(
            b.path(".vendor/SDL3-3.4.0/x86_64-w64-mingw32/bin/SDL3.dll"),
            .bin,
            "SDL3.dll",
        );
        b.getInstallStep().dependOn(&install_dll.step);
    }

    //
    // MARK: game_tools
    //
    const tools_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const tools_includes = [_][]const u8{
        ".vendor/stb",
        ".vendor",
        ".",
        "libs",
        "engine",
        "libs/tools/core",
        "libs/tools/bakers",
        "libs/tools/parsers",
    };

    const tools_sources = [_][]const u8{
        "libs/tools/bakers/gltf_baker.c",
        "libs/tools/parsers/scene_parser.c",
    };

    for (tools_sources) |file| {
        tools_mod.addCSourceFile(.{ .file = b.path(file), .flags = c_flags });
    }

    for (tools_includes) |path| {
        tools_mod.addIncludePath(b.path(path));
    }

    const tools_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "tools",
        .root_module = tools_mod,
    });
    b.installArtifact(tools_lib);

    //
    // MARK: Game
    //
    const game_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const game_includes = [_][]const u8{
        ".",
        "libs",
    };

    const games_sources = [_][]const u8{
        "apps/game/main.c",
    };

    for (games_sources) |file| {
        game_mod.addCSourceFile(.{ .file = b.path(file), .flags = c_flags });
    }

    for (game_includes) |path| {
        game_mod.addIncludePath(b.path(path));
    }

    const game_exe = b.addExecutable(.{
        .name = "game",
        .root_module = game_mod,
    });

    if (target.result.os.tag == .windows) {
        game_mod.addIncludePath(b.path(".vendor/Vulkan/include"));
        const sdl_path = ".vendor/SDL3-3.4.0/x86_64-w64-mingw32";
        game_mod.addIncludePath(b.path(sdl_path ++ "/include"));
        game_mod.addObjectFile(b.path(sdl_path ++ "/lib/libSDL3.dll.a"));
    }

    game_mod.linkLibrary(engine_lib);
    b.installArtifact(game_exe);

    const run_game_cmd = b.addRunArtifact(game_exe);
    run_game_cmd.step.dependOn(b.getInstallStep());
    run_game_cmd.cwd = b.path("zig-out/bin");

    const run_step = b.step("run", "Run the engine");
    run_step.dependOn(&run_game_cmd.step);

    //
    // MARK: Cooker
    //
    const cooker_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    cooker_mod.addCSourceFile(.{ .file = b.path("apps/cooker/main.c"), .flags = c_flags });

    for (include_paths) |path| {
        cooker_mod.addIncludePath(b.path(path));
    }

    const cooker_exe = b.addExecutable(.{
        .name = "cooker",
        .root_module = cooker_mod,
    });

    if (target.result.os.tag == .windows) {
        cooker_mod.addIncludePath(b.path(".vendor/Vulkan/include"));
        const sdl_path = ".vendor/SDL3-3.4.0/x86_64-w64-mingw32";
        cooker_mod.addIncludePath(b.path(sdl_path ++ "/include"));
        cooker_mod.addObjectFile(b.path(sdl_path ++ "/lib/libSDL3.dll.a"));
    }

    cooker_mod.linkLibrary(engine_lib);
    cooker_mod.linkLibrary(tools_lib);
    b.installArtifact(cooker_exe);

    const run_cooker_cmd = b.addRunArtifact(cooker_exe);
    run_cooker_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cooker_cmd.addArgs(args);
    }

    const cook_step = b.step("cook", "Run the level cooker");
    cook_step.dependOn(&run_cooker_cmd.step);

    //
    // MARK: Shaders
    //
    const shaders = [_][2][]const u8{
        .{ "shaders/triangle.vert", "shaders/triangle.vert.spv" },
        .{ "shaders/triangle.frag", "shaders/triangle.frag.spv" },
        .{ "shaders/debug_wireframe.frag", "shaders/debug_wireframe.frag.spv" },
        .{ "shaders/debug_lighting_only.frag", "shaders/debug_lighting_only.frag.spv" },
        .{ "shaders/debug_albedo.frag", "shaders/debug_albedo.frag.spv" },
        .{ "shaders/debug_normals.frag", "shaders/debug_normals.frag.spv" },
        .{ "shaders/debug_vertex_color.frag", "shaders/debug_vertex_color.frag.spv" },
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
}
