QT += core gui serialport widgets printsupport

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mainwindow_autoset.cpp \
    mainwindow_csv.cpp \
    mainwindow_cursors.cpp \
    mainwindow_gui_preset.cpp \
    mainwindow_gui_slots.cpp \
    mainwindow_save_load.cpp \
    mainwindow_timed_events.cpp \
    mydial.cpp \
    mymainplot.cpp \
    myplot.cpp \
    myterminal.cpp \
    myxyplot.cpp \
    outputworker.cpp \
    plotdata.cpp \
    plotmath.cpp \
    qcustomplot.cpp \
    serialparser.cpp \
    serialworker.cpp

HEADERS += \
    colorCodes256.h \
    enums_defines_constants.h \
    mainwindow.h \
    mydial.h \
    mymainplot.h \
    myplot.h \
    myterminal.h \
    myxyplot.h \
    outputworker.h \
    plotdata.h \
    plotmath.h \
    qcustomplot.h \
    serialparser.h \
    serialworker.h

FORMS += mainwindow.ui

TRANSLATIONS += \
    translation_en.ts \
    translation_cz.ts

RESOURCES += resources.qrc


