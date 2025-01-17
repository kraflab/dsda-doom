cmake_minimum_required(VERSION 3.9)

option(WITH_DUMB "Use DUMB if available" ON)
if(WITH_DUMB)
    list(APPEND VCPKG_MANIFEST_FEATURES "dumb")
endif()

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

# Automatically install dependencies
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(X_VCPKG_APPLOCAL_DEPS_INSTALL TRUE)

project("dsda-doom" VERSION 0.28.3)

# Set a default build type if none was specified
set(default_build_type "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckBigEndian)
check_big_endian(WORDS_BIGENDIAN)

include(GNUInstallDirs)

if(POLICY CMP0099)
  cmake_policy(SET CMP0099 NEW)
else()
  message(WARNING "Your version of CMake is very old. This may cause linking issues if your dependencies are not in your compiler's default search paths.")
endif()

option(CMAKE_FIND_PACKAGE_PREFER_CONFIG "Search for package config before using Find modules" ON)

if(VCPKG_TOOLCHAIN)
    set(ENV{PKG_CONFIG_PATH} "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig")
endif()

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT HAVE_LTO)

include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LTO "Use link-time optimisation" OFF "HAVE_LTO" OFF)

set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_TARNAME "dsda-doom")
set(WAD_DATA "dsda-doom.wad")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")

include(CheckSymbolExists)

check_symbol_exists(stricmp "string.h" HAVE_STRICMP)
check_symbol_exists(strnicmp "string.h" HAVE_STRNICMP)
check_symbol_exists(getopt "unistd.h" HAVE_GETOPT)
check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(CreateFileMapping "windows.h" HAVE_CREATE_FILE_MAPPING)
check_symbol_exists(strsignal "string.h" HAVE_STRSIGNAL)
check_symbol_exists(mkstemp "stdlib.h" HAVE_MKSTEMP)
check_symbol_exists(getpwuid "unistd.h;sys/types.h;pwd.h" HAVE_GETPWUID)

include(CheckIncludeFile)

