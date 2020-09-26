// Autor: Jiří Maier
// vytvořeno: 07/2020

// Qt 5.15

// Pro Window XP nutno použít Qt 5.7
// V Qt 5.7 nefungují adaptivní spinboxy,
// nutno vymazat řádky "... setStepType(QAbstractSpinBox::AdaptiveDecimalStepType)"
// z ui_mainwindow.h

#include <QApplication>
#include <QSerialPort>
#include <QTimer>

#include "mainwindow.h"
#include "myterminal.h"
#include "plotdata.h"
#include "qcustomplot.h"
#include "serialparser.h"
#include "serialworker.h"
#include "settings.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  // Zaregistruje typy aby je šlo posílat signály mezi vlákny
  qRegisterMetaType<BinDataSettings_t>();
  qRegisterMetaType<ChannelSettings_t>();
  qRegisterMetaType<PlotSettings_t>();

  MainWindow w;
  PlotData *plotData = new PlotData;
  SerialParser *serialParser = new SerialParser;
  SerialWorker *serialWorker = new SerialWorker;
  QThread plotDataThread;
  QThread serialParserThread;
  QThread serialWorkerThread;

  // Data pro graf -> GUI
  QObject::connect(plotData, &PlotData::dataReady, &w, &MainWindow::addDataToPlot);

  // GUI -> Zpracování příkazů
  QObject::connect(serialParser, &SerialParser::changedMode, &w, &MainWindow::changedDataMode);
  QObject::connect(serialParser, &SerialParser::printMessage, &w, &MainWindow::printMessage);
  QObject::connect(serialParser, &SerialParser::newProcessedLine, &w, &MainWindow::showProcessedCommand);
  QObject::connect(serialParser, &SerialParser::changedBinSettings, &w, &MainWindow::changeBinSettings);
  QObject::connect(serialParser, &SerialParser::printToTerminal, &w, &MainWindow::printToTerminal);

  // Zpracování příkazů -> Data pro graf
  QObject::connect(serialParser, &SerialParser::newDataString, plotData, &PlotData::newDataString);
  QObject::connect(serialParser, &SerialParser::newDataBin, plotData, &PlotData::newDataBin);

  QObject::connect(&w, &MainWindow::setMode, serialParser, &SerialParser::setMode);
  QObject::connect(&w, &MainWindow::setBinBits, serialParser, &SerialParser::setBinBits);
  QObject::connect(&w, &MainWindow::setBinCont, serialParser, &SerialParser::setBinCont);
  QObject::connect(&w, &MainWindow::setBinFCh, serialParser, &SerialParser::setBinFCh);
  QObject::connect(&w, &MainWindow::setBinMax, serialParser, &SerialParser::setBinMax);
  QObject::connect(&w, &MainWindow::setBinMin, serialParser, &SerialParser::setBinMin);
  QObject::connect(&w, &MainWindow::setBinNCh, serialParser, &SerialParser::setBinNCh);
  QObject::connect(&w, &MainWindow::setBinStep, serialParser, &SerialParser::setBinStep);

  QObject::connect(serialWorker, &SerialWorker::finishedWriting, &w, &MainWindow::serialFinishedWriting);
  QObject::connect(serialWorker, &SerialWorker::connectionResult, &w, &MainWindow::serialConnectResult);
  QObject::connect(serialWorker, &SerialWorker::bufferDebug, &w, &MainWindow::bufferDebug);
  QObject::connect(&w, &MainWindow::requestBufferDebug, serialWorker, &SerialWorker::requestedBufferDebug);
  QObject::connect(&w, &MainWindow::writeToSerial, serialWorker, &SerialWorker::write);
  QObject::connect(serialWorker, &SerialWorker::newLine, &w, &MainWindow::showProcessedCommand);
  QObject::connect(serialWorker, &SerialWorker::newLine, serialParser, &SerialParser::parseLine);
  QObject::connect(&w, &MainWindow::connectSerial, serialWorker, &SerialWorker::begin);
  QObject::connect(&w, &MainWindow::disconnectSerial, serialWorker, &SerialWorker::end);
  QObject::connect(&w, &MainWindow::changeLineTimeout, serialWorker, &SerialWorker::changeLineTimeout);
  QObject::connect(&serialWorkerThread, &QThread::started, serialWorker, &SerialWorker::init);

  serialWorker->moveToThread(&serialWorkerThread);
  serialWorkerThread.start();
  serialParser->moveToThread(&serialParserThread);
  serialParserThread.start();
  // plotData->moveToThread(plotDataThread);
  // plotDataThread->start();

  w.init();
  w.show();

  int returnValue = a.exec();

  serialWorker->deleteLater();
  delete plotData;
  serialParser->deleteLater();

  serialWorkerThread.exit();
  serialWorkerThread.wait();
  serialParserThread.exit();
  serialParserThread.wait();

  return returnValue;
}
