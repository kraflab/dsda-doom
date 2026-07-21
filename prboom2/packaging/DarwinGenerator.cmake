set(staging_dir "${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}")
set(packaged_dir "${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}")
file(MAKE_DIRECTORY ${packaged_dir})

file(COPY_FILE
  ${staging_dir}/${CPACK_BIN_DIR}/dsda-doom
  ${packaged_dir}/dsda-doom
)

file(COPY_FILE
  ${staging_dir}/${CPACK_PWAD_DIR}/dsda-doom.wad
  ${packaged_dir}/dsda-doom.wad
)

file(COPY_FILE
  ${staging_dir}/${CPACK_LICENSE_DIR}/COPYING
  ${packaged_dir}/COPYING.txt
)

file(WRITE
  "${packaged_dir}/Troubleshooting.txt"
  "If you are getting errors like 'Apple is not able to verify that dsda-doom is free from malware that could harm your Mac'\n"
  "Run the following command:\n\n"
  "xattr -dr com.apple.quarantine path/to/dsda-doom\n"
)

if(NOT VCPKG_LIBRARY_LINKAGE STREQUAL "static")
  find_program(DYLIBBUNDLER_EXECUTABLE
    NAMES dylibbundler
    REQUIRED
  )

  execute_process(
    COMMAND ${DYLIBBUNDLER_EXECUTABLE}
      --bundle-deps
      --create-dir
      --overwrite-files
      --fix-file ${packaged_dir}/dsda-doom
      --install-path @executable_path/libs_${CPACK_SYSTEM_PROCESSOR}
      --dest-dir ${packaged_dir}/libs_${CPACK_SYSTEM_PROCESSOR}
  )

  # SDL3 is loaded dynamically by sdl2-compat, so dylibbundler cannot detect it
  find_library(SDL3_LIBRARY
    NAMES SDL3
    PATHS /opt/homebrew/lib /usr/local/lib
    NO_DEFAULT_PATH
  )

  file(COPY_FILE
    "${SDL3_LIBRARY}"
    "${packaged_dir}/libs_${CPACK_SYSTEM_PROCESSOR}/libSDL3.dylib"
  )
endif()

execute_process(
  COMMAND zip
    -r ${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}.zip
    ${CPACK_PACKAGE_FILE_NAME}
  WORKING_DIRECTORY ${CPACK_TEMPORARY_DIRECTORY}
)
