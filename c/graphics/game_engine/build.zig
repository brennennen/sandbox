const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    //b.install_path = ".build";

    const c_flags = &.{
        "-std=c23",
        "-fno-sanitize=alignment", // needed for stb_image_resize2
    };

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
        ".",
        "shared",
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
        "libs/engine/core/math/mat4.c",
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
        ".vendor/bc7",
        ".vendor",
        ".",
        "shared",
        "libs",
        "engine",
        "libs/tools/core",
        "libs/tools/bakers",
        "libs/tools/parsers",
    };

    const tools_sources = [_][]const u8{
        ".vendor/bc7/bc7enc.c",
        "libs/tools/core/mesh_utilities.c",
        "libs/tools/core/tools_core.c",
        "libs/tools/bakers/gltf_baker.c",
        "libs/tools/parsers/asset_parser.c",
        "libs/tools/parsers/scene_parser.c",
        "libs/tools/importers/gltf_extract.c",
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
        "shared",
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
    // MARK: GLTF Importer
    //
    const gltf_importer_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    gltf_importer_mod.addCSourceFile(.{ .file = b.path("apps/gltf_importer/main.c"), .flags = c_flags });

    for (include_paths) |path| {
        gltf_importer_mod.addIncludePath(b.path(path));
    }

    const gltf_importer_exe = b.addExecutable(.{
        .name = "gltf_importer",
        .root_module = gltf_importer_mod,
    });

    if (target.result.os.tag == .windows) {
        gltf_importer_mod.addIncludePath(b.path(".vendor/Vulkan/include"));
        const sdl_path = ".vendor/SDL3-3.4.0/x86_64-w64-mingw32";
        gltf_importer_mod.addIncludePath(b.path(sdl_path ++ "/include"));
        gltf_importer_mod.addObjectFile(b.path(sdl_path ++ "/lib/libSDL3.dll.a"));
    }

    gltf_importer_mod.linkLibrary(engine_lib);
    gltf_importer_mod.linkLibrary(tools_lib);
    b.installArtifact(gltf_importer_exe);

    const run_gltf_importer_cmd = b.addRunArtifact(gltf_importer_exe);
    run_gltf_importer_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_gltf_importer_cmd.addArgs(args);
    }

    const gltf_import_step = b.step("gltf_import", "Run the gltf importer");
    gltf_import_step.dependOn(&run_gltf_importer_cmd.step);

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
        .{ "shaders/core/mesh.vert", "shaders/core/mesh.vert.spv" },
        .{ "shaders/core/pbr.frag", "shaders/core/pbr.frag.spv" },
        .{ "shaders/core/skybox.vert", "shaders/core/skybox.vert.spv" },
        .{ "shaders/core/skybox.frag", "shaders/core/skybox.frag.spv" },
        .{ "shaders/core/debug_pbr.frag", "shaders/core/debug_pbr.frag.spv" },
        .{ "shaders/core/debug_wireframe.frag", "shaders/core/debug_wireframe.frag.spv" },
        .{ "shaders/core/line.vert", "shaders/core/line.vert.spv" },
        .{ "shaders/core/line.frag", "shaders/core/line.frag.spv" },
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
