#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include "settings.h"
#include <QByteArrayMatcher>
#include <QObject>
#include <QSerialPort>

class SerialWorker : public QObject {
  Q_OBJECT
public:
  explicit SerialWorker(QObject *parent = nullptr);
  ~SerialWorker();

signals:
  void error(QString);
  void connectionResult(bool, QString);
  void newCommand(QByteArray);
  void newData(QByteArray);
  void finishedWriting();

private:
  QSerialPort serial;
  QByteArray buffer;

private slots:
  void read();

public slots:
  void begin(QString portName, int baudRate);
  void end();
  void write(QByteArray data);
};

#endif // SERIALWORKER_H
