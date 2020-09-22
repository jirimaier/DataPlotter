#include <QApplication>
#include <QSerialPort>

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
  PlotData plotData;
  SerialParser serialParser;
  SerialThread serialThread;
  w.init();

  QObject::connect(&w, &MainWindow::connectSerial, &serialThread, &SerialThread::begin);
  QObject::connect(&w, &MainWindow::disconnectSerial, &serialThread, &SerialThread::end);
  QObject::connect(&serialThread, &SerialThread::connectionResult, &w, &MainWindow::serialConnectResult);
  QObject::connect(&plotData, &PlotData::dataReady, &w, &MainWindow::addDataToPlot);
  QObject::connect(&serialParser, &SerialParser::changedMode, &w, &MainWindow::changedDataMode);
  QObject::connect(&serialParser, &SerialParser::printMessage, &w, &MainWindow::printMessage);
  QObject::connect(&serialParser, &SerialParser::newProcessedCommand, &w, &MainWindow::showProcessedCommand, Qt::QueuedConnection);
  QObject::connect(&serialParser, &SerialParser::changedBinSettings, &w, &MainWindow::changeBinSettings);
  QObject::connect(&serialParser, &SerialParser::newDataString, &plotData, &PlotData::newDataString, Qt::QueuedConnection);
  QObject::connect(&serialParser, &SerialParser::newDataBin, &plotData, &PlotData::newDataBin, Qt::QueuedConnection);

  // connect(serialHandler, SIGNAL(printToTerminal(QByteArray)), ui->myTerminal, SLOT(printToTerminal(QByteArray)));

  QObject::connect(&serialThread, &SerialThread::newData, &serialParser, &SerialParser::parseData, Qt::QueuedConnection);
  QObject::connect(&serialThread, &SerialThread::newCommand, &serialParser, &SerialParser::parseCommand, Qt::QueuedConnection);
  QObject::connect(&w, &MainWindow::setMode, &serialParser, &SerialParser::setMode);
  QObject::connect(&w, &MainWindow::setBinBits, &serialParser, &SerialParser::setBinBits);
  QObject::connect(&w, &MainWindow::setBinCont, &serialParser, &SerialParser::setBinCont);
  QObject::connect(&w, &MainWindow::setBinFCh, &serialParser, &SerialParser::setBinFCh);
  QObject::connect(&w, &MainWindow::setBinMax, &serialParser, &SerialParser::setBinMax);
  QObject::connect(&w, &MainWindow::setBinMin, &serialParser, &SerialParser::setBinMin);
  QObject::connect(&w, &MainWindow::setBinNCh, &serialParser, &SerialParser::setBinNCh);
  QObject::connect(&w, &MainWindow::setBinStep, &serialParser, &SerialParser::setBinStep);
  w.show();
  return a.exec();
}
