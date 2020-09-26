#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include "settings.h"
#include <QByteArrayMatcher>
#include <QObject>
#include <QSerialPort>
#include <QTimer>

class SerialWorker : public QObject {
  Q_OBJECT
public:
  explicit SerialWorker(QObject *parent = nullptr);
  ~SerialWorker();
  void init();

signals:
  void connectionResult(bool, QString);
  void newLine(QByteArray);
  void finishedWriting();

private:
  QTimer *timer;
  int lineTimeout = 100;
  QSerialPort *serial;
  QByteArray *buffer;

private slots:
  void read();
  void lineTimedOut();
  void errorOccured();

public slots:
  void changeLineTimeout(int value) { lineTimeout = value; }
  void begin(QString portName, int baudRate);
  void end();
  void write(QByteArray data);
};

#endif // SERIALWORKER_H
