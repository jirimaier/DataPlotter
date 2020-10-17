// Autor: Jiří Maier
// vytvořeno: 07/2020

// Qt 5.15

// Pro Window XP nutno použít Qt 5.7
// V Qt 5.7 nefungují adaptivní spinboxy

#include <QApplication>
#include <QSerialPort>
#include <QTimer>
#include <QTranslator>

#include "enums_defines_constants.h"
#include "mainwindow.h"
#include "myterminal.h"
#include "outputworker.h"
#include "plotdata.h"
#include "plotmath.h"
#include "qcustomplot.h"
#include "serialparser.h"
#include "serialworker.h"

int main(int argc, char *argv[]) {
  QApplication application(argc, argv);

  qDebug() << "Main thread is " << QThread::currentThreadId();

  // Zaregistruje typy aby je šlo posílat signály mezi vlákny
  qRegisterMetaType<BinDataSettings_t>();
  qRegisterMetaType<ChannelSettings_t>();
  qRegisterMetaType<PlotSettings_t>();
  qRegisterMetaType<QPair<QVector<double>, QVector<double>>>();

  MainWindow mainWindow;
  QTranslator translator;
  PlotData *plotData = new PlotData;
  SerialParser *serialParser = new SerialParser;
  SerialWorker *serialWorker = new SerialWorker;
  OutputWorker *outputWorker = new OutputWorker;
  PlotMath *plotMath = new PlotMath;

  QThread plotDataThread;
  QThread serialParserThread;
  QThread serialWorkerThread;
  QThread outputWorkerThread;
  QThread plotMathThread;

  QObject::connect(outputWorker, &OutputWorker::output, &mainWindow, &MainWindow::showProcessedCommand);
  QObject::connect(&mainWindow, &MainWindow::allowModeChange, serialParser, &SerialParser::allowModeChange);
  QObject::connect(&mainWindow, &MainWindow::setMode, serialParser, &SerialParser::setMode);
  QObject::connect(&mainWindow, &MainWindow::setBinParameters, serialParser, &SerialParser::setBinParameters);
  QObject::connect(&mainWindow, &MainWindow::connectSerial, serialWorker, &SerialWorker::begin);
  QObject::connect(&mainWindow, &MainWindow::disconnectSerial, serialWorker, &SerialWorker::end);
  QObject::connect(&mainWindow, &MainWindow::changeLineTimeout, serialWorker, &SerialWorker::changeLineTimeout);
  QObject::connect(&mainWindow, &MainWindow::resetChannels, plotData, &PlotData::reset);
  QObject::connect(&mainWindow, &MainWindow::requestBufferDebug, serialWorker, &SerialWorker::requestedBufferDebug);
  QObject::connect(&mainWindow, &MainWindow::writeToSerial, serialWorker, &SerialWorker::write);
  QObject::connect(&mainWindow, &MainWindow::setOutputLevel, outputWorker, &OutputWorker::setLevel);
  QObject::connect(&mainWindow, &MainWindow::requestMath, plotMath, &PlotMath::doMath);
  QObject::connect(&mainWindow, &MainWindow::sendManaulInput, serialParser, &SerialParser::parseLine);
  QObject::connect(&mainWindow, &MainWindow::sendManaulInput, outputWorker, &OutputWorker::input);
  QObject::connect(&mainWindow, &MainWindow::parseError, outputWorker, &OutputWorker::input);
  QObject::connect(serialParser, &SerialParser::changedDataMode, &mainWindow, &MainWindow::changedDataMode);
  QObject::connect(serialParser, &SerialParser::printMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(serialParser, &SerialParser::newProcessedLine, &mainWindow, &MainWindow::showProcessedCommand);
  QObject::connect(serialParser, &SerialParser::changedBinSettings, &mainWindow, &MainWindow::changedBinSettings);
  QObject::connect(serialParser, &SerialParser::printToTerminal, &mainWindow, &MainWindow::printToTerminal);
  QObject::connect(serialParser, &SerialParser::newDataString, plotData, &PlotData::newDataString);
  QObject::connect(serialParser, &SerialParser::newDataBin, plotData, &PlotData::newDataBin);
  QObject::connect(serialParser, &SerialParser::parseError, outputWorker, &OutputWorker::input);
  QObject::connect(serialParser, &SerialParser::changeGUISettings, &mainWindow, &MainWindow::useSettings);
  QObject::connect(plotData, &PlotData::updatePlot, &mainWindow, &MainWindow::addDataToPlot);
  QObject::connect(plotMath, &PlotMath::sendResult, &mainWindow, &MainWindow::addDataToPlot);
  QObject::connect(serialWorker, &SerialWorker::finishedWriting, &mainWindow, &MainWindow::serialFinishedWriting);
  QObject::connect(serialWorker, &SerialWorker::connectionResult, &mainWindow, &MainWindow::serialConnectResult);
  QObject::connect(serialWorker, &SerialWorker::bufferDebug, &mainWindow, &MainWindow::bufferDebug);
  QObject::connect(serialWorker, &SerialWorker::newLine, serialParser, &SerialParser::parseLine);
  QObject::connect(serialWorker, &SerialWorker::newLine, outputWorker, &OutputWorker::input);
  QObject::connect(&serialWorkerThread, &QThread::started, serialWorker, &SerialWorker::init);
  QObject::connect(&serialParserThread, &QThread::started, serialParser, &SerialParser::init);
  QObject::connect(&plotDataThread, &QThread::started, plotData, &PlotData::init);

  serialWorker->moveToThread(&serialWorkerThread);
  serialParser->moveToThread(&serialParserThread);
  plotData->moveToThread(&plotDataThread);
  outputWorker->moveToThread(&outputWorkerThread);
  plotMath->moveToThread(&plotMathThread);

  serialWorkerThread.start();
  serialParserThread.start();
  plotDataThread.start();
  outputWorkerThread.start();
  plotMathThread.start();

  mainWindow.init(&translator);
  mainWindow.show();

  int returnValue = application.exec();

  serialWorker->deleteLater();
  plotData->deleteLater();
  serialParser->deleteLater();
  outputWorker->deleteLater();
  plotMath->deleteLater();

  serialWorkerThread.quit();
  serialParserThread.quit();
  plotDataThread.quit();
  outputWorkerThread.quit();
  plotMathThread.quit();

  serialWorkerThread.wait();
  serialParserThread.wait();
  plotDataThread.wait();
  outputWorkerThread.wait();
  plotMathThread.wait();

  return returnValue;
}