check_include_file("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("asm/byteorder.h" HAVE_ASM_BYTEORDER_H)
check_include_file("dirent.h" HAVE_DIRENT_H)

include(GetAvailableFlags)

get_supported_warnings(SUPPORTED_WARNINGS)
get_fast_math_flag(FAST_MATH_FLAG)
get_compile_definitions(DSDA_COMPILE_DEFINITIONS)
get_deprecation_silencing_definitions(DEPRECATION_SILENCING_DEFINITIONS)

include(PkgConfigHelper)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL 2.0 REQUIRED)
find_package(libzip REQUIRED)
find_package(SDL2 2.0.7 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(ZLIB REQUIRED)

if(NOT TARGET SDL2_mixer::SDL2_mixer)
    add_library(SDL2_mixer::SDL2_mixer ALIAS SDL2_mixer::SDL2_mixer-static)
endif()

if(WITH_IMAGE)
    find_package(SDL2_image)
    if(SDL2_image_FOUND)
        set(HAVE_LIBSDL2_IMAGE TRUE)
        if(NOT TARGET SDL2_image::SDL2_image)
            add_library(SDL2_image::SDL2_image ALIAS SDL2_image::SDL2_image-static)
        endif()
    endif()
endif()

if(WITH_MAD)
    find_package(LibMad)
    if(LibMad_FOUND)
        set(HAVE_LIBMAD TRUE)
    endif()
endif()

if(WITH_FLUIDSYNTH)
    find_package(FluidSynth)
    if(FluidSynth_FOUND)
        set(HAVE_LIBFLUIDSYNTH TRUE)
    endif()
endif()

if(WITH_DUMB)
    find_package(DUMB)
    if(DUMB_FOUND)
        set(HAVE_LIBDUMB TRUE)
    endif()
endif()

if(WITH_VORBISFILE)
    find_package(Vorbis COMPONENTS File)
    if(Vorbis_File_FOUND)
        set(HAVE_LIBVORBISFILE TRUE)
    endif()
endif()

if(WITH_PORTMIDI)
    find_package(PortMidi)
    if(PortMidi_FOUND)
        set(HAVE_LIBPORTMIDI TRUE)
    endif()
endif()

if(WIN32)
    set(DEFAULT_WAD_DIR ".")
else()
    set(DEFAULT_WAD_DIR "share/games/doom")
endif()

set(DSDAPWADDIR "${DEFAULT_WAD_DIR}" CACHE STRING "Path to install DSDA-Doom internal WAD, relative to CMAKE_INSTALL_PREFIX or absolute.")
set(DOOMWADDIR "${CMAKE_INSTALL_PREFIX}/${DEFAULT_WAD_DIR}" CACHE PATH "Path to look for WAD files.")

if(IS_ABSOLUTE "${DSDAPWADDIR}")
    set(DSDA_ABSOLUTE_PWAD_PATH "${DSDAPWADDIR}")
else()
    set(DSDA_ABSOLUTE_PWAD_PATH "${CMAKE_INSTALL_PREFIX}/${DSDAPWADDIR}")
endif()

option(SIMPLECHECKS "Enable checks which only impose significant overhead if a posible error is detected" ON)

# Debug options, disabled by default
option(RANGECHECK "Enable internal range checking" OFF)

configure_file(cmake/config.h.cin config.h)

# Support cross compiling
option(FORCE_CROSSCOMPILE "Enable cross-compilation" OFF)
if(FORCE_CROSSCOMPILE)
    set(CMAKE_CROSSCOMPILING ON)
endif()

if(CMAKE_CROSSCOMPILING)
    set(IMPORT_EXECUTABLES "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Export file from native build")
    include(${IMPORT_EXECUTABLES})
else()
    if(NOT CROSS_EXPORTS)
        set(CROSS_EXPORTS "")
    endif()
endif()

set(DSDA_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(WAD_DATA_PATH "${DSDA_OUTPUT_PATH}/${WAD_DATA}")

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(ICONS)

if(NOT CMAKE_CROSSCOMPILING)
    export(TARGETS ${CROSS_EXPORTS} FILE "${CMAKE_BINARY_DIR}/ImportExecutables.cmake")
endif()

if(WIN32)
    install(FILES COPYING DESTINATION . RENAME COPYING.txt)
else()
    install(FILES COPYING DESTINATION "share/doc/${PROJECT_NAME}")
endif()

if(WIN32)
    set(CPACK_GENERATOR ZIP)
elseif(LINUX)
    set(CPACK_GENERATOR External)
    set(CPACK_EXTERNAL_ENABLE_STAGING YES)
    set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${PROJECT_BINARY_DIR}/appimage-generate.cmake")

    file(GENERATE
        OUTPUT "${PROJECT_BINARY_DIR}/appimage-generate.cmake"
        CONTENT [[
            find_program(LINUXDEPLOY_EXECUTABLE
                NAMES linuxdeploy linuxdeploy-x86_64.AppImage
                PATHS ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy)

            if(NOT LINUXDEPLOY_EXECUTABLE)
                message(STATUS "Downloading linuxdeploy")
                set(LINUXDEPLOY_EXECUTABLE ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy/linuxdeploy)
                file(DOWNLOAD
                    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
                    ${LINUXDEPLOY_EXECUTABLE}
                    INACTIVITY_TIMEOUT 10)
                execute_process(COMMAND
                    chmod +x ${LINUXDEPLOY_EXECUTABLE} COMMAND_ECHO STDOUT)
            endif()

            execute_process(COMMAND
                ${CMAKE_COMMAND} -E env
                OUTPUT=${CPACK_PACKAGE_FILE_NAME}.appimage
                VERSION=$<IF:$<BOOL:${CPACK_PACKAGE_VERSION}>,${CPACK_PACKAGE_VERSION},0.1.0>
                ${LINUXDEPLOY_EXECUTABLE}
                --appimage-extract-and-run
                --appdir=${CPACK_TEMPORARY_DIRECTORY}
                --executable=$<TARGET_FILE:dsda-doom>
                --desktop-file=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/applications/dsda-doom.desktop
                --icon-file=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/dsda-doom.svg
                --output=appimage)
            ]])
elseif(APPLE)
    set(CPACK_GENERATOR External)
    set(CPACK_EXTERNAL_ENABLE_STAGING YES)
    set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${PROJECT_BINARY_DIR}/macbundle-generate.cmake")
    set(CPACK_SYSTEM_NAME "mac-${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_BUILD_DIR "${PROJECT_BINARY_DIR}")

    file(GENERATE
        OUTPUT "${PROJECT_BINARY_DIR}/macbundle-generate.cmake"
        CONTENT [[
            set(CPACK_OUTPUT_DIR "${CPACK_BUILD_DIR}/${CPACK_PACKAGE_FILE_NAME}")
            file(MAKE_DIRECTORY ${CPACK_OUTPUT_DIR})

            find_program(DYLIBBUNDLER_EXECUTABLE NAMES dylibbundler)
            if(NOT DYLIBBUNDLER_EXECUTABLE)
                message(FATAL_ERROR "Missing dylibbundler (brew install dylibbundler)")
            endif()

            file(COPY_FILE
                $<TARGET_FILE:dsda-doom>
                ${CPACK_OUTPUT_DIR}/dsda-doom)

            execute_process(COMMAND
                ${CMAKE_COMMAND} -E env
                OUTPUT=${CPACK_PACKAGE_FILE_NAME}.zip
                VERSION=$<IF:$<BOOL:${CPACK_PACKAGE_VERSION}>,${CPACK_PACKAGE_VERSION},0.1.0>
                ${DYLIBBUNDLER_EXECUTABLE}
                --bundle-deps
                --create-dir
                --fix-file ${CPACK_OUTPUT_DIR}/dsda-doom
                --install-path @executable_path/libs_${CPACK_SYSTEM_PROCESSOR}
                --dest-dir ${CPACK_OUTPUT_DIR}/libs_${CPACK_SYSTEM_PROCESSOR})

            file(COPY_FILE
                ${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/games/doom/dsda-doom.wad
                ${CPACK_OUTPUT_DIR}/dsda-doom.wad)
            file(COPY_FILE
                ${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/doc/dsda-doom/COPYING
                ${CPACK_OUTPUT_DIR}/COPYING.txt)

            file(CONFIGURE
                OUTPUT ${CPACK_OUTPUT_DIR}/Troubleshooting.txt
                CONTENT "If you are getting errors like 'libzip.5.5.dylib cant be opened because Apple cannot check it for malicious software.' Run the following command in the dsda-doom folder:\n\nxattr -dr com.apple.quarantine path/to/folder")

            execute_process(COMMAND zip -r ${CPACK_PACKAGE_FILE_NAME}.zip ${CPACK_PACKAGE_FILE_NAME})
            ]])
endif()
include(CPack)
