# Building DSDA-Doom on Windows

This is a guide for building DSDA-Doom on Windows. It covers the use of MSVC and MSYS2.

## Building using MSVC

This section will cover the installation of vcpkg, its configuration, and how to use it in Visual Studio, Visual Studio Code, or the terminal.

### Installing additional tools

vcpkg and DSDA have similar needs for tools, you will need:

- [CMake](https://cmake.org/)
- [Git](https://git-scm.com/)

For both, **make sure you tick the options to add them to your PATH**.

Once the installation finishes, you should be able to open a terminal (preferably a powershell) and run `git --version` and `cmake --version`.

### Installing vcpkg

vcpkg is an open-source cross-platform package manager by Microsoft. It heavily relies on git and CMake to function. On Windows, it is the most convenient way to get dependencies for MSVC.

Start off by opening a terminal and going to the location where you wish to install vcpkg. It should preferably a short path such as `C:\`, `D:\Dev`, or similar due to path limits, for the rest of this section, it will be assumed that vcpkg will be installed in `C:\`. Run the following commands to setup vcpkg:

```
cd C:
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat -disableMetrics
```

You then want to add the directory you chose to your PATH, in this case, you would add `C:\vcpkg\`. After this, if you close and re-open your terminal, you should be able to use the `vcpkg` command from anywhere.

If you plan on using Visual Studio, open a terminal **as Administrator**, and run the following command:

```
vcpkg integrate install
```

**IMPORTANT: vcpkg does not update itself automatically.** To get newer versions of libraries, you will need to regularly come back and run `git pull`. Similarly, to update the tool, run `./bootstrap-vcpkg.bat -disableMetrics` again after pulling the changes.

### Building with Visual Studio

Open Visual Studio, and select the "Clone a repository" option. The repository location is `https://github.com/kraflab/dsda-doom.git`, and the path you provide is where the files will be copied to, for example `C:/git/dsda-doom/`. If you already have cloned the repository beforehand, select "Open" and "Open Folder", you should **not** use the "CMake" option as this repo uses an uncommon layout.

To generate the CMake cache, select "Project" and then "Configure Cache". During configuration, the vcpkg toolchain will be automatically provided to CMake and it will compile all the dependencies. Once it finishes, you should be able to select `dsda-doom.exe` as a startup item and run it.

By default, the CMake integration only provides an `x64-Debug` configuration. To add a release configuration, head back to "Project" and select "CMake settings for dsda-doom".

On the left, you should see a list of all the configurations you have, click the green + at the top and search for `x64-Release`. After adding it, you should now be able to switch configuration at the top. This menu also lets you customise the CMake cache and set defaults.

To install a build, select "Build" and then "Install dsda-doom". The default directories for build and install are, respectively, `prboom2/out/build/<config>` and `prboom2/out/install/<config>`.

### Building from the terminal

Make a clone of the repository:

```
git clone https://github.com/kraflab/dsda-doom.git
```

Run the CMake configuration:

```
cd dsda-doom
cmake -Sprboom2 -Bbuild -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" -DENABLE_LTO=ON
```

During this step, vcpkg will build all the dependencies. If vcpkg does not get invoked or CMake fails at finding the dependencies, delete the build directory and make sure the path to the `vcpkg.cmake` toolchain is correct.

And finally, build the project:

```
cmake --build build --config Release
```

You may replace `Release` with `Debug`, `MinSizeRel`, or `RelWithDebInfo` depending on your needs.

You should then be able to run dsda-doom from the build directory:

```
cd build/Release
./dsda-doom
```

### Building with Visual Studio Code

Make a clone of the repository:

```
git clone https://github.com/kraflab/dsda-doom.git
```

You can either use an extension such as CMake Tools or the integrated terminal to compile. The first time you open the directory, you will be prompted to install recommended extensions for syntax highlighting and CMake support.

If you use the CMake Tools extension, you will need to:

- Set your workspace settings to tell CMake Tools the location of the CMakeLists.txt:

```json
"cmake.sourceDirectory": "${workspaceFolder}/prboom2"
```

- Create a new kit (preferably a copy of the one you want to use), and add this key to it:

```json
"toolchainFile": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

## Building with MSYS2

At the time of writing, MSYS2 provides seven different environments. This section assumes you are using UCRT64 (`mingw-w64-ucrt-x86_64-`). If you wish to use another one, refer to [the package naming](https://www.msys2.org/docs/package-naming/) and replace the UCRT64 prefix with the one of your choice.

### Installing dependencies

To download and build the repo, you will need the following tools:

```
pacman -S mingw-w64-ucrt-x86_64-gcc cmake git ninja pkgconf
```

Additionally, you will need the following external libraries:

```
pacman -S mingw-w64-ucrt-x86_64-libxmp mingw-w64-ucrt-x86_64-fluidsynth mingw-w64-ucrt-x86_64-libmad mingw-w64-ucrt-x86_64-libvorbis mingw-w64-ucrt-x86_64-libzip mingw-w64-ucrt-x86_64-portmidi mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-libsndfile
```

### Building

Make a clone of the repository:

```
git clone https://github.com/kraflab/dsda-doom.git
```

Run the CMake configuration:

```
cd dsda-doom
cmake -Sprboom2 -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release
```

CMake does not appear to support LTO properly for MinGW GCC which results in much longer linking time. Consider only enabling it when making a release.

And finally, build the project:

```
cmake --build build --config Release
```

You should then be able to run dsda-doom from the build directory:

```
cd build
./dsda-doom
```

You will only be able to run the executable from the MSYS2 Terminal directly as the necessary DLLs are not copied over to the build directory.

## Installing and Packaging

This section will assume the build directory is `build`, note that by default on Visual Studio the build directory is different.

To install the project, run:

```
cmake --install build --config Release
```

This will install all the necessary runtime files to the directory `CMAKE_INSTALL_PREFIX` is set to. This can also be overwritten by passing `--prefix C:/my/custom/prefix`. Prefer a directory that does not need priviledges to write files to, otherwise editing config and saving the game may fail.

To package the project, run:

```
cd build
cpack -G ZIP -C Release
```

This will produce a zip archive in your build directory named `dsda-doom-<version>-win64` which contains everything necessary to run the game.

## Troubleshooting

### IdentifyVersion: IWAD not found

This is not related to build issues, DSDA-Doom is only a Doom port. You will need to provide your own copy of DOOM, DOOM2, Final Doom, Heretic, or Hexen.

### Could NOT find \<Package\>

When using MSVC, this most likely means vcpkg was not used. If you are using Visual Studio, make sure you ran `vcpkg integrate install`. If you are building from a terminal or Visual Studio, make sure you used vcpkg's CMake toolchain. In either case, you want to delete the cache (CMakeCache.txt) and re-run the configure step.

When using MSYS2, this most likely means you did not install the correct dependencies. Make sure you are using the same environment everywhere, if you install the `mingw-w64-clang-x86_64-` packages, you cannot use them from the UCRT64 profile, etc. Also make sure you installed the correct version of dependencies. SDL, SDL_image, and SDL_mixer have both version 2 and 1.2 available, currently we require version 2.

### vcpkg: The term 'vcpkg' is not recognized as a name of a cmdlet, function, script file, or executable program.

Make sure you ran `bootstrap-vcpkg.bat`, added the vcpkg directory to your `PATH`, and restarted your terminal/programs to refresh the environment.

### X does not work in my terminal

You are probably using CMD.exe and all its quirks (`cd C:` does not go to `C:\`, `./command` does not work, etc.). Either use powershell, or adapt the commands.

### My prebuilt dependencies in `dsda-doom/dependencies_x64` are no longer picked up by CMake!

Back when prboom-plus added this feature, getting all the dependencies for MSVC was much more difficult. So prebuilt binaries were distributed through a release and users were expected to unpack the two `dependencies_<arch>` directories at the root of the repository.

At the time of writing, the latest prebuilt binaries are more than three years old, and some libraries (most notably the SDL ones) have seen numerous version updates. Moreover, vcpkg has vastly progressed since and plays much more nicely with static linking. Additionally, the prebuilt dependencies only covered x86 and x64 whereas MSVC also provides full arm64 support.

With the recent vcpkg integration and CMake cleanups, it was decided to drop this undocumented feature. If you wish to keep using the outdated prebuilt binaries and want to reproduce the old behaviour, add to your CMake configure command the following flag: `-DCMAKE_PREFIX_PATH="C:/path/to/dependencies_x64"`.
