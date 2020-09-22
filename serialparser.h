#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>

#include "buffer.h"
#include "enums.h"
#include "myterminal.h"
#include "plotdata.h"
#include "serialthread.h"
#include "settings.h"

class SerialParser : public QObject {
  Q_OBJECT
public:
  SerialParser(Settings *in_settings);
  ~SerialParser();

private:
  Settings *settings;
  Buffer *buffer;
  void parseBinaryDataHeader(QByteArray data);

public slots:
  void parseCommand(QByteArray line);
  void parseData(QByteArray line);

signals:
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
