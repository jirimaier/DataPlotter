//  Copyright (C) 2020-2024  Jiří Maier

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

#include "global.h"
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QTimer>

class NewSerialParser : public QObject {
  Q_OBJECT

public:
  explicit NewSerialParser(MessageTarget::enumMessageTarget target, QObject *parent = nullptr);
  ~NewSerialParser();

signals:
  /// Sends a message to the log
  void sendDeviceMessage(QByteArray header, bool warning, bool ended);
  /// Forwards a message from the device
  void sendMessage(QString header, QByteArray message, MessageLevel::enumMessageLevel type, MessageTarget::enumMessageTarget target);
  /// Sends the buffer contents
  void sendBuffer(QByteArray buffer);
  /// Sends data to the terminal
  void sendTerminal(QByteArray message);
  /// Sends a single setting value
  void sendSettings(QByteArray message, MessageTarget::enumMessageTarget source);
  /// Sends a file request
  void sendFileRequest(QByteArray message, MessageTarget::enumMessageTarget source);
  /// Sends a point for processing
  void sendPoint(QList<QPair<ValueType, QByteArray>> data);
  /// Sends a logic point for processing
  void sendLogicPoint(QPair<ValueType, QByteArray> timeArray, QPair<ValueType, QByteArray> valueArray, unsigned int bits);
  /// Sends a channel for processing
  void sendChannel(QPair<ValueType, QByteArray> data, unsigned int ch, QPair<ValueType, QByteArray> timeRaw, int zeroIndex, int bits, QPair<ValueType, QByteArray> min, QPair<ValueType, QByteArray> max);
  /// Sends a logic channel for processing
  void sendLogicChannel(QPair<ValueType, QByteArray> data, QPair<ValueType, QByteArray> timeRaw, int bits, int zeroIndex);
  /// Confirms readiness
  void ready();
  /// Sends data that should be echoed back to the port
  void sendEcho(QByteArray);
  /// Sends an error message from the device
  void deviceError(QByteArray, MessageTarget::enumMessageTarget source);
  /// Send compressed and stuffed qml file
  void sendQmlCode(QByteArray data);

  void sendFileToSave(QByteArray data);

  void sendQmlDirectInput(QByteArray data);

  void sendQmlVar(QByteArray data);

private:
  MessageTarget::enumMessageTarget target;
  void resetChHeader();
  bool channelHeaderRead = false;
  QPair<ValueType, QByteArray> channelTime;
  QList<QPair<ValueType, QByteArray>> additionalHeaderParameters;
  uint32_t channelLength;
  QList<int> channelNumber;
  void fatalError(QString header, QByteArray message);
  enum readResult { incomplete = 0, complete = 1, notProperlyEnded = 2 };
  enum delimiter { comma, semicolon, dollar, none };
  void sendMessageIfAllowed(QString header, QByteArray message, MessageLevel::enumMessageLevel type);
  void sendMessageIfAllowed(QString header, QString message, MessageLevel::enumMessageLevel type);
  DataMode::enumDataMode currentMode = DataMode::unknown;
  OutputLevel::enumOutputLevel debugLevel = OutputLevel::info;
  enum PrintUnknownToTerminal { puttNo, puttYesLF, puttYesCRLF, puttPending } printUnknownToTerminal = puttNo;
  QByteArray printUnknownToTerminalBuffer;
  QTimer *printUnknownToTerminalTimer = nullptr;
  QByteArray buffer;
  QByteArray pendingDataBuffer;
  QList<QPair<ValueType, QByteArray>> pendingPointBuffer;
  void parseMode(QChar modeChar);
  readResult bufferPullFull(QByteArray &result);
  void changeMode(DataMode::enumDataMode mode, DataMode::enumDataMode previousMode, QByteArray modeName);
  readResult bufferPullBeforeSemicolon(QByteArray &result, bool removeNewline = false);
  readResult bufferReadPoint(QList<QPair<ValueType, QByteArray>> &result);
  uint32_t arrayToUint(QPair<ValueType, QByteArray> value);
  readResult bufferPullChannel(QPair<ValueType, QByteArray> &result);
  bool replyToEcho = true;
  bool initialEchoPending = false;

  NewSerialParser::readResult bufferPullBeforeNull(QByteArray &result);

private slots:
  void printUnknownToTerminalTimerSlot();

public slots:
  /// Processes data
  void parse(QByteArray newData);
  /// Clear buffers
  void clearBuffer();
  /// Show content of buffers
  void showBuffer();
  /// Set messages level
  void setMsgLevel(OutputLevel::enumOutputLevel level) { debugLevel = level; }
  /// Clears the buffer and confirms readiness
  void getReady();
  /// Sets whether the reply to echo should be sent
  void replyEcho(bool enabled) { replyToEcho = enabled; }
};

#endif // NEWSERIALPARSER_H
