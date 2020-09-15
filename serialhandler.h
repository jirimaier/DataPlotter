#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

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
  bool connectSerial(int portIndex, long baud);
  QStringList refresh();
  QString currentPort() { return serial->portName(); }
  unsigned int currentBaud() { return serial->baudRate(); }

private:
  Settings *settings;
  QSerialPort *serial;
  QStringList portList;
  Buffer *buffer;
  void parseCommand(QByteArray);
  void parseData(QByteArray);
  void parseBinaryDataHeader(QByteArray data);

public slots:
  void write(QByteArray data);
  void disconnectSerial();

private slots:
  void readData();
  void readBuffer();
  void serialErrorOccurred();

signals:
  void serialErrorOccurredSignal();
  void changedMode(int);
  void showErrorMessage(QByteArray message);
  void printToTerminal(QByteArray message);
  void printMessage(QByteArray message, bool urgent);
  void newDataString(QByteArray data);
  void newDataBin(QByteArray data);
  void newProcessedCommand(QPair<bool, QByteArray>);
  void changedBinSettings(Settings::binDataSettings_t);
  void changedDataMode(int mode);
};

#endif // SERIALHANDLER_H
