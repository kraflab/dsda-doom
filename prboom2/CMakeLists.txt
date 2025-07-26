cmake_minimum_required(VERSION 3.17...4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Setup dependencies options before `project()` so vcpkg features can be used
include(DsdaDepsSetup)

project("dsda-doom" VERSION 0.29.3)

set(dsda_is_top_project FALSE)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(dsda_is_top_project TRUE)
endif()

if(dsda_is_top_project)
    include(DsdaSanitiser)
endif()

include(DsdaHelpers)
if(dsda_is_top_project)
    dsda_set_default_build_config(RelWithDebInfo)
endif()

if(VCPKG_TOOLCHAIN)
    set(ENV{PKG_CONFIG_PATH} "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig")
endif()

set(PROJECT_TARNAME "dsda-doom")
set(WAD_DATA "dsda-doom.wad")
set(PROJECT_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")

include(DsdaOptions)
include(DsdaTargetFeatures)
include(DsdaDependencies)
include(DsdaConfigHeader)

set(DSDA_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(WAD_DATA_PATH "${DSDA_OUTPUT_PATH}/${WAD_DATA}")

set(DSDA_BUILD_PWAD_WITH_EXTERNAL_PROJECT ${CMAKE_CROSSCOMPILING})
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(DSDA_BUILD_PWAD_WITH_EXTERNAL_PROJECT TRUE)
    set(DSDA_EXTERNAL_PROJECT_VS_ARGS
        "CMAKE_GENERATOR" "${CMAKE_GENERATOR}"
        "CMAKE_GENERATOR_PLATFORM" "${CMAKE_VS_PLATFORM_NAME_DEFAULT}"
    )
endif()

if(DSDA_BUILD_PWAD_WITH_EXTERNAL_PROJECT)
    set(DSDA_HOST_TOOLCHAIN ""
        CACHE FILEPATH "Toolchain file to build host tools with"
    )
    include(ExternalProject)
    ExternalProject_Add(dsda-doom-wad
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data"
        ${DSDA_EXTERNAL_PROJECT_VS_ARGS}
        CMAKE_CACHE_ARGS
            "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
            "-DWAD_DATA_PATH:FILEPATH=${WAD_DATA_PATH}"
            "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${DSDA_HOST_TOOLCHAIN}"
        BUILD_ALWAYS TRUE
        BUILD_BYPRODUCTS "${WAD_DATA_PATH}"
        INSTALL_COMMAND ""
    )
else()
    add_subdirectory(data)
endif()

add_subdirectory(src)
add_subdirectory(ICONS)

install(FILES
    "${WAD_DATA_PATH}"
    DESTINATION "${DSDAPWADDIR}"
)

install(FILES
    "COPYING"
    DESTINATION "${DSDA_INSTALL_COPYRIGHT_DIR}"
)

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
