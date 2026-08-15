find_program(LINUXDEPLOY_EXECUTABLE
  NAMES linuxdeploy linuxdeploy-${CPACK_SYSTEM_PROCESSOR}.AppImage
  PATHS ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy
)

set(linuxdeploy_prebuilt_archs x86_64 i386 aarch64 armhf)

if(NOT LINUXDEPLOY_EXECUTABLE)
  if(NOT CPACK_SYSTEM_PROCESSOR IN_LIST linuxdeploy_prebuilt_archs)
    message(FATAL_ERROR "linuxdeploy could not be found, and is not available to download for your system.")
  endif()

  message(STATUS "Downloading linuxdeploy")
  set(LINUXDEPLOY_EXECUTABLE ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy/linuxdeploy)
  file(DOWNLOAD
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${CPACK_SYSTEM_PROCESSOR}.AppImage"
    ${LINUXDEPLOY_EXECUTABLE}
    INACTIVITY_TIMEOUT 10
  )

  execute_process(
    COMMAND chmod +x ${LINUXDEPLOY_EXECUTABLE}
    COMMAND_ECHO STDOUT
  )
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E env
    LDAI_OUTPUT=${CPACK_PACKAGE_FILE_NAME}.appimage
    LINUXDEPLOY_OUTPUT_VERSION=${CPACK_PACKAGE_VERSION}
    NO_STRIP=1
    ${LINUXDEPLOY_EXECUTABLE}
    --appimage-extract-and-run
    --appdir=${CPACK_TEMPORARY_DIRECTORY}
    --executable=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/${CPACK_BIN_DIR}/dsda-doom
    --desktop-file=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/applications/dsda-doom.desktop
    --icon-file=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/dsda-doom.svg
    --output=appimage
)
