#include <QApplication>
#include <QSerialPort>

#include "buffer.h"
#include "mainwindow.h"
#include "myterminal.h"
#include "plotdata.h"
#include "qcustomplot.h"
#include "serialparser.h"
#include "serialthread.h"
#include "settings.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  Settings settings;
  PlotData plotData(&settings);
  SerialParser serialParser(&settings);
  SerialThread serialThread;
  w.init(&settings);
  QObject::connect(&w, &MainWindow::connectSerial, &serialThread, &SerialThread::begin);
  QObject::connect(&w, &MainWindow::disconnectSerial, &serialThread, &SerialThread::end);
  QObject::connect(&serialThread, &SerialThread::connectionResult, &w, &MainWindow::serialConnectResult);
  QObject::connect(&plotData, &PlotData::dataReady, &w, &MainWindow::addDataToPlot);
  // Serial -> MainWindow
  QObject::connect(&serialParser, &SerialParser::changedMode, &w, &MainWindow::setDataMode);
  QObject::connect(&serialParser, &SerialParser::printMessage, &w, &MainWindow::printMessage);
  QObject::connect(&serialParser, &SerialParser::newProcessedCommand, &w, &MainWindow::showProcessedCommand);
  QObject::connect(&serialParser, &SerialParser::changedBinSettings, &w, &MainWindow::changeBinSettings);

  // Serial -> Plotting
  QObject::connect(&serialParser, &SerialParser::newDataString, &plotData, &PlotData::newDataString);
  QObject::connect(&serialParser, &SerialParser::newDataBin, &plotData, &PlotData::newDataBin);

  // Serial -> Terminal
  // connect(serialHandler, SIGNAL(printToTerminal(QByteArray)), ui->myTerminal, SLOT(printToTerminal(QByteArray)));

  QObject::connect(&serialThread, &SerialThread::newData, &serialParser, &SerialParser::parseData);
  QObject::connect(&serialThread, &SerialThread::newCommand, &serialParser, &SerialParser::parseCommand);
  w.show();
  return a.exec();
}
