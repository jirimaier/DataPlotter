#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "buffer.h"
#include "enums.h"
#include "myterminal.h"
#include "plotting.h"

class SerialHandler : public QObject {
  Q_OBJECT
public:
  SerialHandler();
  ~SerialHandler();
  bool connectSerial(int portIndex, long baud);
  QStringList refresh();
  QString currentPort() { return serial->portName(); }
  unsigned int currentBaud() { return serial->baudRate(); }
  int currentMode() { return mode; }

private:
  int mode = DATA_MODE_DATA_STRING;
  QSerialPort *serial;
  QStringList portList;
  Buffer *buffer;
  void parseCommand(QByteArray);
  void parseData(QByteArray);
  void parseBinaryDataHeader(QByteArray data);
  int bits = 8;
  double valueMin = 0;
  double valueMax = 255;
  double timeStep = 1;
  int numCh = 1;
  int firstCh = 1;
public slots:
  void write(QByteArray data);
  void disconnectSerial();
  void changeMode(int mode) { this->mode = mode; }
  void setBits(int value) { this->bits = value; }
  void setNumCh(int value) { this->numCh = value; }
  void setFirstCh(int value) { this->firstCh = value; }
  void setValueMin(double value) { this->valueMin = value; }
  void setValueMax(double value) { this->valueMax = value; }
  void setTimeStep(double value) { this->timeStep = value; }
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
  void newProcessedCommand(QPair<bool, QByteArray>);
  void changedBitMode(int bits, double valMin, double valMax, double timeStep, int numCh, int firstCh);
  void newDataBin(QByteArray data, int bits, double valMin, double valMax, double timeStep, int numCh, int firstCh);
};

#endif // SERIALHANDLER_H
