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

#include "global.h"
#include "mainwindow.h"
#include "newserialparser.h"
#include "plotdata.h"
#include "plotmath.h"
#include "serialreader.h"
#include "signalprocessing.h"
#include "interpolator.h"
#include "averager.h"
#include "xymode.h"

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
Q_DECLARE_METATYPE(HAxisType::enumHAxisType);
Q_DECLARE_METATYPE(Cursors::enumCursors);
Q_DECLARE_METATYPE(ValueType);
Q_DECLARE_METATYPE(QCPRange);
Q_DECLARE_METATYPE(QSerialPort::DataBits);
Q_DECLARE_METATYPE(QSerialPort::StopBits);
Q_DECLARE_METATYPE(QSerialPort::Parity);
Q_DECLARE_METATYPE(QSerialPort::FlowControl);

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
  qRegisterMetaType<HAxisType::enumHAxisType>();
  qRegisterMetaType<Cursors::enumCursors>();
  qRegisterMetaType<QPair<ValueType, QByteArray>>();
  qRegisterMetaType<QList<QPair<ValueType, QByteArray>>>();
  qRegisterMetaType<QCPRange>();
  qRegisterMetaType<QSerialPort::DataBits>();
  qRegisterMetaType<QSerialPort::StopBits>();
  qRegisterMetaType<QSerialPort::Parity>();
  qRegisterMetaType<QSerialPort::FlowControl>();

  // Vytvoří instance hlavních objektů
  MainWindow mainWindow;
  QTranslator translator; // Musí být zde aby dokázal přeložit i texty v objektech jiných než MainWindow
  PlotData* plotData = new PlotData();
  NewSerialParser* serialParser = new NewSerialParser(MessageTarget::serial1);
  NewSerialParser* serialParserM = new NewSerialParser(MessageTarget::manual);
  SerialReader* serial1 = new SerialReader();
  PlotMath* plotMath = new PlotMath();
  XYMode* xyMode = new XYMode();
  SignalProcessing* signalProcessing1 = new SignalProcessing();
  SignalProcessing* signalProcessing2 = new SignalProcessing();
  SignalProcessing* signalProcessingFFT1 = new SignalProcessing();
  SignalProcessing* signalProcessingFFT2 = new SignalProcessing();
  Interpolator* interpolator = new Interpolator();
  Averager* averager = new Averager();

  // Vytvoří vlákna
  //QThread plotDataThread;
  QThread serialParserThread;
  QThread plotMathThread;
  QThread serialReaderThread;
  QThread signalProcessing1Thread, signalProcessing2Thread, signalProcessingFFT1Thread, signalProcessingFFT2Thread;
  QThread interpolatorThread;
  QThread averagerThread;
  QThread xyThread;

  // Propojí signály
  QObject::connect(serial1, &SerialReader::sendData, serialParser, &NewSerialParser::parse);
  QObject::connect(serial1, &SerialReader::started, serialParser, &NewSerialParser::getReady);
  QObject::connect(serialParser, &NewSerialParser::ready, serial1, &SerialReader::parserReady);
  QObject::connect(serial1, &SerialReader::connectionResult, &mainWindow, &MainWindow::serialConnectResult);
  QObject::connect(serialParser, &NewSerialParser::sendMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(serialParser, &NewSerialParser::sendDeviceMessage, &mainWindow, &MainWindow::printDeviceMessage);
  QObject::connect(serialParser, &NewSerialParser::sendSettings, &mainWindow, &MainWindow::useSettings);
  QObject::connect(serialParser, &NewSerialParser::sendFileRequest, &mainWindow, &MainWindow::fileRequest);
  QObject::connect(serialParser, &NewSerialParser::deviceError, &mainWindow, &MainWindow::deviceError);
  QObject::connect(serialParser, &NewSerialParser::sendTerminal, &mainWindow, &MainWindow::printToTerminal);
  QObject::connect(serialParser, &NewSerialParser::sendPoint, plotData, &PlotData::addPoint);
  QObject::connect(serialParser, &NewSerialParser::sendLogicPoint, plotData, &PlotData::addLogicPoint);
  QObject::connect(serialParser, &NewSerialParser::sendChannel, plotData, &PlotData::addChannel);
  QObject::connect(serialParser, &NewSerialParser::sendLogicChannel, plotData, &PlotData::addLogicChannel);
  QObject::connect(serialParser, &NewSerialParser::sendEcho, serial1, &SerialReader::write);
  QObject::connect(&mainWindow, &MainWindow::requestSerialBufferClear, serialParser, &NewSerialParser::clearBuffer);
  QObject::connect(&mainWindow, &MainWindow::requestSerialBufferShow, serialParser, &NewSerialParser::showBuffer);
  QObject::connect(&mainWindow, &MainWindow::setSerialMessageLevel, serialParser, &NewSerialParser::setMsgLevel);
  QObject::connect(&mainWindow, &MainWindow::setSerialMessageLevel, plotData, &PlotData::setDebugLevel);
  QObject::connect(&mainWindow, &MainWindow::enableSerialMonitor, serial1, &SerialReader::enableMonitoring);
  QObject::connect(serial1, &SerialReader::monitor, &mainWindow, &MainWindow::printSerialMonitor);
  QObject::connect(serialParserM, &NewSerialParser::sendMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(serialParserM, &NewSerialParser::sendDeviceMessage, &mainWindow, &MainWindow::printDeviceMessage);
  QObject::connect(serialParserM, &NewSerialParser::sendSettings, &mainWindow, &MainWindow::useSettings);
  QObject::connect(serialParserM, &NewSerialParser::deviceError, &mainWindow, &MainWindow::deviceError);
  QObject::connect(serialParserM, &NewSerialParser::sendTerminal, &mainWindow, &MainWindow::printToTerminal);
  QObject::connect(serialParserM, &NewSerialParser::sendPoint, plotData, &PlotData::addPoint);
  QObject::connect(serialParserM, &NewSerialParser::sendLogicPoint, plotData, &PlotData::addLogicPoint);
  QObject::connect(serialParserM, &NewSerialParser::sendChannel, plotData, &PlotData::addChannel);
  QObject::connect(serialParserM, &NewSerialParser::sendLogicChannel, plotData, &PlotData::addLogicChannel);
  QObject::connect(serialParserM, &NewSerialParser::sendFileRequest, &mainWindow, &MainWindow::fileRequest);
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
  QObject::connect(plotData, &PlotData::setExpectedRange, &mainWindow, &MainWindow::setExpectedRange);
  QObject::connect(plotMath, &PlotMath::sendMessage, &mainWindow, &MainWindow::printMessage);
  QObject::connect(&mainWindow, &MainWindow::setChDigital, plotData, &PlotData::setDigitalChannel);
  QObject::connect(&mainWindow, &MainWindow::setLogicBits, plotData, &PlotData::setLogicBits);
  QObject::connect(&mainWindow, &MainWindow::resetMath, plotMath, &PlotMath::resetMath);
  QObject::connect(&mainWindow, &MainWindow::requestXY, xyMode, &XYMode::calculateXY);
  QObject::connect(plotData, &PlotData::addMathData, plotMath, &PlotMath::addMathData);
  QObject::connect(&mainWindow, &MainWindow::setMathFirst, plotData, &PlotData::setMathFirst);
  QObject::connect(&mainWindow, &MainWindow::setMathSecond, plotData, &PlotData::setMathSecond);
  QObject::connect(&mainWindow, &MainWindow::clearMath, plotMath, &PlotMath::clearMath);
  QObject::connect(&mainWindow, &MainWindow::requstMeasurements1, signalProcessing1, &SignalProcessing::process);
  QObject::connect(&mainWindow, &MainWindow::requstMeasurements2, signalProcessing2, &SignalProcessing::process);
  QObject::connect(&mainWindow, &MainWindow::requestFFT1, signalProcessingFFT1, &SignalProcessing::getFFTPlot);
  QObject::connect(&mainWindow, &MainWindow::requestFFT2, signalProcessingFFT2, &SignalProcessing::getFFTPlot);
  QObject::connect(signalProcessing1, &SignalProcessing::result, &mainWindow, &MainWindow::signalMeasurementsResult1);
  QObject::connect(signalProcessing2, &SignalProcessing::result, &mainWindow, &MainWindow::signalMeasurementsResult2);
  QObject::connect(signalProcessingFFT1, &SignalProcessing::fftResult, &mainWindow, &MainWindow::fftResult1);
  QObject::connect(signalProcessingFFT2, &SignalProcessing::fftResult, &mainWindow, &MainWindow::fftResult2);
  QObject::connect(xyMode, &XYMode::sendResultXY, &mainWindow, &MainWindow::xyResult);
  QObject::connect(&mainWindow, &MainWindow::interpolate, interpolator, &Interpolator::interpolate);
  QObject::connect(interpolator, &Interpolator::interpolationResult, &mainWindow, &MainWindow::interpolationResult);
  QObject::connect(&mainWindow, &MainWindow::setAverager, plotData, &PlotData::setAverager);
  QObject::connect(&mainWindow, &MainWindow::resetAverager, averager, &Averager::reset);
  QObject::connect(&mainWindow, &MainWindow::setAveragerCount, averager, &Averager::setCount);
  QObject::connect(plotData, &PlotData::addDataToAverager, averager, &Averager::newDataVector);
  QObject::connect(plotData, &PlotData::addPointToAverager, averager, &Averager::newDataPoint);
  QObject::connect(&mainWindow, &MainWindow::setInterpolationFilter, interpolator, &Interpolator::loadFilterFromFile);
  QObject::connect(&mainWindow, &MainWindow::replyEcho, serialParser, &NewSerialParser::replyEcho);
  QObject::connect(&mainWindow, &MainWindow::changeSerialBaud, serial1, &SerialReader::changeBaud);

  // Funkce init je zavolána až z nového vlákna
  QObject::connect(&serialReaderThread, &QThread::started, serial1, &SerialReader::init);

  // Přesuny do vláken
  serial1->moveToThread(&serialReaderThread);
  serialParser->moveToThread(&serialParserThread);
  serialParserM->moveToThread(&serialParserThread);
  plotData->moveToThread(&serialParserThread);
  plotMath->moveToThread(&plotMathThread);

  xyMode->moveToThread(&xyThread);
  signalProcessing1->moveToThread(&signalProcessing1Thread);
  signalProcessing2->moveToThread(&signalProcessing2Thread);
  signalProcessingFFT1->moveToThread(&signalProcessingFFT1Thread);
  signalProcessingFFT2->moveToThread(&signalProcessingFFT2Thread);
  interpolator->moveToThread(&interpolatorThread);
  averager->moveToThread(&averagerThread);

  // Zahájí vlákna
  serialReaderThread.start();
  serialParserThread.start();
  plotMathThread.start();
  signalProcessing1Thread.start();
  signalProcessing2Thread.start();
  signalProcessingFFT1Thread.start();
  signalProcessingFFT2Thread.start();
  interpolatorThread.start();
  averagerThread.start();
  xyThread.start();

  // Zobrazí okno a čeká na ukončení
  mainWindow.init(&translator, plotData, plotMath, serial1, averager);
  mainWindow.show();
  int returnValue = application.exec();

  // Smazat objekty až budou dokončeny procesy v nich
  serialParser->deleteLater();
  serialParserM->deleteLater();
  plotData->deleteLater();
  plotMath->deleteLater();
  serial1->deleteLater();
  signalProcessing1->deleteLater();
  signalProcessing2->deleteLater();
  signalProcessingFFT1->deleteLater();
  signalProcessingFFT2->deleteLater();
  interpolator->deleteLater();
  averager->deleteLater();
  xyMode->deleteLater();

  // Vyžádá ukončení event loopu
  serialParserThread.quit();
  plotMathThread.quit();
  serialReaderThread.quit();
  signalProcessing1Thread.quit();
  signalProcessing2Thread.quit();
  signalProcessingFFT1Thread.quit();
  signalProcessingFFT2Thread.quit();
  interpolatorThread.quit();
  averagerThread.quit();
  xyThread.quit();

  // Čeká na ukončení procesů
  serialParserThread.wait();
  plotMathThread.wait();
  serialReaderThread.wait();
  signalProcessing1Thread.wait();
  signalProcessing2Thread.wait();
  signalProcessingFFT1Thread.wait();
  signalProcessingFFT2Thread.wait();
  interpolatorThread.wait();
  averagerThread.wait();
  xyThread.wait();

  return returnValue;
}
