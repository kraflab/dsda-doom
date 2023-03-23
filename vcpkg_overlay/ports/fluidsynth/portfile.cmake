vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO FluidSynth/fluidsynth
    REF "0589db1a286a5fd70711a020bf5a3fd527e3fdb1"
    SHA512 6bc6a54ddafa32e676f664a1f88524d86a0b242b4c8ac6d231f6556aedffb38d8b5344d7816d1b775d426dbed887f0be4363584fbe51acfa4c1b990f8e86a6d2
    HEAD_REF master
    PATCHES
        gentables.patch
        no-install-tool.patch
)

vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        buildtools  VCPKG_BUILD_MAKE_TABLES
        sndfile     enable-libsndfile
)

# enable platform-specific features, and force the build to fail if the
# required libraries are not found
set(WINDOWS_OPTIONS "enable-dsound" "enable-wasapi" "enable-waveout" "enable-winmidi" "HAVE_MMSYSTEM_H" "HAVE_DSOUND_H" "HAVE_OBJBASE_H")
set(MACOS_OPTIONS "enable-coreaudio" "enable-coremidi" "COREAUDIO_FOUND" "COREMIDI_FOUND")
set(LINUX_OPTIONS "enable-alsa" "ALSA_FOUND")
set(ANDROID_OPTIONS "enable-opensles")
set(IGNORED_OPTIONS "enable-coverage" "enable-dbus" "enable-floats" "enable-fpe-check" "enable-framework" "enable-jack" "enable-lash" 
    "enable-libinstpatch" "enable-midishare" "enable-oboe" "enable-openmp" "enable-oss" "enable-pipewire" "enable-portaudio"
    "enable-profiling" "enable-pulseaudio" "enable-readline" "enable-sdl2" "enable-systemd" "enable-trap-on-fpe" "enable-ubsan")

if(VCPKG_TARGET_IS_WINDOWS)
    set(OPTIONS_TO_ENABLE ${WINDOWS_OPTIONS})
    set(OPTIONS_TO_DISABLE ${MACOS_OPTIONS} ${LINUX_OPTIONS} ${ANDROID_OPTIONS})
elseif(VCPKG_TARGET_IS_OSX)
    set(OPTIONS_TO_ENABLE ${MACOS_OPTIONS})
    set(OPTIONS_TO_DISABLE ${WINDOWS_OPTIONS} ${LINUX_OPTIONS} ${ANDROID_OPTIONS})
elseif(VCPKG_TARGET_IS_LINUX)
    set(OPTIONS_TO_ENABLE ${LINUX_OPTIONS})
    set(OPTIONS_TO_DISABLE ${WINDOWS_OPTIONS} ${MACOS_OPTIONS} ${ANDROID_OPTIONS})
elseif(VCPKG_TARGET_IS_ANDROID)
    set(OPTIONS_TO_ENABLE ${ANDROID_OPTIONS})
    set(OPTIONS_TO_DISABLE ${WINDOWS_OPTIONS} ${MACOS_OPTIONS} ${LINUX_OPTIONS})
endif()

foreach(_option IN LISTS OPTIONS_TO_ENABLE)
    list(APPEND ENABLED_OPTIONS "-D{_option}:BOOL=ON")
endforeach()
    
foreach(_option IN LISTS OPTIONS_TO_DISABLE IGNORED_OPTIONS)
    list(APPEND DISABLED_OPTIONS "-D${_option}:BOOL=OFF")
endforeach()

vcpkg_find_acquire_program(PKGCONFIG)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DVCPKG_HOST_TRIPLET="${HOST_TRIPLET}"
        ${FEATURE_OPTIONS}
        ${ENABLED_OPTIONS}
        ${DISABLED_OPTIONS}
        -DPKG_CONFIG_EXECUTABLE="${PKGCONFIG}"
    MAYBE_UNUSED_VARIABLES
        ${OPTIONS_TO_DISABLE}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/fluidsynth)

vcpkg_fixup_pkgconfig()

if("buildtools" IN_LIST FEATURES)
    list(APPEND tools make_tables)
    vcpkg_copy_tools(TOOL_NAMES make_tables AUTO_CLEAN)
endif()

vcpkg_copy_pdbs()

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
    "${CURRENT_PACKAGES_DIR}/share/man")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
