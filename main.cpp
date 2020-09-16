#include <QApplication>

#include "mainwindow.h"
#include "myterminal.h"
#include "plotdata.h"
#include "qcustomplot.h"
#include "serialhandler.h"
#include "settings.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  Settings settings;
  PlotData plotData(&settings);
  SerialHandler serialHandler(&settings);
  w.init(&settings);
  QObject::connect(&w, &MainWindow::connectSerial, &serialHandler, &SerialHandler::connectSerial);
  QObject::connect(&w, &MainWindow::disconnectSerial, &serialHandler, &SerialHandler::disconnectSerial);
  QObject::connect(&serialHandler, &SerialHandler::connectionResult, &w, &MainWindow::serialConnectResult);
  QObject::connect(&plotData, &PlotData::dataReady, &w, &MainWindow::addDataToPlot);
  // Serial -> MainWindow
  QObject::connect(&serialHandler, &SerialHandler::serialErrorOccurredSignal, &w, &MainWindow::serialErrorOccurred);
  QObject::connect(&serialHandler, &SerialHandler::changedMode, &w, &MainWindow::setDataMode);
  QObject::connect(&serialHandler, &SerialHandler::showErrorMessage, &w, &MainWindow::showErrorMessage);
  QObject::connect(&serialHandler, &SerialHandler::printMessage, &w, &MainWindow::printMessage);
  QObject::connect(&serialHandler, &SerialHandler::newProcessedCommand, &w, &MainWindow::showProcessedCommand);
  QObject::connect(&serialHandler, &SerialHandler::changedBinSettings, &w, &MainWindow::changeBinSettings);

  // Serial -> Plotting
  QObject::connect(&serialHandler, &SerialHandler::newDataString, &plotData, &PlotData::newDataString);
  QObject::connect(&serialHandler, &SerialHandler::newDataBin, &plotData, &PlotData::newDataBin);

  // Serial -> Terminal
  // connect(serialHandler, SIGNAL(printToTerminal(QByteArray)), ui->myTerminal, SLOT(printToTerminal(QByteArray)));
  w.show();
  return a.exec();
}
