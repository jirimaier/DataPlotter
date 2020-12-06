QT += core gui serialport widgets printsupport opengl

win32:RC_ICONS += icon.ico

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QCUSTOMPLOT_USE_OPENGL

LIBS += -lOpengl32

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
    mycursorslider.cpp \
    mydial.cpp \
    mymainplot.cpp \
    myplot.cpp \
    myterminal.cpp \
    myxyplot.cpp \
    newserialparser.cpp \
    plotdata.cpp \
    plotmath.cpp \
    qcustomplot.cpp \
    serialreader.cpp

HEADERS += \
    colorCodes.h \
    enums_defines_constants.h \
    mainwindow.h \
    mycursorslider.h \
    mydial.h \
    mymainplot.h \
    myplot.h \
    myterminal.h \
    myxyplot.h \
    newserialparser.h \
    plotdata.h \
    plotmath.h \
    qcustomplot.h \
    serialreader.h

FORMS += mainwindow.ui

TRANSLATIONS += \
    translation_en.ts \
    translation_cz.ts

RESOURCES += resources.qrc


