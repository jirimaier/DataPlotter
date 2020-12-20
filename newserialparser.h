//  Copyright (C) 2020  Jiří Maier

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef NEWSERIALPARSER_H
#define NEWSERIALPARSER_H

#include "enums_defines_constants.h"
#include <QDebug>
#include <QObject>
#include <QThread>

class NewSerialParser : public QObject {
  Q_OBJECT
public:
  explicit NewSerialParser(MessageTarget::enumerator target, QObject *parent = nullptr);
  ~NewSerialParser();
  void init();

signals:
  /// Pošle zprávu do záznamu
  void sendDeviceMessage(QByteArray header, bool warning, bool ended);
  /// Předá zprávu od zařízení
  void sendMessage(QString header, QByteArray message, MessageLevel::enumerator type, MessageTarget::enumerator target);
  /// Pošle obsah bufferu
  void sendBuffer(QByteArray buffer);
  /// Pošle data do terminálu
  void sendTerminal(QByteArray message);
  /// Pošle nastavení (jedno, ne celý úsek)
  void sendSettings(QByteArray message, MessageTarget::enumerator source);
  /// Pošle bod ke zpracování
  void sendPoint(QByteArrayList data);
  /// Pošle kanál ke zpracování
  void sendChannel(QByteArray data, unsigned int ch, QByteArray timeRaw, int bits, QByteArray min, QByteArray max);
  /// Potvrdí připravenost
  void ready();

private:
  MessageTarget::enumerator target;
  void resetChHeader();
  bool channelHeaderOK = false;
  QByteArray channelTime, channelMin, channelMax;
  int channelBits = -1;
  uint32_t channelLength;
  int channelNumber;
  void fatalError(QString header, QByteArray &message);
  enum readResult { incomplete = 0, complete = 1, notProperlyEnded = 2 };
  enum delimiter { comma, semicolon, dollar, none };
  void sendMessageIfAllowed(QString header, QByteArray &message, MessageLevel::enumerator type);
  void sendMessageIfAllowed(QString header, QString &message, MessageLevel::enumerator type);
  DataMode::enumerator currentMode = DataMode::unknown;
  OutputLevel::enumerator debugLevel = OutputLevel::info;
  QByteArray buffer;
  QByteArray pendingDataBuffer;
  QByteArrayList pendingPointBuffer;
  void parseMode(QChar modeChar);
  readResult bufferPullFull(QByteArray &result);
  void changeMode(DataMode::enumerator mode, DataMode::enumerator previousMode, QByteArray modeName);
  readResult bufferPullBeforeSemicolumn(QByteArray &result, bool removeNewline = false);
  readResult bufferPullPoint(QByteArrayList &result);
  uint32_t arrayToUint(QByteArray array, bool &isok);
  readResult bufferPullChannel(QByteArray &result);

public slots:
  /// Zpracuje data
  void parse(QByteArray newData);
  /// Clear buffers
  void clearBuffer();
  /// Show content of buffers
  void showBuffer();
  /// Set messages level
  void setMsgLevel(OutputLevel::enumerator level) { debugLevel = level; }
  /// Vymaže buffer a potvrdí připravenost
  void getReady();
};

#endif // NEWSERIALPARSER_H
