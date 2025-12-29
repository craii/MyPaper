cmake_minimum_required(VERSION 3.16)

project(wallpaper VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(wallpaper
    main.cpp
    app_icon.rc
)

qt_add_qml_module(wallpaper
    URI wallpaper
    QML_FILES Main.qml
    SOURCES photobackend.h photobackend.cpp
    SOURCES
    QML_FILES
    QML_FILES Settings.qml
    SOURCES settingsbackend.h settingsbackend.cpp
    SOURCES
    SOURCES utils.h utils.cpp
    SOURCES systemtray.h systemtray.cpp
    SOURCES resources.qrc
)

# Set application properties
set_target_properties(wallpaper PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(wallpaper
    PRIVATE Qt6::Quick Qt6::Widgets
)

# Installation setup
include(GNUInstallDirs)
install(TARGETS wallpaper
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Windows-specific installation (copy Qt runtime files)
if(WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    
    # Install essential Qt DLLs
    install(FILES
        "${QT_BIN_DIR}/Qt6Core.dll"
        "${QT_BIN_DIR}/Qt6Gui.dll"
        "${QT_BIN_DIR}/Qt6Qml.dll"
        "${QT_BIN_DIR}/Qt6QmlModels.dll"
        "${QT_BIN_DIR}/Qt6Quick.dll"
        "${QT_BIN_DIR}/Qt6Widgets.dll"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        OPTIONAL
    )
endif()
