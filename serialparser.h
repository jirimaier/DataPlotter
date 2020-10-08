#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>

#include "myterminal.h"
#include "plotdata.h"
#include "enums_defines_constants.h"

class SerialParser : public QObject {
  Q_OBJECT
public:
  SerialParser();
  ~SerialParser();
  void init();

private:
  bool modeChangeAllowed = true;
  BinDataSettings_t binDataSettings;
  int dataMode = DataMode::unknown;
  void parseBinaryDataHeader(QByteArray data);
  void changeMode(int mode);

public slots:
  /// Přijme řádek ke zpracování
  void parseLine(QByteArray line, int type);
  /// Nastaví parametry pro zpracování binárních dat
  void setBinParameters(BinDataSettings_t settings);
  /// Nastaví režim zpracování
  void setMode(int mode);
  /// Nastaví jestli se má rezim řídit vstupem, nebo ručním nastavením
  void allowModeChange(bool enabled) { modeChangeAllowed = enabled; }

signals:
  /// Pošle data do terminálu
  void printToTerminal(QByteArray message);
  /// Pošle zprávu k vypsání
  void printMessage(QByteArray message, bool urgent);
  /// Pošle decimální data
  void newDataString(QByteArray data);
  /// Pošle binární data
  void newDataBin(QByteArray data, BinDataSettings_t settings);
  /// Pošle zpracovaný řádek
  void newProcessedLine(QByteArray);
  /// Pošle aktuální nastavení pro binární data
  void changedBinSettings(BinDataSettings_t);
  /// Pošle aktuální režim
  void changedDataMode(int mode);
  /// Pošle zprávu o chybě v nastavení
  void parseError(QByteArray, int type = DataLineType::debugMessage);
  /// Pošle příkaz k změně nastavení
  void changeGUISettings(QByteArray);
};

#endif // SERIALHANDLER_H
