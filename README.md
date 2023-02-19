# GameEngine
Starting point for game engine development

## Build Instructions

### Prerequisites
Requiremets: CMake, MinGW, GLFW

#### CMake
1. Download and install [CMake](https://cmake.org/download/)

#### MinGW
1. Download [MinGW](https://github.com/niXman/mingw-builds-binaries/releases). I usually use [this one](https://github.com/niXman/mingw-builds-binaries/releases/download/12.2.0-rt_v10-rev2/x86_64-12.2.0-release-posix-seh-msvcrt-rt_v10-rev2.7z).
2. Unzip and move the **mingw64** folder to **C:\\** or wherever else you want to put it.
3. Add **C:\mingw64\bin** to your **Path** environment variable.
4. For convenience, in the **C:\mingw64\bin** directory, I usually create a copy of **mingw32-make.exe** and rename to **make.exe**.

#### GLFW
1. Dowload source package of [GLFW](https://www.glfw.org/download.html)
2. unzip, navigate to GLFW root directory, and run these commands to build and install GLFW (may need to run command prompt as administrator)
```
cmake -G "MinGW Makefiles" -S . -B build -DCMAKE_INSTALL_PREFIX=C:\mingw64\x86_64-w64-mingw32
cmake --build build
cmake --build build --target install
```

### Building Game
1. Set environment variable **glfw3_DIR** to **C:\msys64\mingw64\x86_64-w64-mingw32\lib\cmake\glfw3** for cmake to find the GLFW CMake package.
2. From the game root directory run these commands
```
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build
```
