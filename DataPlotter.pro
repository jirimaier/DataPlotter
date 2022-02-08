QT += core gui serialport widgets printsupport opengl qml quickwidgets \
    quick

LIBS += -lOpenGL32

RC_ICONS = icons/icon.ico

DEFINES += QT_DEPRECATED_WARNINGS QCUSTOMPLOT_USE_OPENGL

CONFIG += c++11

SOURCES += \
    averager.cpp \
    clickablelabel.cpp \
    cobs.cpp \
    filesender.cpp \
    interpolator.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow_autoset.cpp \
    mainwindow_cursors.cpp \
    mainwindow_cursors_xy.cpp \
    mainwindow_export.cpp \
    mainwindow_gui_preset.cpp \
    mainwindow_gui_slots.cpp \
    mainwindow_qml.cpp \
    mainwindow_save_load.cpp \
    mainwindow_send_file.cpp \
    mainwindow_timed_events.cpp \
    myaxistickerwithunit.cpp \
    mycursorslider.cpp \
    mydial.cpp \
    mydoublespinboxwithunits.cpp \
    myelidedcombobox.cpp \
    myfftplot.cpp \
    myframewithresizesignal.cpp \
    mymainplot.cpp \
    mymodifiedqcptracer.cpp \
    mypeakplot.cpp \
    myplot.cpp \
    mypow2spinbox.cpp \
    myscaledoublespinbox.cpp \
    myterminal.cpp \
    myxyplot.cpp \
    newserialparser.cpp \
    plotdata.cpp \
    plotmath.cpp \
    qcustomplot.cpp \
    qmlterminalinterface.cpp \
    serialreader.cpp \
    serialsettingsdialog.cpp \
    signalprocessing.cpp \
    xymode.cpp

HEADERS += \
    averager.h \
    clickablelabel.h \
    cobs.h \
    filesender.h \
    global.h \
    interpolator.h \
    mainwindow.h \
    myaxistickerwithunit.h \
    mycursorslider.h \
    mydial.h \
    mydoublespinboxwithunits.h \
    myelidedcombobox.h \
    myfftplot.h \
    myframewithresizesignal.h \
    mymainplot.h \
    mymodifiedqcptracer.h \
    mypeakplot.h \
    myplot.h \
    mypow2spinbox.h \
    myscaledoublespinbox.h \
    myterminal.h \
    myxyplot.h \
    newserialparser.h \
    plotdata.h \
    plotmath.h \
    qcustomplot.h \
    qmlterminalinterface.h \
    serialreader.h \
    serialsettingsdialog.h \
    signalprocessing.h \
    xymode.h

FORMS += mainwindow.ui \
    serialsettingsdialog.ui

TRANSLATIONS += \
    translations/translation_en.ts \
    translations/translation_cz.ts

RESOURCES += resources.qrc

DISTFILES += \
    ExampleQmlTerminal.qml


