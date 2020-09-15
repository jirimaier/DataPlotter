#ifndef BUFFER_H
#define BUFFER_H

#include <QByteArrayMatcher>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <QTimer>

#include "enums.h"

class Buffer : public QObject {
  Q_OBJECT

private:
  QTimer *timeoutTimer;
  QByteArray buffer;
  QByteArrayMatcher head;
  QByteArrayMatcher tail;
  QQueue<QPair<bool, QByteArray>> queue;

public:
  Buffer();
  void add(QByteArray data);
  QPair<bool, QByteArray> next();

private slots:
  void timeout();

signals:
  void newEntry();
};

#endif // BUFFER_H
