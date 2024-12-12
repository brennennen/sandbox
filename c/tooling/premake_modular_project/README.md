# Premake Sandbox
Sandbox for using the premake tooling to generate build system files. I plan to use this to make dealing with the windows compiler toolchain less painful, so the examples below will all be windows based. However, premake can generate other build system files

# Environment Setup
* Build Tools for Visual Studio
  * https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
  * Add MSBuild.exe to path (ex: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin`)
* Premake5
  * https://premake.github.io/download
  * Add premake5 to path (ex: `C:\bin`)

# Build
* `premake5 vs2022`
* `MSBuild.exe ./build/PremakeSandbox.sln`
* `MSBuild.exe ./build/PremakeSandbox.sln /property:Configuration=Release`
