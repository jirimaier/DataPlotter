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
#include "plotmath.h"
#include "qcustomplot.h"
#include "receivedoutputhandler.h"
#include "serialparser.h"
#include "serialworker.h"
#include "settings.h"

int main(int argc, char *argv[]) {
  qDebug() << "Main thread is " << QThread::currentThreadId();
  QApplication application(argc, argv);

  // Zaregistruje typy aby je šlo posílat signály mezi vlákny
  qRegisterMetaType<BinDataSettings_t>();
  qRegisterMetaType<ChannelSettings_t>();
  qRegisterMetaType<PlotSettings_t>();
  qRegisterMetaType<QPair<QVector<double>, QVector<double>>>();

  MainWindow mainWindow;
  PlotData *plotData = new PlotData;
  SerialParser *serialParser = new SerialParser;
  SerialWorker *serialWorker = new SerialWorker;
  ReceivedOutputHandler *receivedOutputHandler = new ReceivedOutputHandler;
  PlotMath *plotMath = new PlotMath;

  QThread plotDataThread;
  QThread serialParserThread;
  QThread serialWorkerThread;
  QThread receivedOutputHandlerThread;
  QThread plotMathThread;

  // Data pro graf -> GUI
  QObject::connect(plotData, &PlotData::updatePlot, &mainWindow, &MainWindow::addDataToPlot);
  QObject::connect(plotMath, &PlotMath::sendResult, &mainWindow, &MainWindow::addDataToPlot);

  // GUI -> Zpracování příkazů
  QObject::connect(serialParser, &SerialParser::changedDataMode, &mainWindow, &MainWindow::changedDataMode);
  QObject::connect(serialParser, &SerialParser::printMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(serialParser, &SerialParser::newProcessedLine, &mainWindow, &MainWindow::showProcessedCommand);
  QObject::connect(serialParser, &SerialParser::changedBinSettings, &mainWindow, &MainWindow::changedBinSettings);
  QObject::connect(serialParser, &SerialParser::printToTerminal, &mainWindow, &MainWindow::printToTerminal);

  // Zpracování příkazů -> Data pro graf
  QObject::connect(serialParser, &SerialParser::newDataString, plotData, &PlotData::newDataString);
  QObject::connect(serialParser, &SerialParser::newDataBin, plotData, &PlotData::newDataBin);
  QObject::connect(&mainWindow, &MainWindow::allowModeChange, serialParser, &SerialParser::allowModeChange);
  QObject::connect(&mainWindow, &MainWindow::setMode, serialParser, &SerialParser::setMode);
  QObject::connect(&mainWindow, &MainWindow::setBinParameters, serialParser, &SerialParser::setBinParameters);
  QObject::connect(serialWorker, &SerialWorker::finishedWriting, &mainWindow, &MainWindow::serialFinishedWriting);
  QObject::connect(serialWorker, &SerialWorker::connectionResult, &mainWindow, &MainWindow::serialConnectResult);
  QObject::connect(serialWorker, &SerialWorker::bufferDebug, &mainWindow, &MainWindow::bufferDebug);
  QObject::connect(&mainWindow, &MainWindow::requestBufferDebug, serialWorker, &SerialWorker::requestedBufferDebug);
  QObject::connect(&mainWindow, &MainWindow::writeToSerial, serialWorker, &SerialWorker::write);

  QObject::connect(serialWorker, &SerialWorker::newLine, serialParser, &SerialParser::parseLine);
  QObject::connect(&mainWindow, &MainWindow::connectSerial, serialWorker, &SerialWorker::begin);
  QObject::connect(&mainWindow, &MainWindow::disconnectSerial, serialWorker, &SerialWorker::end);
  QObject::connect(&mainWindow, &MainWindow::changeLineTimeout, serialWorker, &SerialWorker::changeLineTimeout);

  QObject::connect(&mainWindow, &MainWindow::resetChannels, plotData, &PlotData::reset);

  QObject::connect(receivedOutputHandler, &ReceivedOutputHandler::output, &mainWindow, &MainWindow::showProcessedCommand);
  QObject::connect(serialWorker, &SerialWorker::newLine, receivedOutputHandler, &ReceivedOutputHandler::input);
  QObject::connect(&mainWindow, &MainWindow::setOutputLevel, receivedOutputHandler, &ReceivedOutputHandler::setLevel);

  QObject::connect(&mainWindow, &MainWindow::requestMath, plotMath, &PlotMath::doMath);

  QObject::connect(&serialWorkerThread, &QThread::started, serialWorker, &SerialWorker::init);
  QObject::connect(&serialParserThread, &QThread::started, serialParser, &SerialParser::init);
  QObject::connect(&plotDataThread, &QThread::started, plotData, &PlotData::init);

  serialWorker->moveToThread(&serialWorkerThread);
  serialParser->moveToThread(&serialParserThread);
  plotData->moveToThread(&plotDataThread);
  receivedOutputHandler->moveToThread(&receivedOutputHandlerThread);
  plotMath->moveToThread(&plotMathThread);

  serialWorkerThread.start();
  serialParserThread.start();
  plotDataThread.start();
  receivedOutputHandlerThread.start();
  plotMathThread.start();

  mainWindow.init();
  mainWindow.show();

  int returnValue = application.exec();

  serialWorker->deleteLater();
  plotData->deleteLater();
  serialParser->deleteLater();
  receivedOutputHandler->deleteLater();
  plotMath->deleteLater();

  serialWorkerThread.quit();
  serialParserThread.quit();
  plotDataThread.quit();
  receivedOutputHandlerThread.quit();
  plotMathThread.quit();

  serialWorkerThread.wait();
  serialParserThread.wait();
  plotDataThread.wait();
  receivedOutputHandlerThread.wait();
  plotMathThread.wait();

  return returnValue;
}
