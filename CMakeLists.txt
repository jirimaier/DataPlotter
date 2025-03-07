cmake_minimum_required(VERSION 3.16)

project(DataPlotter VERSION 3.3.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/src/forms")

set(TS_FILES
    translations/translation_en.ts
    translations/translation_cz.ts
)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(RESOURCES resources.qrc)
qt5_add_resources(RESOURCE_FILES ${RESOURCES})

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
    ${TS_FILES}
    ${RESOURCE_FILES}
    ${PROJECT_HEADERFILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DataPlotter
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(DataPlotter SHARED ${PROJECT_SOURCES})
    else()
        add_executable(DataPlotter ${PROJECT_SOURCES})
endif()
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

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
)

if(${QT_VERSION_MAJOR} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DataPlotter)
endif()

set_target_properties(DataPlotter PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DataPlotter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DataPlotter)
endif()
