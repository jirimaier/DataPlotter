#ifndef RECEIVEDOUTPUTHANDLER_H
#define RECEIVEDOUTPUTHANDLER_H

#include "settings.h"
#include <QObject>

class ReceivedOutputHandler : public QObject {
  Q_OBJECT

private:
  int outputLevel = CMD_OUTPUT_LEVEL_LOW;

public:
  explicit ReceivedOutputHandler(QObject *parent = nullptr);

signals:
  void output(QString data);

public slots:
  void input(QByteArray message);
  void setLevel(int level) { outputLevel = level; };
};

#endif // RECEIVEDOUTPUTHANDLER_H
