# RETRO-Engine

A small OpenGL 3.3 engine (GLFW + glad + FreeType).

## Prerequisites

Most third-party headers are vendored in `include/` (GLFW, glad, KHR, stb_image,
FreeType), so you only need a compiler, `make`, and the following libraries:

- **glm** (headers only, and the one dependency *not* vendored)
- **GLFW** and **FreeType** (the compiled libraries, headers are vendored)
- An OpenGL implementation

### Linux (Arch)

```sh
sudo pacman -S base-devel glm glfw freetype2 mesa
```

On Debian/Ubuntu the equivalent is:

```sh
sudo apt install build-essential libglm-dev libglfw3-dev libfreetype-dev libgl1-mesa-dev
```

### Windows (MSYS2)

Install [MSYS2](https://www.msys2.org/), then open the **MSYS2 UCRT64** shell
(not the plain "MSYS" shell, the environment matters) and run:

```sh
pacman -S mingw-w64-ucrt-x86_64-gcc \
          mingw-w64-ucrt-x86_64-make \
          mingw-w64-ucrt-x86_64-glm \
          mingw-w64-ucrt-x86_64-glfw \
          mingw-w64-ucrt-x86_64-freetype
```

## Build and run

From the repository root:

```sh
make          # Linux
./Engine

mingw32-make  # Windows, from the UCRT64 shell
./Engine.exe
```

`make re` rebuilds from scratch, `make clean` removes the build output, and
`make run` builds and launches in one step.

The Makefile detects the platform and adjusts the link flags and binary name on
its own, so the same targets work in both environments.

## Notes

Run the binary from the repository root. Assets under `resources/` and the
shaders in `shaders/` are loaded through relative paths, so launching from
another directory will fail to find them.

On Windows, `Engine.exe` links against the MSYS2 runtime DLLs (`glfw3.dll`,
`libfreetype-6.dll`, `libstdc++-6.dll`, `libgcc_s_seh-1.dll`), which live in
`C:\msys64\ucrt64\bin`. It runs from the UCRT64 shell, where that directory is
already on `PATH`, but double-clicking it in Explorer will fail with missing-DLL
errors.

## Controls

| Input | Action |
| --- | --- |
| `W` `A` `S` `D` | Move |
| Space / Left Shift | Up / down |
| Mouse | Look |
| `F3` | Toggle the debug HUD |
| `Esc` | Quit |

## Third-party libraries

This project uses several third-party libraries. See
[THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) for their respective
licenses and copyright notices.