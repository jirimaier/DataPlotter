QT += core gui serialport widgets printsupport opengl

RC_ICONS = icons/icon.ico

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mainwindow_autoset.cpp \
    mainwindow_cursors.cpp \
    mainwindow_export.cpp \
    mainwindow_gui_preset.cpp \
    mainwindow_gui_slots.cpp \
    mainwindow_save_load.cpp \
    mainwindow_timed_events.cpp \
    myaxistickerwithunit.cpp \
    mycursorslider.cpp \
    mydial.cpp \
    myfftplot.cpp \
    mymainplot.cpp \
    mymodifiedqcptracer.cpp \
    myplot.cpp \
    myterminal.cpp \
    myxyplot.cpp \
    newserialparser.cpp \
    plotdata.cpp \
    plotmath.cpp \
    qcustomplot.cpp \
    serialreader.cpp \
    signalprocessing.cpp

HEADERS += \
    enums_defines_constants.h \
    mainwindow.h \
    myaxistickerwithunit.h \
    mycursorslider.h \
    mydial.h \
    myfftplot.h \
    mymainplot.h \
    mymodifiedqcptracer.h \
    myplot.h \
    myterminal.h \
    myxyplot.h \
    newserialparser.h \
    plotdata.h \
    plotmath.h \
    qcustomplot.h \
    serialreader.h \
    signalprocessing.h

FORMS += mainwindow.ui

TRANSLATIONS += \
    translations/translation_en.ts \
    translations/translation_cz.ts

RESOURCES += resources.qrc


