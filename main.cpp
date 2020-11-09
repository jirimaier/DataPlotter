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
#include "newserialparser.h"
#include "plotdata.h"
#include "plotmath.h"
#include "serialreader.h"

int main(int argc, char *argv[]) {
  QApplication application(argc, argv);

  qDebug() << "Main thread is " << QThread::currentThreadId();

  // Zkontroluje jestli je systém little-endian
  uint16_t endianCheck = 0x0102;
  if ((uint8_t) * (&endianCheck) == 1)
    Global::platformIsBigEndian = true;
  else
    Global::platformIsBigEndian = false;
  qDebug() << "This platform is " << (Global::platformIsBigEndian ? "Big-endian" : "Little-endian");

  // Zaregistruje typy aby je šlo posílat signály mezi vlákny
  qRegisterMetaType<ChannelSettings_t>();
  qRegisterMetaType<PlotSettings_t>();
  qRegisterMetaType<PlotFrame_t>();
  qRegisterMetaType<QPair<QVector<double>, QVector<double>>>();
  qRegisterMetaType<DataMode::enumerator>();
  qRegisterMetaType<OutputLevel::enumerator>();
  qRegisterMetaType<MessageLevel::enumerator>();
  qRegisterMetaType<PlotStatus::enumerator>();
  qRegisterMetaType<MessageTarget::enumerator>();

  // Vytvoří instance hlavních objektů
  MainWindow mainWindow;
  QTranslator translator; // Musí bít zde aby dokázal přeložit i texty v objektech jiných než MainWindow
  PlotData *plotData = new PlotData;
  NewSerialParser *serialParser1 = new NewSerialParser(MessageTarget::serial1);
  NewSerialParser *serialParserM = new NewSerialParser(MessageTarget::manual);
  SerialReader *serial1 = new SerialReader;
  PlotMath *plotMath = new PlotMath;

  // Vytvoří vlákna
  QThread plotDataThread;
  QThread serialParser1Thread;
  QThread serialParserMThread;
  QThread plotMathThread;
  QThread serialReader1Thread;

  // Propojí signály
  QObject::connect(serial1, &SerialReader::sendData, serialParser1, &NewSerialParser::parse);
  QObject::connect(serial1, &SerialReader::started, serialParser1, &NewSerialParser::getReady);
  QObject::connect(serialParser1, &NewSerialParser::ready, serial1, &SerialReader::parserReady);

  QObject::connect(serial1, &SerialReader::finishedWriting, &mainWindow, &MainWindow::serialFinishedWriting);
  QObject::connect(serial1, &SerialReader::connectionResult, &mainWindow, &MainWindow::serialConnectResult);

  QObject::connect(serialParser1, &NewSerialParser::sendMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(serialParser1, &NewSerialParser::sendDeviceMessage, &mainWindow, &MainWindow::printDeviceMessage);
  QObject::connect(serialParser1, &NewSerialParser::sendSettings, &mainWindow, &MainWindow::useSettings);
  QObject::connect(serialParser1, &NewSerialParser::sendTerminal, &mainWindow, &MainWindow::printToTerminal);
  QObject::connect(serialParser1, &NewSerialParser::sendPoint, plotData, &PlotData::addPoint);
  QObject::connect(serialParser1, &NewSerialParser::sendChannel, plotData, &PlotData::addChannel);
  QObject::connect(&mainWindow, &MainWindow::requestSerialBufferClear, serialParser1, &NewSerialParser::clearBuffer);
  QObject::connect(&mainWindow, &MainWindow::requestSerialBufferShow, serialParser1, &NewSerialParser::showBuffer);
  QObject::connect(&mainWindow, &MainWindow::setSerialMessageLevel, serialParser1, &NewSerialParser::setMsgLevel);
  QObject::connect(&mainWindow, &MainWindow::setSerialMessageLevel, plotData, &PlotData::setDebugLevel);

  QObject::connect(&mainWindow, &MainWindow::enableSerialMonitor, serial1, &SerialReader::enableMonitoring);
  QObject::connect(serial1, &SerialReader::monitor, &mainWindow, &MainWindow::printSerialMonitor);

  QObject::connect(serialParserM, &NewSerialParser::sendMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(serialParserM, &NewSerialParser::sendDeviceMessage, &mainWindow, &MainWindow::printDeviceMessage);
  QObject::connect(serialParserM, &NewSerialParser::sendSettings, &mainWindow, &MainWindow::useSettings);
  QObject::connect(serialParserM, &NewSerialParser::sendTerminal, &mainWindow, &MainWindow::printToTerminal);
  QObject::connect(serialParserM, &NewSerialParser::sendPoint, plotData, &PlotData::addPoint);
  QObject::connect(serialParserM, &NewSerialParser::sendChannel, plotData, &PlotData::addChannel);
  QObject::connect(&mainWindow, &MainWindow::requestManualBufferClear, serialParserM, &NewSerialParser::clearBuffer);
  QObject::connect(&mainWindow, &MainWindow::requestManualBufferShow, serialParserM, &NewSerialParser::showBuffer);
  QObject::connect(&mainWindow, &MainWindow::setManualMessageLevel, serialParserM, &NewSerialParser::setMsgLevel);

  QObject::connect(&mainWindow, &MainWindow::toggleSerialConnection, serial1, &SerialReader::toggle);
  QObject::connect(&mainWindow, &MainWindow::disconnectSerial, serial1, &SerialReader::end);
  QObject::connect(&mainWindow, &MainWindow::resetChannels, plotData, &PlotData::reset);
  QObject::connect(&mainWindow, &MainWindow::writeToSerial, serial1, &SerialReader::write);
  QObject::connect(&mainWindow, &MainWindow::requestMath, plotMath, &PlotMath::doMath);
  QObject::connect(&mainWindow, &MainWindow::requestXY, plotMath, &PlotMath::doXY);
  QObject::connect(&mainWindow, &MainWindow::sendManualInput, serialParserM, &NewSerialParser::parse);

  QObject::connect(plotData, &PlotData::addVectorToPlot, &mainWindow, &MainWindow::addVectorToPlot);
  QObject::connect(plotData, &PlotData::addPointToPlot, &mainWindow, &MainWindow::addPointToPlot);
  QObject::connect(plotData, &PlotData::sendMessage, &mainWindow, &MainWindow::printMessage);

  QObject::connect(plotMath, &PlotMath::sendResult, &mainWindow, &MainWindow::addVectorToPlot);
  QObject::connect(plotMath, &PlotMath::sendResultXY, &mainWindow, &MainWindow::addDataToXY);

  // Funkce init jsou zavolány až z nového vlákna
  QObject::connect(&serialParser1Thread, &QThread::started, serialParser1, &NewSerialParser::init);
  QObject::connect(&serialParserMThread, &QThread::started, serialParser1, &NewSerialParser::init);
  QObject::connect(&plotDataThread, &QThread::started, plotData, &PlotData::init);
  QObject::connect(&serialReader1Thread, &QThread::started, serial1, &SerialReader::init);
  QObject::connect(&plotDataThread, &QThread::started, plotData, &PlotData::init);

  // Přesuny do vláken
  serial1->moveToThread(&serialReader1Thread);
  serialParser1->moveToThread(&serialParser1Thread);
  serialParserM->moveToThread(&serialParserMThread);
  plotData->moveToThread(&plotDataThread);
  plotMath->moveToThread(&plotMathThread);

  // Zahájí vlákna
  serialReader1Thread.start();
  serialParser1Thread.start();
  serialParserMThread.start();
  plotDataThread.start();
  plotMathThread.start();

  // Zobrazí okno a čeká na ukončení
  mainWindow.init(&translator);
  mainWindow.show();
  int returnValue = application.exec();

  // Smazat objekty až budou dokončeny procesy v nich
  serialParser1->deleteLater();
  plotData->deleteLater();
  plotMath->deleteLater();
  serial1->deleteLater();

  // Vyžádá ukončení event loopu
  serialParser1Thread.quit();
  serialParserMThread.quit();
  plotDataThread.quit();
  plotMathThread.quit();
  serialReader1Thread.quit();

  // Čeká na ukončení procesů
  serialParser1Thread.wait();
  serialParserMThread.wait();
  plotDataThread.wait();
  plotMathThread.wait();
  serialReader1Thread.wait();

  return returnValue;
}
