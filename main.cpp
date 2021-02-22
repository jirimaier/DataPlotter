//  Copyright (C) 2020-2021  Jiří Maier

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

//  Tento program je svobodný software: můžete jej šířit a upravovat
//  podle ustanovení Obecné veřejné licence GNU (GNU General Public Licence),
//  vydávané Free Software Foundation a to buď podle 3. verze této Licence,
//  nebo (podle vašeho uvážení) kterékoli pozdější verze.

//  Tento program je rozšiřován v naději, že bude užitečný,
//  avšak BEZ JAKÉKOLIV ZÁRUKY. Neposkytují se ani odvozené záruky PRODEJNOSTI
//  anebo VHODNOSTI PRO URČITÝ ÚČEL. Další podrobnosti hledejte v Obecné veřejné licenci GNU.

//  Kopii Obecné veřejné licence GNU jste měli obdržet spolu s tímto programem.
//  Pokud se tak nestalo, najdete ji zde: <http://www.gnu.org/licenses/>.

//  Contact
//  Jiri Maier
//  Faculty of electrical engineering
//  Czech technical university
//  Prague
//  maierjir@fel.cvut.cz
//  jiri.maier.x@gmail.com

//  Kontakt
//  Jiří Maier
//  Fakulta elektrotechnická
//  České vysoké učení technické
//  Praha
//  maierjir@fel.cvut.cz
//  jiri.maier.x@gmail.com

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
#include "signalprocessing.h"

