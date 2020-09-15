QT += core gui serialport widgets printsupport

CONFIG += c++11

SOURCES += \
    buffer.cpp \
    channel.cpp \
    main.cpp \
    mainwindow.cpp \
    myplot.cpp \
    myterminal.cpp \
    plotting.cpp \
    qcustomplot.cpp \
    serialhandler.cpp

HEADERS += \
    buffer.h \
    channel.h \
    enums.h \
    mainwindow.h \
    myplot.h \
    myterminal.h \
    plotting.h \
    qcustomplot.h \
    serialhandler.h

FORMS += mainwindow.ui

TRANSLATIONS += \
    translation_en.ts \
    translation_cz.ts

RESOURCES += resources.qrc
