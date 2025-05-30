# Building DSDA-Doom on macOS
This is a basic guide for building DSDA-Doom for a x86_64 or arm64 macOS target using brew. 
## Configure brew
[brew](https://brew.sh) is a package manager for macOS and Linux. we will use it to download everything we need to build DSDA-Doom.

To install it we need to run:
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
On x86_64 machines, brew will be installed in `/usr/local/homebrew`.

On arm64 machines, brew will be installed in `/opt/homebrew`.
## Install Build Dependencies
Install cmake, SDL2 and additional dependencies for DSDA-Doom:
```
brew install cmake pkgconf libopenmpt fluid-synth libvorbis libzip mad portmidi sdl2 sdl2_image sdl2_mixer
```
## Build DSDA-Doom
Make a clone of the DSDA-Doom Git repository:
```
git clone https://github.com/kraflab/dsda-doom.git
```
Prepare the build folder, generate the build system, and compile:
```
cd dsda-doom
cmake -Sprboom2 -Bbuild -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
cmake --build build
```

The newly built binaries are located in the build folder.

## Collect DYLIB Files
Create a release folder next to the build folder and copy the Binaries and .wad files to it:
```
mkdir release
cp ./build/dsda-doom ./release/dsda-doom
cp ./build/*.wad ./release/
```

Install the "dylibbundler" program and use it to bundle the .dylib files:

```
brew install dylibbundler

cd ./release
dylibbundler -od -b -x ./dsda-doom -d ./libs/ -p @executable_path/libs
```

## Final Steps

Since this is a release build, it's customary to remove symbols from the binaries (and since we are changing the binary file, we will need to codesign it again):

```
strip ./dsda-doom
codesign --force --deep --preserve-metadata=entitlements,requirements,flags,runtime --sign - "./dsda-doom"
```
Finally, add the files to an archive with today's date:
```
zip -r ./dsda-doom-$(date +"%Y%m%d")-mac.zip . -x .\*
```
