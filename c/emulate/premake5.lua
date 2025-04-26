workspace "PremakeSandbox"
    configurations { "Debug", "Release" }
    location ".build"

project "emulate_intel"
    kind "StaticLib"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./libraries/emulate_intel/include/**.h", "./libraries/emulate_intel/source/**.c" }
    includedirs { "./libraries/emulate_intel/include", "./shared/include", "." }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
    filter { "system:linux", "action:gmake" }
        toolset "gcc"
        buildoptions { "-std=c23" }

project "test_emulate_intel"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./libraries/emulate_intel/test/**.h", "./libraries/emulate_intel/test/**.c" }
    includedirs { "./libraries/emulate_intel/include", "./shared/include", "." }
    links { "emulate_intel", "criterion" }
    sanitize { "Address" }
    symbols "On"
    filter { "system:linux", "action:gmake" }
        toolset "gcc"
        buildoptions { "-std=c23" }

project "decode"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./apps/decode/**.h", "./apps/decode/**.c" }
    includedirs { "./libraries/emulate_intel/include", "./shared/include", "." }
    links { "emulate_intel" }
    sanitize { "Address" }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
    filter { "system:linux", "action:gmake" }
        toolset "gcc"
        buildoptions { "-std=c23" }

project "emulate"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./apps/emulate/**.h", "./apps/emulate/**.c" }
    includedirs { "./libraries/emulate_intel/include", "./shared/include", "." }
    links { "emulate_intel" }
    sanitize { "Address" }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
    filter { "system:linux", "action:gmake" }
        toolset "gcc"
        buildoptions { "-std=c23" }
