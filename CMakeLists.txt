cmake_minimum_required(VERSION 3.18)

project(DataPlotter VERSION 3.3.5 LANGUAGES CXX DESCRIPTION "GUI interface for custom software-defined instruments")

# ============================================================================
# Project Metadata
# ============================================================================
set(JIRI_MAIER "Jiri Maier <jiri.maier.x@gmail.com>")
set(PROJECT_HOMEPAGE_URL "https://jirimaier.github.io/DataPlotter/")
set(GENERIC_NAME "DataPlotter")
set(LICENCE "GPL-3.0-or-later")
set(LONG_DESCRIPTION "GUI interface for custom software-defined instruments.")

# ============================================================================
# CMake and Compiler Config
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Output paths
set(DEV_MODE false CACHE BOOL "Enable developer options like packaging.")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/target" CACHE STRING "Path to place executables")
set(PACKAGE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/pkg" CACHE STRING "Path to place generated packages")

# QML Module Dependencies
set(QML_DEPENDS_LIST
    "qml-module-qtquick-controls2"
    "qml-module-qtquick-controls"
    "qml-module-qtquick-layouts"
    "qml-module-qtquick-dialogs"
    "qml-module-qtgraphicaleffects"
    "qml-module-qtquick-window2"
    "qml-module-qtquick2"
    "qml-module-qtcharts")
string (REPLACE ";" "," QML_DEPENDS "${QML_DEPENDS_LIST}")

# Get current year
string(TIMESTAMP CURRENT_YEAR "%Y")

# ============================================================================
# Include Paths and Definitions
# ============================================================================
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# ============================================================================
# Global Project Info
# ============================================================================
set(MAIN_PROJECT_NAME ${PROJECT_NAME})
set(MAIN_PROJECT_NAME_LOWER "data-plotter")
set(MAIN_PROJECT_VERSION ${PROJECT_VERSION})
set(MAIN_PROJECT_APPID "cz.cvut.fel.embedded.sdi.data-plotter")
set(MAIN_PROJECT_ORGANIZATION "FEE CTU")
set(MAIN_PROJECT_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})

if(WIN32)
    set(MAIN_EXECUTEBLE_NAME ${MAIN_PROJECT_NAME})
else()
    set(MAIN_EXECUTEBLE_NAME ${MAIN_PROJECT_NAME_LOWER})
endif()

# ============================================================================
# Version and Copyright
# ============================================================================
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/version.h)
include(cmake/CopyrightTools.cmake)
copyright("Copyright (c) 2020-${CURRENT_YEAR} ${JIRI_MAIER}")
include(cmake/GPL-3.0-or-later.cmake)

# ============================================================================
# Dependencies
# ============================================================================
find_package(OpenGL REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets Core Gui SerialPort PrintSupport OpenGL Qml QuickWidgets Quick
    QuickControls2 Network LinguistTools)

set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/forms")

# ============================================================================
# Translations and Resources
# ============================================================================
set(TS_FILES
    translations/dataplotter_cs.ts
    translations/dataplotter_en.ts)

qt5_add_translation(QM_FILES ${TS_FILES})
include(TranslationUtils)
add_app_translations_resource(APP_RES ${QM_FILES})
add_qt_translations_resource(QT_RES en cs)

set(RESOURCES resources.qrc)
qt5_add_resources(RESOURCE_FILES ${RESOURCES})

