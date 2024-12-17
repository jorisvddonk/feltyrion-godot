# feltyrion-godot

This repository contains a highly experimental GDExtension extension for Godot that exposes Noctis IV universe information to Godot, allowing either a re-implementation of Noctis IV in Godot, or creation of explorer tools in Godot. The GDExtension is intended to be called via GDScript or C# scripts; with only very few exceptions (one at the time of writing), the extension does not operate on a Godot scene tree at all.

This repository structure is based on [GDExtensionTemplate](https://github.com/asmaloney/GDExtensionTemplate) and is currently set up to work with the **[Godot 4.2](https://github.com/godotengine/godot/releases/tag/4.2-stable)** release.

Noctis IV source code that was used for this (and stripped heavily) is from the [noctis-iv-lr](https://github.com/dgcole/noctis-iv-lr) project. The Noctis IV source code itself was heavily stripped and modified in places to make compilation easier, and to ensure that certain information could be exposed to a GDExtension.

Because this extension is highly experimental, there is little to no documentation, and everything is subject to change.

## Notes and caveats

Please be aware that the extension exposes a lot of internal Noctis IV details, and uses SOURCE CODE PARSIS COORDINATES. Internally, Noctis IV's Parsis Y coordinates are flipped compared to what users see, so if you ever want to display parsis coordinates, make sure to flip the Y coordinate on your end!

## License

Since the project is directly based on the Noctis IV source code, the entire codebase is licensed under the WTOF Public License. See LICENSE.md for more information.

## Prerequisites

To use this locally on your machine, you will need the following:

- **[CMake](https://cmake.org/)** v3.22+
- C++ Compiler with at least **C++17** support (any recent compiler)
- (optional) **[ccache](https://ccache.dev/)** for faster rebuilds

## Build

Here's an example of how to build a release version; use the terminal to run the following commands in the root directory of this repository.

(If you want to build from some entirely different path, you can specify the absolute path to the locally checked-out git repository via cmake's `-S` flag (so instead of `-S .` as below, use `-S <absolute path to local git repository>`))

After building, you can find the built addon in `feltyion-godot-install`

### Linux

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=feltyrion-godot-install
cmake --build ./build --parallel
cmake --install ./build --config Release
```

### MacOS

On MacOS, you'll want to build without the `--parallel` flag to avoid running out of memory/PIDs:

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=feltyrion-godot-install
cmake --build ./build
cmake --install ./build --config Release
```

### Windows - MSVC

```powershell
cmake -B build -S . -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=feltyrion-godot-install
cmake --build ./build --config Release
cmake --install ./build --config Release
```

This tells CMake to use `Visual Studio 2022`. There is a list of Visual Studio generators [on the CMake site](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) - pick the one you are using.

## Cmake Options

The following additional CMake options are available:

| Option                                                                   | Description                                      | Default                                                                                              |
|--------------------------------------------------------------------------|--------------------------------------------------|------------------------------------------------------------------------------------------------------|
| `CCACHE_PROGRAM`                                                         | Path to `ccache` for faster rebuilds             | This is automatically set **ON** if `ccache` is found. If you do not want to use it, set this to "". |
| `${PROJECT_NAME_UPPERCASE}_WARN_EVERYTHING` (e.g. FOO_WARN_EVERYTHING)   | Turns on all warnings. (Not available for MSVC.) | **OFF** (too noisy, but can be useful sometimes)                                                     |
| `${PROJECT_NAME_UPPERCASE}_WARNING_AS_ERROR` (e.g. FOO_WARNING_AS_ERROR) | Turns warnings into errors.                      | **ON**                                                                                               |

# Usage

More usage information and documentation follows later.
