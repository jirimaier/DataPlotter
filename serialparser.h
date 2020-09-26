#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>

#include "myterminal.h"
#include "plotdata.h"
#include "settings.h"

class SerialParser : public QObject {
  Q_OBJECT
public:
  SerialParser();
  ~SerialParser();

private:
  BinDataSettings_t binDataSettings;
  int dataMode = DATA_MODE_DATA_UNKNOWN;
  void parseBinaryDataHeader(QByteArray data);

public slots:
  void parseLine(QByteArray line);
  void setBinBits(int value) { binDataSettings.bits = value; }
  void setBinMin(double value) { binDataSettings.valueMin = value; }
  void setBinMax(double value) { binDataSettings.valueMax = value; }
  void setBinStep(double value) { binDataSettings.timeStep = value; }
  void setBinNCh(int value) { binDataSettings.numCh = value; }
  void setBinFCh(int value) { binDataSettings.firstCh = value; }
  void setBinCont(bool value) { binDataSettings.continuous = value; }
  void setMode(int mode) { dataMode = mode; }

signals:
  void changedMode(int);
  void printToTerminal(QByteArray message);
  void printMessage(QByteArray message, bool urgent);
  void newDataString(QByteArray data);
  void newDataBin(QByteArray data, BinDataSettings_t settings);
  void newProcessedLine(QByteArray);
  void changedBinSettings(BinDataSettings_t);
  void changedDataMode(int mode);
};

#endif // SERIALHANDLER_H
