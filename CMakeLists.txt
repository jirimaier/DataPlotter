cmake_minimum_required(VERSION 3.16)

project(DataPlotter
        VERSION 3.3.1
        LANGUAGES CXX
        DESCRIPTION "GUI interface for custom software-defined instruments")

set(MAIN_PROJECT_NAME_LOWER "data-plotter")

if(WIN32)
    set(MAIN_EXECUTEBLE_NAME "DataPlotter")
else()
    set(MAIN_EXECUTEBLE_NAME "data-plotter")
endif()


# Get current year dynamically
string(TIMESTAMP CURRENT_YEAR "%Y")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JIRI_MAIER Jiri Maier <jiri.maier.x@gmail.com>)

set(PROJECT_HOMEPAGE_URL "https://github.com/jirimaier/DataPlotter")
set(GENERIC_NAME "DataPlotter")
set(LICENCE "GPL-3.0-or-later")
set(LONG_DESCRIPTION
        "GUI interface for custom software-defined instruments.")

include(cmake/CopyrightTools.cmake)

copyright(
        "Copyright (c) 2020-2025 ${JIRI_MAIER}")

include(cmake/GPL-3.0-or-later.cmake)

# =============================================================================
# Configurable options
# =============================================================================

set(DEV_MODE false CACHE BOOL "Enable developer options in this CMake, like packaging.\
    They should be ignored, when user just wants to build this project.")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/target"
        CACHE STRING "Absolute path to place executables to.")
set(PACKAGE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/pkg"
        CACHE STRING "Absolute path to place generated package files.")

# =============================================================================
# Generated variables
# =============================================================================

# I don't want to relly on the assumption, that this file is invoked as root
# project. Therefore I propagate the information to all subprojects
# MAIN_PROJECT_*. Lowercase and uppercase are used for executable names and
# C defines, respectively.
set(MAIN_PROJECT_NAME "${PROJECT_NAME}")
set(MAIN_PROJECT_VERSION "${PROJECT_VERSION}")
set(MAIN_PROJECT_APPID "cz.cvut.edu.comparch.qtrvsim")
set(MAIN_PROJECT_ORGANIZATION "FEE CTU")
set(MAIN_PROJECT_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
#string(TOLOWER "${PROJECT_NAME}" MAIN_PROJECT_NAME_LOWER)
string(TOUPPER "${PROJECT_NAME}" MAIN_PROJECT_NAME_UPPER)

# =============================================================================
# CMake config and tools
# =============================================================================

find_package(OpenGL REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets
    LinguistTools
    Core
    Gui
    SerialPort
    PrintSupport
    OpenGL
    Qml
    QuickWidgets
    Quick
    QuickControls2
    Network
)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/forms")

include_directories(${CMAKE_SOURCE_DIR}/src)

set(TS_FILES
    translations/dataplotter_cs.ts
    translations/dataplotter_en.ts
)

qt5_add_translation(QM_FILES ${TS_FILES})
include(TranslationUtils)
add_app_translations_resource(APP_RES ${QM_FILES})
add_qt_translations_resource(QT_RES en cs)

set(RESOURCES resources.qrc)
qt_add_resources(RESOURCE_FILES ${RESOURCES})

set(PROJECT_HEADERFILES
    src/communication/cobs.h
    src/communication/filesender.h
    src/communication/newserialparser.h
    src/communication/plotdata.h
    src/communication/serialreader.h
    src/communication/serialsettingsdialog.h
    src/communication/telnetserver.h
    src/customwidgets/checkbuttons.h
    src/customwidgets/clickablelabel.h
    src/customwidgets/mycursorslider.h
    src/customwidgets/mydial.h
    src/customwidgets/mydoublespinboxwithunits.h
    src/customwidgets/myelidedcombobox.h
    src/customwidgets/myframewithresizesignal.h
    src/customwidgets/mypow2spinbox.h
    src/customwidgets/myscaledoublespinbox.h
    src/defaultpathmanager.h
    src/developeroptions.h
    src/freqtimeplotdialog.h
    src/mainwindow/appsettings.h
    src/mainwindow/mainwindow.h
    src/mainwindow/updatechecker.h
    src/manualinputdialog.h
    src/math/averager.h
    src/math/expressionparser.h
    src/math/interpolator.h
    src/math/plotmath.h
    src/math/signalprocessing.h
    src/math/simpleexpressionparser.h
    src/math/variableexpressionparser.h
    src/math/xymode.h
    src/customwidgets/myterminal.h
    src/plots/myaxistickerwithunit.h
    src/plots/myfftplot.h
    src/plots/mymainplot.h
    src/plots/mymodifiedqcptracer.h
    src/plots/mypeakplot.h
    src/plots/myplot.h
    src/plots/myxyplot.h
    src/plots/qcustomplot.h
    src/qml/ansiterminalmodel.h
    src/qml/messagemodel.h
    src/qml/qmlterminalinterface.h
    src/utils.h
)

set(PROJECT_SOURCES
    src/main.cpp
    src/communication/cobs.cpp
    src/communication/filesender.cpp
    src/communication/newserialparser.cpp
    src/communication/plotdata.cpp
    src/communication/serialreader.cpp
    src/communication/serialsettingsdialog.cpp
    src/communication/telnetserver.cpp
    src/customwidgets/checkbuttons.cpp
    src/customwidgets/clickablelabel.cpp
    src/customwidgets/mycursorslider.cpp
    src/customwidgets/mydial.cpp
    src/customwidgets/mydoublespinboxwithunits.cpp
    src/customwidgets/myelidedcombobox.cpp
    src/customwidgets/myframewithresizesignal.cpp
    src/customwidgets/mypow2spinbox.cpp
    src/customwidgets/myscaledoublespinbox.cpp
    src/defaultpathmanager.cpp
    src/developeroptions.cpp
    src/freqtimeplotdialog.cpp
    src/mainwindow/appsettings.cpp
    src/mainwindow/mainwindow.cpp
    src/mainwindow/mainwindow_autoset.cpp
    src/mainwindow/mainwindow_cursors.cpp
    src/mainwindow/mainwindow_cursors_xy.cpp
    src/mainwindow/mainwindow_export.cpp
    src/mainwindow/mainwindow_gui_preset.cpp
    src/mainwindow/mainwindow_gui_slots.cpp
    src/mainwindow/mainwindow_qml.cpp
    src/mainwindow/mainwindow_send_file.cpp
    src/mainwindow/mainwindow_timed_events.cpp
    src/mainwindow/updatechecker.cpp
    src/manualinputdialog.cpp
    src/math/averager.cpp
    src/math/expressionparser.cpp
    src/math/interpolator.cpp
    src/math/plotmath.cpp
    src/math/signalprocessing.cpp
    src/math/simpleexpressionparser.cpp
    src/math/variableexpressionparser.cpp
    src/math/xymode.cpp
    src/customwidgets/myterminal.cpp
    src/plots/myaxistickerwithunit.cpp
    src/plots/myfftplot.cpp
    src/plots/mymainplot.cpp
    src/plots/mymodifiedqcptracer.cpp
    src/plots/mypeakplot.cpp
    src/plots/myplot.cpp
    src/plots/myxyplot.cpp
    src/plots/qcustomplot.cpp
    src/qml/ansiterminalmodel.cpp
    src/qml/messagemodel.cpp
    src/qml/qmlterminalinterface.cpp
    src/utils.cpp
    src/forms/mainwindow.ui
    src/forms/developeroptions.ui
    src/forms/freqtimeplotdialog.ui
    src/forms/manualinputdialog.ui
    src/forms/serialsettingsdialog.ui
    ${RESOURCE_FILES}
    ${PROJECT_HEADERFILES}
)

configure_file(
    ${CMAKE_SOURCE_DIR}/deploy_windows/resources.rc.in
    ${CMAKE_BINARY_DIR}/resources.rc
    @ONLY
)

if(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${CMAKE_BINARY_DIR}/resources.rc)
    # Ensure the icon is copied to the build directory
    set(APP_ICON ${CMAKE_BINARY_DIR}/icon.ico)
    configure_file(${CMAKE_SOURCE_DIR}/icons/icon.ico ${APP_ICON} COPYONLY)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DataPlotter MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(DataPlotter ${PROJECT_SOURCES} ${APP_RES} ${QT_RES})
endif()

# Define the preprocessor macro for QCustomPlot OpenGL support
target_compile_definitions(DataPlotter PRIVATE QCUSTOMPLOT_USE_OPENGL)

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
    OpenGL::GL
)

