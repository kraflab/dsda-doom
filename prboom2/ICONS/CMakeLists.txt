if(UNIX AND NOT APPLE)
    install(FILES dsda-doom.desktop DESTINATION share/applications)
    install(FILES dsda-doom.svg DESTINATION share/icons/hicolor/scalable/apps)
endif()
