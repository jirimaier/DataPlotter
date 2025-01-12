VERSION = 3.3.0.0

QT += core gui serialport widgets printsupport opengl qml quickwidgets \
    quick quickcontrols2 network

win32{
    LIBS += -lOpenGL32
}

linux-g++* {
    LIBS += -lssl -lcrypto
}

RC_ICONS = icons/icon.ico

DEFINES += QT_DEPRECATED_WARNINGS QCUSTOMPLOT_USE_OPENGL

CONFIG += c++11

FORMS += mainwindow.ui \
    developeroptions.ui \
    freqtimeplotdialog.ui \
    manualinputdialog.ui \
    serialsettingsdialog.ui

TRANSLATIONS += \
    translations/translation_en.ts \
    translations/translation_cz.ts

RESOURCES += resources.qrc

HEADERS += \
    communication/cobs.h \
    communication/filesender.h \
    communication/newserialparser.h \
    communication/plotdata.h \
    communication/serialreader.h \
    communication/serialsettingsdialog.h \
    communication/telnetserver.h \
    customwidgets/checkbuttons.h \
    customwidgets/clickablelabel.h \
    customwidgets/mycursorslider.h \
    customwidgets/mydial.h \
    customwidgets/mydoublespinboxwithunits.h \
    customwidgets/myelidedcombobox.h \
    customwidgets/myframewithresizesignal.h \
    customwidgets/mypow2spinbox.h \
    customwidgets/myscaledoublespinbox.h \
    defaultpathmanager.h \
    developeroptions.h \
    freqtimeplotdialog.h \
    global.h \
    mainwindow/appsettings.h \
    mainwindow/mainwindow.h \
    mainwindow/updatechecker.h \
    manualinputdialog.h \
    math/averager.h \
    math/expressionparser.h \
    math/interpolator.h \
    math/plotmath.h \
    math/signalprocessing.h \
    math/simpleexpressionparser.h \
    math/variableexpressionparser.h \
    math/xymode.h \
    customwidgets/myterminal.h \
    plots/myaxistickerwithunit.h \
    plots/myfftplot.h \
    plots/mymainplot.h \
    plots/mymodifiedqcptracer.h \
    plots/mypeakplot.h \
    plots/myplot.h \
    plots/myxyplot.h \
    plots/qcustomplot.h \
    qml/ansiterminalmodel.h \
    qml/messagemodel.h \
    qml/qmlterminalinterface.h \
    utils.h

SOURCES += \
    communication/cobs.cpp \
    communication/filesender.cpp \
    communication/newserialparser.cpp \
    communication/plotdata.cpp \
    communication/serialreader.cpp \
    communication/serialsettingsdialog.cpp \
    communication/telnetserver.cpp \
    customwidgets/checkbuttons.cpp \
    customwidgets/clickablelabel.cpp \
    customwidgets/mycursorslider.cpp \
    customwidgets/mydial.cpp \
    customwidgets/mydoublespinboxwithunits.cpp \
    customwidgets/myelidedcombobox.cpp \
    customwidgets/myframewithresizesignal.cpp \
    customwidgets/mypow2spinbox.cpp \
    customwidgets/myscaledoublespinbox.cpp \
    defaultpathmanager.cpp \
    developeroptions.cpp \
    freqtimeplotdialog.cpp \
    main.cpp \
    mainwindow/appsettings.cpp \
    mainwindow/mainwindow.cpp \
    mainwindow/mainwindow_autoset.cpp \
    mainwindow/mainwindow_cursors.cpp \
    mainwindow/mainwindow_cursors_xy.cpp \
    mainwindow/mainwindow_export.cpp \
    mainwindow/mainwindow_gui_preset.cpp \
    mainwindow/mainwindow_gui_slots.cpp \
    mainwindow/mainwindow_qml.cpp \
    mainwindow/mainwindow_send_file.cpp \
    mainwindow/mainwindow_timed_events.cpp \
    mainwindow/updatechecker.cpp \
    manualinputdialog.cpp \
    math/averager.cpp \
    math/expressionparser.cpp \
    math/interpolator.cpp \
    math/plotmath.cpp \
    math/signalprocessing.cpp \
    math/simpleexpressionparser.cpp \
    math/variableexpressionparser.cpp \
    math/xymode.cpp \
    customwidgets/myterminal.cpp \
    plots/myaxistickerwithunit.cpp \
    plots/myfftplot.cpp \
    plots/mymainplot.cpp \
    plots/mymodifiedqcptracer.cpp \
    plots/mypeakplot.cpp \
    plots/myplot.cpp \
    plots/myxyplot.cpp \
    plots/qcustomplot.cpp \
    qml/ansiterminalmodel.cpp \
    qml/messagemodel.cpp \
    qml/qmlterminalinterface.cpp \
    utils.cpp