Q_DECLARE_METATYPE(ChannelSettings_t)
Q_DECLARE_METATYPE(DataMode::enumDataMode)
Q_DECLARE_METATYPE(OutputLevel::enumOutputLevel)
Q_DECLARE_METATYPE(MessageLevel::enumMessageLevel)
Q_DECLARE_METATYPE(PlotStatus::enumPlotStatus)
Q_DECLARE_METATYPE(MessageTarget::enumMessageTarget)
Q_DECLARE_METATYPE(QSharedPointer<QVector<double>>);
Q_DECLARE_METATYPE(QSharedPointer<QCPGraphDataContainer>);
Q_DECLARE_METATYPE(QSharedPointer<QCPCurveDataContainer>);
Q_DECLARE_METATYPE(MathOperations::enumMathOperations);
Q_DECLARE_METATYPE(FFTWindow::enumFFTWindow);
Q_DECLARE_METATYPE(FFTType::enumFFTType);
Q_DECLARE_METATYPE(ValueType);

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

  // Zaregistruje typy aby je šlo posílat signály mezi vlákny
  qRegisterMetaType<ChannelSettings_t>();
  qRegisterMetaType<QPair<QVector<double>, QVector<double>>>();
  qRegisterMetaType<DataMode::enumDataMode>();
  qRegisterMetaType<OutputLevel::enumOutputLevel>();
  qRegisterMetaType<MessageLevel::enumMessageLevel>();
  qRegisterMetaType<PlotStatus::enumPlotStatus>();
  qRegisterMetaType<MessageTarget::enumMessageTarget>();
  qRegisterMetaType<QSharedPointer<QVector<double>>>();
  qRegisterMetaType<QSharedPointer<QCPGraphDataContainer>>();
  qRegisterMetaType<QSharedPointer<QCPCurveDataContainer>>();
  qRegisterMetaType<MathOperations::enumMathOperations>();
  qRegisterMetaType<FFTWindow::enumFFTWindow>();
  qRegisterMetaType<FFTType::enumFFTType>();
  qRegisterMetaType<QPair<ValueType, QByteArray>>();
  qRegisterMetaType<QList<QPair<ValueType, QByteArray>>>();

  // Vytvoří instance hlavních objektů
  MainWindow mainWindow;
  QTranslator translator; // Musí být zde aby dokázal přeložit i texty v objektech jiných než MainWindow
  PlotData* plotData = new PlotData();
  NewSerialParser* serialParser1 = new NewSerialParser(MessageTarget::serial1);
  NewSerialParser* serialParserM = new NewSerialParser(MessageTarget::manual);
  SerialReader* serial1 = new SerialReader();
  PlotMath* plotMath = new PlotMath();
  SignalProcessing* signalProcessing1 = new SignalProcessing();
  SignalProcessing* signalProcessing2 = new SignalProcessing();
  SignalProcessing* signalProcessingFFT = new SignalProcessing();

  // Vytvoří vlákna
  QThread plotDataThread;
  QThread serialParser1Thread;
  QThread serialParserMThread;
  QThread plotMathThread;
  QThread serialReader1Thread;
  QThread signalProcessing1Thread, signalProcessing2Thread, signalProcessingFFTThread;

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
  QObject::connect(serialParser1, &NewSerialParser::sendEcho, serial1, &SerialReader::write);
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
  QObject::connect(&mainWindow, &MainWindow::sendManualInput, serialParserM, &NewSerialParser::parse);
  QObject::connect(plotData, &PlotData::sendMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(plotData, &PlotData::ch1dataUpdated, &mainWindow, &MainWindow::ch1WasUpdated);
  QObject::connect(plotMath, &PlotMath::sendMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(&mainWindow, &MainWindow::setChDigital, plotData, &PlotData::setDigitalChannel);
  QObject::connect(&mainWindow, &MainWindow::setLogicBits, plotData, &PlotData::setLogicBits);
  QObject::connect(&mainWindow, &MainWindow::resetMath, plotMath, &PlotMath::resetMath);
  QObject::connect(&mainWindow, &MainWindow::requestXY, plotMath, &PlotMath::calculateXY);
  QObject::connect(plotData, &PlotData::addMathData, plotMath, &PlotMath::addMathData);
  QObject::connect(&mainWindow, &MainWindow::setMathFirst, plotData, &PlotData::setMathFirst);
  QObject::connect(&mainWindow, &MainWindow::setMathSecond, plotData, &PlotData::setMathSecond);
  QObject::connect(&mainWindow, &MainWindow::clearMath, plotMath, &PlotMath::clearMath);
  QObject::connect(&mainWindow, &MainWindow::requstMeasurements1, signalProcessing1, &SignalProcessing::process);
  QObject::connect(&mainWindow, &MainWindow::requstMeasurements2, signalProcessing2, &SignalProcessing::process);
  QObject::connect(&mainWindow, &MainWindow::requestFFT, signalProcessingFFT, &SignalProcessing::calculateSpectrum);
  QObject::connect(signalProcessing1, &SignalProcessing::result, &mainWindow, &MainWindow::signalMeasurementsResult1);
  QObject::connect(signalProcessing2, &SignalProcessing::result, &mainWindow, &MainWindow::signalMeasurementsResult2);
  QObject::connect(signalProcessingFFT, &SignalProcessing::fftResult, &mainWindow, &MainWindow::fftResult);
  QObject::connect(plotMath, &PlotMath::sendResultXY, &mainWindow, &MainWindow::xyResult);

  // Funkce init je zavolána až z nového vlákna
  QObject::connect(&serialReader1Thread, &QThread::started, serial1, &SerialReader::init);

  // Přesuny do vláken
  serial1->moveToThread(&serialReader1Thread);
  serialParser1->moveToThread(&serialParser1Thread);
  serialParserM->moveToThread(&serialParserMThread);
  plotData->moveToThread(&plotDataThread);
  plotMath->moveToThread(&plotMathThread);
  signalProcessing1->moveToThread(&signalProcessing1Thread);
  signalProcessing2->moveToThread(&signalProcessing2Thread);
  signalProcessingFFT->moveToThread(&signalProcessingFFTThread);

  // Zahájí vlákna
  serialReader1Thread.start();
  serialParser1Thread.start();
  serialParserMThread.start();
  plotDataThread.start();
  plotMathThread.start();
  signalProcessing1Thread.start();
  signalProcessing2Thread.start();
  signalProcessingFFTThread.start();

  // Zobrazí okno a čeká na ukončení
  mainWindow.init(&translator, plotData, plotMath, serial1);
  mainWindow.show();
  int returnValue = application.exec();

  // Smazat objekty až budou dokončeny procesy v nich
  serialParser1->deleteLater();
  serialParserM->deleteLater();
  plotData->deleteLater();
  plotMath->deleteLater();
  serial1->deleteLater();
  signalProcessing1->deleteLater();
  signalProcessing2->deleteLater();
  signalProcessingFFT->deleteLater();

  // Vyžádá ukončení event loopu
  serialParser1Thread.quit();
  serialParserMThread.quit();
  plotDataThread.quit();
  plotMathThread.quit();
  serialReader1Thread.quit();
  signalProcessing1Thread.quit();
  signalProcessing2Thread.quit();
  signalProcessingFFTThread.quit();

  // Čeká na ukončení procesů
  serialParser1Thread.wait();
  serialParserMThread.wait();
  plotDataThread.wait();
  plotMathThread.wait();
  serialReader1Thread.wait();
  signalProcessing1Thread.wait();
  signalProcessing2Thread.wait();
  signalProcessingFFTThread.wait();

  return returnValue;
}
