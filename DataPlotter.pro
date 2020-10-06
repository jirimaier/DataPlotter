QT += core gui serialport widgets printsupport

CONFIG += c++11

SOURCES += \
    channel.cpp \
    main.cpp \
    mainwindow.cpp \
    mydial.cpp \
    myplot.cpp \
    myterminal.cpp \
    outputworker.cpp \
    plotdata.cpp \
    plotmath.cpp \
    qcustomplot.cpp \
    serialparser.cpp \
    serialworker.cpp

HEADERS += \
    channel.h \
    colorCodes256.h \
    mainwindow.h \
    mydial.h \
    myplot.h \
    myterminal.h \
    outputworker.h \
    plotdata.h \
    plotmath.h \
    qcustomplot.h \
    serialparser.h \
    serialworker.h \
    settings.h

FORMS += mainwindow.ui

TRANSLATIONS += \
    translation_en.ts \
    translation_cz.ts

RESOURCES += resources.qrc
