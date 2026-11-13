# Auto Move NG

 User Requires [Original AutoMove Mod](https://www.nexusmods.com/skyrimspecialedition/mods/42845)
 
 This dll was built from reverse engineering the original mod dll. No source code was given to me. This mod does not distribute anything that was not my work. You still require the script files and esp from the original mod for this dll to be at all useful.
 
 ## Install
 
  - Install the [Original AutoMove Mod](https://www.nexusmods.com/skyrimspecialedition/mods/42845)
  - Install this mod, make sure this mod overwrites `AutoMove.dll`.
 
 
 ## Supported game versions

 Skyrim SE 1.5.97 and Skyrim AE 1.6.x / 1.7.x (newest: 1.7.104), through Address Library. VR is not supported.

 ## Build

 Every push builds the dll in GitHub Actions (`.github/workflows/build.yml`). Download it from the "AutoMove-NG" artifact of the workflow run.

 ### Requirements

  - Visual Studio 2022 (or newer) with the C++ desktop workload
  - cmake 3.21 or newer, and Ninja
  - vcpkg, with environment variable `VCPKG_ROOT` set to its folder
  - A python 3 install for scripts
  - Optional: Mod Organizer 2

 The library [CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG) is a git submodule in `lib/commonlibsse-ng`. Get it with:

 ```
 git submodule update --init --recursive
 ```

 ### Commands

 Optional: set environment variable `MO_SSE_INSTANCE` to the folder of your Skyrim Special Edition Mod Organizer instance. Then each build copies the dll into the mod "AutoMove NG" (`PROJECT_PLUGIN_NAME`), folder `SKSE/Plugins`. Make this mod and folder first.

 Build with (from a Visual Studio developer prompt):

 ```
 cmake --preset ninja-msvc-release
 cmake --build build --target AutoMove
 ```

 Or use the `vs2022-windows` preset with `cmake --build build --config (Debug|Release)`.

 To make a release zip for Nexus, run `py scripts/mkrelease.py -v <version> -n "AutoMove NG" -f build/AutoMove.dll -o AutoMove`.