if(WIN32)
    set_target_properties(DataPlotter PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DataPlotter)
endif()

add_custom_target(clean-all
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning the build directory"
)

add_custom_target(lupdate
    COMMAND ${Qt${QT_VERSION_MAJOR}_LUPDATE_EXECUTABLE} -no-obsolete ${PROJECT_SOURCE_DIR} -ts ${TS_FILES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Updating translations"
)

# =============================================================================
# Installation
# =============================================================================

# Prior to CMake version 3.13, installation must be performed in the subdirectory,
# there the target was created. Therefore executable installation is to be found
# in corresponding CMakeLists.txt.

if (NOT "${WASM}")
#   install(FILES "deploy_linux/icons/icon.svg"
#           DESTINATION "share/icons/hicolor/scalable/apps"
#           RENAME "${MAIN_PROJECT_NAME_LOWER}.svg")
    install(FILES "deploy_linux/icon.png"
            DESTINATION "share/icons/hicolor/256x256/apps"
            RENAME "${MAIN_PROJECT_NAME_LOWER}.png")
    install(FILES "deploy_linux/${MAIN_PROJECT_NAME_LOWER}.desktop"
            DESTINATION share/applications)

#   install(FILES "${EXECUTABLE_OUTPUT_PATH}/${MAIN_PROJECT_APPID}.metainfo.xml"
#           DESTINATION share/metainfo)
endif ()

# =============================================================================
# Packages
# =============================================================================

if ("${DEV_MODE}")
    # The condition prevents execution of this section during regular user installation.
    # It created files useless to normal users and requires additional tools (git, xz).
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
    set(PACKAGE_GIT "github.com:/jirimaier/DataPlotter")
    set(PACKAGE_LICENCE "${LICENCE} ")

    include(cmake/PackageTools.cmake)

    # Inject up-to-date information into package config files.
    package_config_file(appimage appimage.yml extras/packaging/appimage/appimage.yml.in)
    package_config_file(archlinux PKGBUILD extras/packaging/arch/PKGBUILD.in)
    package_config_file(rpm ${PACKAGE_NAME}.spec extras/packaging/rpm/spec.in)
    # Debian uses whole directory which has to be saved to archive and shipped.
    package_debian_quilt(deb
            ${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}.dsc
            extras/packaging/deb/dsc.in
            extras/packaging/deb/debian
            ${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}.debian.tar.xz)
    # Creates bunch of files in ${CMAKE_BINARY_DIR}/target/pkg that you can just pass to
    # Open Build Service and it will build all packaging.
    # TODO: Currently changelog is not handled automatically.
    add_custom_target(open_build_service_bundle
            DEPENDS ${PACKAGE_SOURCE_ARCHIVE_FILE} appimage archlinux deb rpm
            WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/pkg)
endif ()
=======
add_dependencies(DataPlotter lupdate)
