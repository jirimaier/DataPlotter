#ifndef OUTPUTWORKER_H
#define OUTPUTWORKER_H

#include "settings.h"
#include <QDebug>
#include <QObject>
#include <QThread>

class OutputWorker : public QObject {
  Q_OBJECT

private:
  int outputLevel = OutputLevel::low;

public:
  explicit OutputWorker(QObject *parent = nullptr);
  ~OutputWorker();

signals:
  void output(QString data);

public slots:
  void input(QByteArray message);
  void setLevel(int level) { outputLevel = level; };
};

#endif // OUTPUTWORKER
