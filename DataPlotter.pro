QT += core gui serialport widgets printsupport

CONFIG += c++11

SOURCES += \
    channel.cpp \
    main.cpp \
    mainwindow.cpp \
    myplot.cpp \
    myterminal.cpp \
    plotdata.cpp \
    qcustomplot.cpp \
    receivedoutputhandler.cpp \
    serialparser.cpp \
    serialworker.cpp

HEADERS += \
    channel.h \
    mainwindow.h \
    myplot.h \
    myterminal.h \
    plotdata.h \
    qcustomplot.h \
    receivedoutputhandler.h \
    serialparser.h \
    serialworker.h \
    settings.h

FORMS += mainwindow.ui

TRANSLATIONS += \
    translation_en.ts \
    translation_cz.ts

RESOURCES += resources.qrc
