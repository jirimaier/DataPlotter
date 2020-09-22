#ifndef BUFFER_H
#define BUFFER_H

#include <QByteArrayMatcher>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <QSerialPort>
#include <QTimer>

#include "enums.h"

class Buffer : public QObject {
  Q_OBJECT

private:
  QSerialPort serial;
  QTimer *timeoutTimer;
  QByteArray buffer;
  QByteArrayMatcher head;
  QByteArrayMatcher tail;
  QQueue<QPair<bool, QByteArray>> queue;

public:
  Buffer();
  QPair<bool, QByteArray> next();

private slots:
  void timeout();

public slots:
  void readNew();

signals:
  void newEntry();
};

#endif // BUFFER_H