# ============================================================================
# Source Files
# ============================================================================
file(GLOB_RECURSE PROJECT_HEADERFILES src/*.h)
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp src/forms/*.ui)

list(APPEND PROJECT_SOURCES ${RESOURCE_FILES} ${PROJECT_HEADERFILES})

# Platform-Specific Configurations
if(WIN32)
    configure_file(${CMAKE_SOURCE_DIR}/extras/deploy_windows/resources.rc.in
                   ${CMAKE_BINARY_DIR}/resources.rc @ONLY)
    list(APPEND PROJECT_SOURCES ${CMAKE_BINARY_DIR}/resources.rc)
    configure_file(${CMAKE_SOURCE_DIR}/icons/icon.ico ${CMAKE_BINARY_DIR}/icon.ico COPYONLY)
endif()

if(APPLE)
    set(ICON_NAME data-plotter)
    set(ICON_PATH ${CMAKE_SOURCE_DIR}/extras/deploy_macos/${ICON_NAME}.icns)
    set_property(SOURCE ${ICON_PATH} PROPERTY MACOSX_PACKAGE_LOCATION Resources)
endif()

# ============================================================================
# Executable Target
# ============================================================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DataPlotter MANUAL_FINALIZATION ${ICON_PATH} ${PROJECT_SOURCES})
else()
    add_executable(DataPlotter ${PROJECT_SOURCES} ${ICON_PATH} ${APP_RES} ${QT_RES})
endif()

set_target_properties(DataPlotter PROPERTIES OUTPUT_NAME ${MAIN_EXECUTEBLE_NAME})
target_compile_definitions(DataPlotter PRIVATE QCUSTOMPLOT_USE_OPENGL)

# Linking
target_link_libraries(DataPlotter PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Network
    OpenGL::GL)

if(WIN32)
    set_target_properties(DataPlotter PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DataPlotter)
endif()

# ============================================================================
# Custom Targets
# ============================================================================
add_custom_target(clean-all
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning the build directory")

add_custom_target(lupdate
    COMMAND ${Qt${QT_VERSION_MAJOR}_LUPDATE_EXECUTABLE} -no-obsolete ${PROJECT_SOURCE_DIR} -ts ${TS_FILES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Updating translations")

add_dependencies(DataPlotter lupdate)

# MacOS Bundle Info
if(APPLE)
    set_target_properties(DataPlotter PROPERTIES
        MACOSX_BUNDLE true
        MACOSX_BUNDLE_GUI_IDENTIFIER cz.cvut.fel.embedded.${MAIN_PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_NAME ${MAIN_PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION "${MAIN_PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${MAIN_PROJECT_VERSION}"
        MACOSX_BUNDLE_ICONFILE ${ICON_NAME})
endif()

# ============================================================================
# Installation
# ============================================================================
install(TARGETS DataPlotter RUNTIME DESTINATION bin BUNDLE DESTINATION ${EXECUTABLE_OUTPUT_PATH})
install(FILES extras/deploy_debian_native/icon.png DESTINATION share/icons/hicolor/256x256/apps RENAME data-plotter.png)
install(FILES extras/deploy_debian_native/data-plotter.desktop DESTINATION share/applications)

# ============================================================================
# Packaging Tools
# ============================================================================
if("${DEV_MODE}")
    message(STATUS "Packaging tools enabled.")

    set(PACKAGE_NAME "${MAIN_PROJECT_NAME_LOWER}")
    set(PACKAGE_VERSION "${PROJECT_VERSION}")
    set(PACKAGE_RELEASE "1")
    set(PACKAGE_SOURCE_ARCHIVE_FILE "${PACKAGE_NAME}_${PACKAGE_VERSION}.orig.tar.xz")
    set(PACKAGE_SOURCE_ARCHIVE_PATH "${PACKAGE_OUTPUT_PATH}/${PACKAGE_SOURCE_ARCHIVE_FILE}")
    set(PACKAGE_TOPLEVEL_DIR "${PACKAGE_NAME}-${PACKAGE_VERSION}")
    set(PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
    set(PACKAGE_LONG_DESCRIPTION "${LONG_DESCRIPTION}")
    set(PACKAGE_MAINTAINER "${JIRI_MAIER}")
    set(PACKAGE_URL "${PROJECT_HOMEPAGE_URL}")
    set(PACKAGE_GIT "https://github.com/jirimaier/DataPlotter")
    set(PACKAGE_LICENCE "${LICENCE} ")

    include(cmake/PackageTools.cmake)

    package_config_file(appimage appimage.yml extras/packaging/appimage/appimage.yml.in)
    package_config_file(archlinux PKGBUILD extras/packaging/arch/PKGBUILD.in)
    package_config_file(rpm ${PACKAGE_NAME}.spec extras/packaging/rpm/spec.in)

    package_debian_quilt(deb
        ${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}.dsc
        extras/packaging/deb/dsc.in
        extras/packaging/deb/debian
        ${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}.debian.tar.xz)

    add_custom_target(open_build_service_bundle
        DEPENDS ${PACKAGE_SOURCE_ARCHIVE_FILE} appimage archlinux deb rpm
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/pkg)
endif()
