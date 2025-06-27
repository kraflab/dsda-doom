option(WITH_FLUIDSYNTH "Use FluidSynth if available" ON)
if(WITH_FLUIDSYNTH)
    list(APPEND VCPKG_MANIFEST_FEATURES "fluidsynth")
endif()

option(WITH_IMAGE "Use SDL2_image if available" ON)
if(WITH_IMAGE)
    list(APPEND VCPKG_MANIFEST_FEATURES "sdl2-image")
endif()

option(WITH_MAD "Use libmad if available" ON)
if(WITH_MAD)
    list(APPEND VCPKG_MANIFEST_FEATURES "libmad")
endif()

option(WITH_PORTMIDI "Use PortMidi if available" ON)
if(WITH_PORTMIDI)
    list(APPEND VCPKG_MANIFEST_FEATURES "portmidi")
endif()

option(WITH_VORBISFILE "Use vorbisfile if available" ON)
if(WITH_VORBISFILE)
    list(APPEND VCPKG_MANIFEST_FEATURES "libvorbis")
endif()

option(WITH_XMP "Use libxmp if available" ON)
if(WITH_XMP)
    list(APPEND VCPKG_MANIFEST_FEATURES "libxmp")
endif()

if(CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg.cmake$")
  option(VCPKG_APPLOCAL_DEPS "Copy dependencies in the output directory" ON)
  option(X_VCPKG_APPLOCAL_DEPS_INSTALL "Copy dependencies during installation" ON)
endif()
