workspace "PremakeSandbox"
    configurations { "Debug", "Release" }
    location ".build"

project "lib_a"
    kind "StaticLib"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./libraries/lib_a/**.h", "./libraries/lib_a/**.c" }
    includedirs { "./librares/lib_a/include", "./shared/include", "." }

project "lib_b"
    kind "StaticLib"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./libraries/lib_b/**.h", "./libraries/lib_b/**.c" }
    includedirs { "./libraries/lib_b/include", "./shared/include", "." }

project "module_a"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./modules/module_a/**.h", "./modules/module_a/**.c" }
    includedirs { "./libraries", "./shared", "." }
    links { "lib_a" }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

project "module_b"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./modules/module_b/**.h", "./modules/module_b/**.c" }
    includedirs { "./libraries", "./libraries/**/include", "./shared/include", "." }
    links { "lib_a", "lib_b" }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
