IF(WIN32 AND NOT UNIX)

    # Libs needed for packaging
    FIND_PACKAGE(ZLIB REQUIRED)
    FIND_PACKAGE(CURL REQUIRED)

    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BibleTime for Windows svn")
    SET(CPACK_PACKAGE_VENDOR "http://www.bibletime.info")
    SET(CPACK_PACKAGE_VERSION_MAJOR ${BT_VERSION_MAJOR})
    SET(CPACK_PACKAGE_VERSION_MINOR ${BT_VERSION_MINOR})
    SET(CPACK_PACKAGE_VERSION_PATCH ${BT_VERSION_PATCH})
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "BibleTime")

    SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
    SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.

    # We need the libraries, and they're not pulled in automatically
    SET(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)

    SET(QT_BINARY_DIR "${Qt5Core_DIR}/../../../bin")
    INSTALL(FILES
        "${QT_BINARY_DIR}/icudt51d.dll"
        "${QT_BINARY_DIR}/icuin51d.dll"
        "${QT_BINARY_DIR}/icuuc51d.dll"
        "${QT_BINARY_DIR}/libEGLd.dll"
        "${QT_BINARY_DIR}/libGLESv2d.dll"
        "${QT_BINARY_DIR}/Qt5Cored.dll"
        "${QT_BINARY_DIR}/Qt5Guid.dll"
        "${QT_BINARY_DIR}/Qt5Multimediad.dll"
        "${QT_BINARY_DIR}/Qt5MultimediaWidgetsd.dll"
        "${QT_BINARY_DIR}/Qt5Networkd.dll"
        "${QT_BINARY_DIR}/Qt5OpenGLd.dll"
        "${QT_BINARY_DIR}/Qt5Qmld.dll"
        "${QT_BINARY_DIR}/Qt5Quickd.dll"
        "${QT_BINARY_DIR}/Qt5Positioningd.dll"
        "${QT_BINARY_DIR}/Qt5PrintSupportd.dll"
        "${QT_BINARY_DIR}/Qt5Sensorsd.dll"
        "${QT_BINARY_DIR}/Qt5Sqld.dll"
        "${QT_BINARY_DIR}/Qt5Svgd.dll"
        "${QT_BINARY_DIR}/Qt5WebKitd.dll"
        "${QT_BINARY_DIR}/Qt5WebKitWidgetsd.dll"
        "${QT_BINARY_DIR}/Qt5Widgetsd.dll"
        "${QT_BINARY_DIR}/Qt5Xmld.dll"
        "${QT_BINARY_DIR}/Qt5XmlPatternsd.dll"
        DESTINATION "${BT_DESTINATION}"
        CONFIGURATIONS "Debug"
    )
    INSTALL(FILES
        "${QT_BINARY_DIR}/icudt51.dll"
        "${QT_BINARY_DIR}/icuin51.dll"
        "${QT_BINARY_DIR}/icuuc51.dll"
        "${QT_BINARY_DIR}/libEGL.dll"
        "${QT_BINARY_DIR}/libGLESv2.dll"
        "${QT_BINARY_DIR}/Qt5Core.dll"
        "${QT_BINARY_DIR}/Qt5Gui.dll"
        "${QT_BINARY_DIR}/Qt5Multimedia.dll"
        "${QT_BINARY_DIR}/Qt5MultimediaWidgets.dll"
        "${QT_BINARY_DIR}/Qt5Network.dll"
        "${QT_BINARY_DIR}/Qt5OpenGL.dll"
        "${QT_BINARY_DIR}/Qt5Qml.dll"
        "${QT_BINARY_DIR}/Qt5Quick.dll"
        "${QT_BINARY_DIR}/Qt5Positioning.dll"
        "${QT_BINARY_DIR}/Qt5PrintSupport.dll"
        "${QT_BINARY_DIR}/Qt5Sensors.dll"
        "${QT_BINARY_DIR}/Qt5Sql.dll"
        "${QT_BINARY_DIR}/Qt5Svg.dll"
        "${QT_BINARY_DIR}/Qt5WebKit.dll"
        "${QT_BINARY_DIR}/Qt5WebKitWidgets.dll"
        "${QT_BINARY_DIR}/Qt5Widgets.dll"
        "${QT_BINARY_DIR}/Qt5Xml.dll"
        "${QT_BINARY_DIR}/Qt5XmlPatterns.dll"
        DESTINATION "${BT_DESTINATION}"
        CONFIGURATIONS "Release"
    )
	INSTALL(FILES
	    "cmake/platforms/windows/qt.conf"
        DESTINATION "${BT_DESTINATION}"
        CONFIGURATIONS "Release"
    )
    SET(QT_PLUGINS_DIR "${Qt5Core_DIR}/../../../plugins")
    INSTALL(FILES
        "${QT_PLUGINS_DIR}/iconengines/qsvgicon.dll"
        DESTINATION "${BT_DESTINATION}/plugins/iconengines"
        CONFIGURATIONS "Release"
    )
    INSTALL(FILES
        "${QT_PLUGINS_DIR}/platforms/qminimal.dll"
        "${QT_PLUGINS_DIR}/platforms/qoffscreen.dll"
        "${QT_PLUGINS_DIR}/platforms/qwindows.dll"
        DESTINATION "${BT_DESTINATION}/plugins/platforms"
        CONFIGURATIONS "Release"
    )
    INSTALL(FILES
        "${QT_PLUGINS_DIR}/imageformats/qgif.dll"
        "${QT_PLUGINS_DIR}/imageformats/qico.dll"
        "${QT_PLUGINS_DIR}/imageformats/qjpeg.dll"
        "${QT_PLUGINS_DIR}/imageformats/qmng.dll"
        "${QT_PLUGINS_DIR}/imageformats/qsvg.dll"
        "${QT_PLUGINS_DIR}/imageformats/qtiff.dll"
        DESTINATION "${BT_DESTINATION}/plugins/imageformats"
        CONFIGURATIONS "Release"
    )

    # This adds in the required Windows system libraries
    MESSAGE(STATUS  "INSTALL Microsoft Redist ${MSVC_REDIST}" )
    INSTALL(PROGRAMS ${MSVC_REDIST} DESTINATION bin)
    SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        ExecWait \\\"$INSTDIR\\\\bin\\\\vcredist_x86.exe  /q\\\"
        Delete   \\\"$INSTDIR\\\\bin\\\\vcredist_x86.exe\\\"
    ")

    MESSAGE(STATUS  "INSTALL CLucene_LIBRARY ${CLucene_LIBRARY}" )
    STRING(REPLACE ".lib" ".dll" CLUCENE_DLL "${CLucene_LIBRARY}")
    INSTALL(FILES ${CLUCENE_DLL} DESTINATION ${BT_DESTINATION})

    MESSAGE(STATUS  "INSTALL CLucene_LIBRARY ${CLucene_SHARED_LIB}" )
    STRING(REPLACE ".lib" ".dll" CLUCENE_SHARED_DLL "${CLucene_SHARED_LIB}")
    INSTALL(FILES ${CLUCENE_SHARED_DLL} DESTINATION ${BT_DESTINATION})

    MESSAGE(STATUS  "INSTALL CURL_LIBRARY ${CURL_LIBRARY}" )
    STRING(REPLACE "_imp.lib" ".dll" CURL_DLL "${CURL_LIBRARY}")
    INSTALL(FILES ${CURL_DLL} DESTINATION ${BT_DESTINATION})

    # Some options for the CPack system.  These should be pretty self-evident
    SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\pics\\\\icons\\\\bibletime.png")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\bibletime.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} for Windows")
    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.bibletime.info")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.bibletime.info")
    SET(CPACK_NSIS_CONTACT "bt-devel@crosswire.org")
    SET(CPACK_NSIS_MODIFY_PATH OFF)
    SET(CPACK_GENERATOR "NSIS")

    SET(CPACK_PACKAGE_EXECUTABLES "bibletime" "BibleTime")

    INCLUDE(CPack)

ENDIF(WIN32 AND NOT UNIX)

