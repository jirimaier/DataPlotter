QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    buffer.cpp \
    channel.cpp \
    main.cpp \
    mainwindow.cpp \
    myterminal.cpp \
    plotting.cpp \
    qcustomplot.cpp \
    serialHandler.cpp

HEADERS += \
    buffer.h \
    channel.h \
    enums.h \
    mainwindow.h \
    myterminal.h \
    plotting.h \
    qcustomplot.h \
    serialHandler.h
FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    translation_en.ts translation_cz.ts

DEFINES += QCUSTOMPLOT_USE_OPENGL

LIBS += -lOpenGL32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
