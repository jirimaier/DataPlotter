#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>

#include "buffer.h"
#include "enums.h"
#include "myterminal.h"
#include "plotdata.h"
#include "settings.h"

class SerialHandler : public QObject {
  Q_OBJECT
public:
  SerialHandler(Settings *in_settings);
  ~SerialHandler();

private:
  Settings *settings;
  QSerialPort *serial;
  Buffer *buffer;
  void parseCommand(QByteArray);
  void parseData(QByteArray);
  void parseBinaryDataHeader(QByteArray data);

public slots:
  void write(QByteArray data);
  void disconnectSerial();
  void connectSerial(QString portName, long baud);

private slots:
  void readData();
  void readBuffer();
  void serialErrorOccurred();

signals:
  void serialErrorOccurredSignal(QString error);
  void changedMode(int);
  void showErrorMessage(QByteArray message);
  void printToTerminal(QByteArray message);
  void printMessage(QByteArray message, bool urgent);
  void newDataString(QByteArray data);
  void newDataBin(QByteArray data);
  void newProcessedCommand(QPair<bool, QByteArray>);
  void changedBinSettings(Settings::binDataSettings_t);
  void changedDataMode(int mode);
  void connectionResult(bool, QString);
};

#endif // SERIALHANDLER_H
