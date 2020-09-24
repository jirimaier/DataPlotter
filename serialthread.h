#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include "settings.h"
#include <QApplication>
#include <QByteArrayMatcher>
#include <QMutex>
#include <QObject>
#include <QSerialPort>
#include <QThread>

class SerialReader : public QThread {
  Q_OBJECT
public:
  SerialReader();
  ~SerialReader();

signals:
  void error(QString);
  void connectionResult(bool, QString);
  void newCommand(QByteArray);
  void newData(QByteArray);
  void finishedWriting();

private:
  QMutex mutex;
  void run() override;
  QString port;
  QByteArray writeBuffer;
  int baud;
  int lineTimeout = 5;

public slots:
  void begin(QString portName, int baudRate);
  void end();
  void write(QByteArray data);
};

#endif // SERIALTHREAD_H
