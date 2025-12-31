
-- "export compile commands" - used for clangd highlighting/code navigation
require "ecc/ecc"

-- MARK: Workspace
workspace "EmulateWorkspace"
    configurations { "Debug", "Release" }
    location ".build"
    defines { "DO_LOG" }
    buildoptions { "-std=c23" }

    -- symbols "On"
    filter "configurations:Debug"
        defines { "DEBUG", "DO_DEBUG_LOG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
    filter { "system:linux", "action:gmake" }
        toolset "gcc"
        sanitize { "Address" }
        links { "m" }
    filter { "system:windows", "action:gmake" }
        toolset "gcc"
        defines { "CRITERION_DLL" }
        includedirs { "C:/include" }
        libdirs     { "C:/libs", "C:/lib" }
        links { "m" }
        postbuildcommands {
            "cp C:/bin/libcriterion-3.dll %{cfg.targetdir}"
        }

-- MARK: Libraries
project "logger"
    kind "StaticLib"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files {
        "./libraries/logger/include/**.h",
        "./libraries/logger/source/**.c"
    }
    includedirs {
        "./libraries/logger/include",
        "./shared/include",
        "."
    }

project "device_tree"
    kind "StaticLib"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files {
        "./libraries/device_tree/include/**.h",
        "./libraries/device_tree/source/**.c"
    }
    includedirs {
        "./libraries/device_tree/include",
        "./shared/include",
        "."
    }

project "emulate_intel"
    kind "StaticLib"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files {
        "./libraries/emulate_intel/include/**.h",
        "./libraries/emulate_intel/source/**.c"
    }
    includedirs {
        "./libraries/emulate_intel/include",
        "./libraries/logger/include",
        "./shared/include",
        "."
    }
    links { "logger" }

project "emulate_arm"
    kind "StaticLib"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files {
        "./libraries/emulate_arm/include/**.h",
        "./libraries/emulate_arm/source/**.c"
    }
    includedirs {
        "./libraries/emulate_arm/include",
        "./libraries/logger/include",
        "./shared/include",
        "."
    }
    links { "logger" }

project "emulate_riscv"
    kind "StaticLib"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files {
        "./libraries/emulate_riscv/include/**.h",
        "./libraries/emulate_riscv/source/**.c"
    }
    includedirs {
        "./libraries/emulate_riscv/include",
        "./libraries/logger/include",
        "./shared/include",
        "."
    }
    links { "logger" }

-- MARK: Applications
project "test_emulate_intel"
    kind "ConsoleApp"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files {
        "./libraries/emulate_intel/test/**.h",
        "./libraries/emulate_intel/test/**.c"
    }
    includedirs {
        "./libraries/logger/include",
        "./libraries/emulate_intel/include",
        "./shared/include",
        "."
    }
    links {
        "emulate_intel",
        "logger",
        "criterion"
    }

project "test_emulate_arm"
    kind "ConsoleApp"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files {
        "./libraries/emulate_arm/test/**.h",
        "./libraries/emulate_arm/test/**.c"
    }
    includedirs {
        "./libraries/logger/include",
        "./libraries/emulate_arm/include",
        "./shared/include",
        "."
    }
    links {
        "emulate_arm",
        "logger",
        "criterion"
    }

project "test_emulate_riscv"
    kind "ConsoleApp"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files {
        "./libraries/emulate_riscv/test/**.h",
        "./libraries/emulate_riscv/test/**.c"
    }
    includedirs {
        "./libraries/logger/include",
        "./libraries/emulate_riscv/include",
        "./shared/include",
        "."
    }
    links {
        "emulate_riscv",
        "logger",
        "criterion"
    }

project "disassemble"
    kind "ConsoleApp"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files { "./apps/disassemble/**.h", "./apps/disassemble/**.c" }
    includedirs {
        "./libraries/logger/include",
        "./libraries/emulate_riscv/include",
        "./libraries/emulate_arm/include",
        "./libraries/emulate_intel/include",
        "./shared/include",
        "." }
    links { "emulate_intel", "emulate_arm", "emulate_riscv", "logger" }

project "emulate"
    kind "ConsoleApp"
    language "C"
    targetdir ".build/bin/%{cfg.buildcfg}"
    files { "./apps/emulate/**.h", "./apps/emulate/**.c" }
    includedirs {
        "./libraries/logger/include",
        "./libraries/emulate_riscv/include",
        "./libraries/emulate_arm/include",
        "./libraries/emulate_intel/include",
        "./shared/include",
        "."
    }
    links { "emulate_intel", "emulate_arm", "emulate_riscv", "logger" }
