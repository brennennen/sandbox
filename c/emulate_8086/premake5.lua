workspace "PremakeSandbox"
    configurations { "Debug", "Release" }
    location ".build"

project "decode8086"
    kind "StaticLib"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./libraries/decode8086/include/**.h", "./libraries/decode8086/source/**.c" }
    includedirs { "./librares/decode8086/include", "./shared/include", "." }
project "test_decode8086"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./libraries/decode8086/test/**.h", "./libraries/decode8086/test/**.c" }
    includedirs { "./librares/decode8086/include", "./shared/include", "." }
    links { "decode8086", "criterion" }
    sanitize { "Address" }

project "sandbox"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./apps/sandbox/**.h", "./apps/sandbox/**.c" }
    includedirs { "./libraries", "./shared", "." }
    links { "decode8086" }
    sanitize { "Address" }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

project "test"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"
    files { "./test/**.h", "./test/**.c" }
    includedirs { "./libraries", "./libraries/**/include", "./shared/include", "." }
    links { "decode8086", "criterion" }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
