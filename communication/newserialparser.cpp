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

#include "newserialparser.h"

NewSerialParser::NewSerialParser(MessageTarget::enumMessageTarget target, QObject *parent) : QObject(parent) {
  this->target = target;
  resetChHeader();
}

NewSerialParser::~NewSerialParser() {
  if (printUnknownToTerminalTimer != nullptr)
    delete printUnknownToTerminalTimer;
}

void NewSerialParser::resetChHeader() {
  channelHeaderRead = false;
  channelLength = 0;
  channelNumber.clear();
  channelTime.second.clear();
  additionalHeaderParameters.clear();
}

void NewSerialParser::sendMessageIfAllowed(QString header, QByteArray message, MessageLevel::enumMessageLevel type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message, type, target);
}

void NewSerialParser::sendMessageIfAllowed(QString header, QString message, MessageLevel::enumMessageLevel type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message.toUtf8(), type, target);
}

void NewSerialParser::showBuffer() {
  if ((!buffer.isEmpty()) || (!pendingDataBuffer.isEmpty())) {
    QByteArray bufferToPrint = pendingDataBuffer + buffer;
    // nahradí netisknutelné znaky hex číslem
    for (unsigned char ch = 0;; ch++) {
      if (ch == 32)
        ch = 127;
      bufferToPrint.replace(ch, " 0x" + QString::number((unsigned int)ch, 16).toLocal8Bit() + " ");
      if (ch == 255)
        break;
    }
    emit sendMessage(tr("Buffer"), bufferToPrint, MessageLevel::info, target);
  }
  if (!pendingPointBuffer.isEmpty()) {
    QByteArray pointBuffer;
    emit sendMessage(tr("Point buffer content"), pointBuffer, MessageLevel::info, target);
    foreach (auto line, pendingPointBuffer)
      emit sendMessage("->" + valueTypeToString(line.first).toUtf8(), (line.first.isBinary ? line.second.toHex() : line.second), MessageLevel::info, target);
  }
  if (buffer.isEmpty() && pendingDataBuffer.isEmpty() && pendingPointBuffer.isEmpty())
    emit sendMessage(tr("Buffer is empty"), "", MessageLevel::info, target);
}

void NewSerialParser::getReady() {
  clearBuffer();
  initialEchoPending = true;
  printUnknownToTerminalBuffer.clear();
  printUnknownToTerminal = puttPending;
  if (printUnknownToTerminalTimer == nullptr) {
    printUnknownToTerminalTimer = new QTimer(this);
    connect(printUnknownToTerminalTimer, &QTimer::timeout, this, &NewSerialParser::printUnknownToTerminalTimerSlot);
  }
  printUnknownToTerminalTimer->setSingleShot(true);

  printUnknownToTerminalTimer->start(1000);

  emit ready();
}

void NewSerialParser::clearBuffer() {
  if (!pendingDataBuffer.isEmpty())
    pendingDataBuffer.clear();
  if (!pendingPointBuffer.isEmpty())
    pendingPointBuffer.clear();
  if (!buffer.isEmpty())
    buffer.clear();
  changeMode(DataMode::unknown, currentMode, tr("Unknown").toUtf8());
  resetChHeader();
}

void NewSerialParser::parse(QByteArray newData) {
  buffer.push_back(newData);
  while (!buffer.isEmpty()) {
    try {
      if (buffer.length() >= 3) {
        if (buffer.left(2) == "$$") {
          if (currentMode == DataMode::info || currentMode == DataMode::warning)
            emit sendDeviceMessage("", false, true); // Pokud byl předchozí režim výpis zprávy, ohlásí její konec
          if (currentMode == DataMode::initialEcho)
            initialEchoPending = false;
          parseMode(buffer.at(2));
          buffer.remove(0, 3);
          continue;
        }
      } else {
        if (buffer == "$")
          break;
        if (buffer == "$$")
          break;
      }

      /*if (buffer.length() >= 1 && removeCommaRightAfterTypeID) {
        if (buffer.left(1) == ",")
          buffer.remove(0, 1);
        removeCommaRightAfterTypeID = false;
      }*/

      if (currentMode == DataMode::point) {
        readResult result;
        try {
          result = bufferReadPoint(pendingPointBuffer);
        } catch (QString msg) {
          throw(tr("Error reading point: ") + msg);
        }

        if (result == incomplete)
          break;

        if (pendingPointBuffer.length() < 2)
          throw(tr("Point has no value"));

        if (result == complete) {
          emit sendPoint(pendingPointBuffer);
          pendingPointBuffer.clear();
          continue;
        }
        if (result == notProperlyEnded) {
          sendMessageIfAllowed(tr("Missing semicolon ?"), pendingPointBuffer.last().second, MessageLevel::warning);
          emit sendPoint(pendingPointBuffer);
          pendingPointBuffer.clear();
          continue;
        }
      }

      if (currentMode == DataMode::logicPoint) {
        readResult result;
        try {
          result = bufferReadPoint(pendingPointBuffer);
        } catch (QString msg) {
          throw(tr("Error reading logic point: ") + msg);
        }

        if (result == incomplete)
          break;

        if (pendingPointBuffer.length() < 2)
          throw(tr("Logic point has no value"));

        if (pendingPointBuffer.length() > 3)
          throw(tr("Too many values in logic point"));

        if (pendingPointBuffer.at(1).first.type != ValueType::unsignedint)
          throw(tr("Logic point value is not unsigned integer data type"));

        unsigned int bits;
        if (pendingPointBuffer.length() == 3) {
          try {
            bits = arrayToUint(pendingPointBuffer.at(2));
          } catch (QString msg) {
            throw(tr("Invalid logic point: ") + tr("Invalid number of bits - ") + msg);
          }
        } else
          bits = pendingPointBuffer.at(1).first.bytes * 8;

        if (result == complete) {
          emit sendLogicPoint(pendingPointBuffer.at(0), pendingPointBuffer.at(1), bits);
          pendingPointBuffer.clear();
          continue;
        }
        if (result == notProperlyEnded) {
          sendMessageIfAllowed(tr("Missing semicolon ?"), pendingPointBuffer.last().second, MessageLevel::warning);
          emit sendLogicPoint(pendingPointBuffer.at(0), pendingPointBuffer.at(1), bits);
          pendingPointBuffer.clear();
          continue;
        }
      }

      if (currentMode == DataMode::unknown) {

        if (printUnknownToTerminal == puttYesLF || printUnknownToTerminal == puttYesCRLF) {
          QByteArray data;
          readResult result = bufferPullFull(data);
          if (printUnknownToTerminal == puttYesLF && !data.contains('\r'))
            data.replace('\n', "\r\n");
          else
            printUnknownToTerminal = puttYesCRLF;
          emit sendTerminal(data);
          if (result == complete)
            continue;
          break;
        } else {
          QByteArray dropped;
          readResult result = bufferPullFull(dropped);
          if (printUnknownToTerminal == puttPending)
            printUnknownToTerminalBuffer.append(dropped);
          else if (!dropped.isEmpty())
            sendMessageIfAllowed(tr("Unknown"), dropped, MessageLevel::info);
          if (result == complete)
            continue;
          break;
        }
      }

      if (currentMode == DataMode::channel) {
        if (!channelHeaderRead) {
          readResult result = bufferReadPoint(pendingPointBuffer);
          if (result == complete) {
            if (pendingPointBuffer.length() >= 3 && pendingPointBuffer.length() <= 7) {
              channelHeaderRead = true;
              try {
                if (pendingPointBuffer.at(0).first.isBinary || !pendingPointBuffer.at(0).second.contains('+')) {
                  // Jen jeden kanál
                  channelNumber.append(arrayToUint(pendingPointBuffer.at(0)));
                  if (channelNumber.at(0) == 0 || channelNumber.at(0) > ANALOG_COUNT)
                    throw(tr("out of range (1 - %1): %2").arg(ANALOG_COUNT).arg(channelNumber.at(0)));
                } else {
                  // Vícero kanálů na přeskáčku
                  QByteArrayList values = pendingPointBuffer.at(0).second.split('+');
                  for (int i = 0; i < values.size(); i++) {
                    channelNumber.append(values.at(i).toUInt());
                    if (channelNumber.at(i) == 0 || channelNumber.at(0) > ANALOG_COUNT)
                      throw(tr("out of range (1 - %1): %2").arg(ANALOG_COUNT).arg(channelNumber.at(0)));
                  }
                }
              } catch (QString msg) {
                throw(tr("Invalid channel: ") + tr("Invalid channel number - ") + msg);
              }

              channelTime = pendingPointBuffer.at(1);

              try {
                channelLength = arrayToUint(pendingPointBuffer.at(2));
              } catch (QString msg) {
                throw(tr("Invalid channel: ") + tr("Invalid channel length - ") + msg);
              }

              for (int i = 3; i < pendingPointBuffer.length(); i++) {
                additionalHeaderParameters.append(pendingPointBuffer.at(i));
              }

              pendingPointBuffer.clear();

            } else
              throw(tr("Invalid channel: ") + tr("Wrong header length (%1 entries)").arg(pendingPointBuffer.length()));
          }
          if (result == incomplete)
            break;
          if (result == notProperlyEnded)
            throw(tr("Invalid channel: ") + tr("Header not properly ended"));
        }
        if (channelHeaderRead) {
          QPair<ValueType, QByteArray> channel;
          readResult result;
          try {
            result = bufferPullChannel(channel);
          } catch (QString msg) {
            throw(tr("Error reading channel: ") + msg);
          }

          if (result == incomplete)
            break;

          if (result == notProperlyEnded && debugLevel >= OutputLevel::warning) {
            QByteArray semicolonPositionMessage = tr("(enable info messages to show nearest semicolon position)").toUtf8();
            if (debugLevel == OutputLevel::info) {
              semicolonPositionMessage = tr("No semicolon found").toUtf8();
              if (buffer.contains(';') && channel.second.contains(';'))
                semicolonPositionMessage = tr("There are semicolons %1 byte before and %2 after end.").arg(buffer.indexOf(';')).arg(channel.second.length() - channel.second.lastIndexOf(';')).toUtf8();
              else {
                if (channel.second.contains(';'))
                  semicolonPositionMessage = tr("There is semicolon %1 bytes before end.").arg(channel.second.length() - channel.second.lastIndexOf(';')).toUtf8();
                if (buffer.contains(';'))
                  semicolonPositionMessage = tr("There is semicolon %1 bytes after end.").arg(buffer.indexOf(';')).toUtf8();
              }
            }

            emit sendMessage(tr("Channel not ended with ';'"), semicolonPositionMessage, MessageLevel::warning, target);
          }

          int zeroIndex = 0, channelBits = channel.first.bytes * 8;
          QPair<ValueType, QByteArray> channelMin, channelMax;

          if (channel.first.type == ValueType::Type::floatingpoint) {
            if (!additionalHeaderParameters.isEmpty()) {
              if (additionalHeaderParameters.size() == 1) {
                try {
                  zeroIndex = arrayToUint(additionalHeaderParameters.first());
                  pendingPointBuffer.clear();
                } catch (QString msg) {
                  throw(tr("Invalid channel: ") + tr("Invalid zero position - ") + msg);
                }
              } else
                throw(tr("Invalid channel: ") + tr("To many header entries for floating point type"));
            }
          } else if (channel.first.type == ValueType::Type::unsignedint) {
            if (additionalHeaderParameters.length() == 1) {
              try {
                zeroIndex = arrayToUint(additionalHeaderParameters.at(0));
                pendingPointBuffer.clear();
              } catch (QString msg) {
                throw(tr("Invalid channel: ") + tr("Invalid zero position - ") + msg);
              }
            }
            if (additionalHeaderParameters.length() >= 2) {
              try {
                channelBits = arrayToUint(additionalHeaderParameters.at(0));
              } catch (QString msg) {
                throw(tr("Invalid channel: ") + tr("Invalid number of bits - ") + msg);
              }
            }
            if (additionalHeaderParameters.length() == 2)
              channelMax = additionalHeaderParameters.at(1);
            if (additionalHeaderParameters.length() >= 3) {
              channelMin = additionalHeaderParameters.at(1);
              channelMax = additionalHeaderParameters.at(2);
            }
            if (additionalHeaderParameters.length() == 4) {
              try {
                zeroIndex = arrayToUint(additionalHeaderParameters.at(3));
                pendingPointBuffer.clear();
              } catch (QString msg) {
                throw(tr("Invalid channel: ") + tr("Invalid zero position - ") + msg);
              }
            }
            // Delší by neprošlo kontrolou při čtení záhlaví
          } else if (channel.first.type == ValueType::Type::integer) {
            if (!additionalHeaderParameters.isEmpty()) {
              if (additionalHeaderParameters.size() == 1) {
                try {
                  zeroIndex = arrayToUint(additionalHeaderParameters.first());
                  pendingPointBuffer.clear();
                } catch (QString msg) {
                  throw(tr("Invalid channel: ") + tr("Invalid zero position - ") + msg);
                }
              } else
                throw(tr("Invalid channel: ") + tr("To many header entries for signed integer type"));
            }
          }
          if (channelNumber.size() == 1)
            emit sendChannel(channel, channelNumber.first(), channelTime, zeroIndex, channelBits, channelMin, channelMax);
          else {
            int N = channelNumber.size();
            QVector<QByteArray> subChannels;
            subChannels.resize(N);
            for (int i = 0; i < channel.second.size(); i++)
              subChannels[(i / channel.first.bytes) % N].append(channel.second.at(i));
            for (int i = 0; i < N; i++)
              emit sendChannel(QPair<ValueType, QByteArray>(channel.first, subChannels.at(i)), channelNumber.at(i), channelTime, zeroIndex, channelBits, channelMin, channelMax);
          }
          resetChHeader();
          continue;
        }
      }

      if (currentMode == DataMode::logicChannel) {
        if (!channelHeaderRead) {
          readResult result = bufferReadPoint(pendingPointBuffer);
          if (result == complete) {
            if (pendingPointBuffer.length() >= 2 && pendingPointBuffer.length() <= 4) {
              channelHeaderRead = true;

              channelTime = pendingPointBuffer.at(0);

              try {
                channelLength = arrayToUint(pendingPointBuffer.at(1));
              } catch (QString msg) {
                throw(tr("Invalid logic channel: ") + tr("Invalid channel length - ") + msg);
              }

              for (int i = 2; i < pendingPointBuffer.length(); i++) {
                additionalHeaderParameters.append(pendingPointBuffer.at(i));
              }

              pendingPointBuffer.clear();

            } else
              throw(tr("Invalid logic channel: ") + tr("Wrong header length (%1 entries)").arg(pendingPointBuffer.length()));
          }
          if (result == incomplete)
            break;
          if (result == notProperlyEnded)
            throw(tr("Invalid logic channel: ") + tr("Header not properly ended"));
        }
        if (channelHeaderRead) {
          QPair<ValueType, QByteArray> channel;
          readResult result;
          try {
            result = bufferPullChannel(channel);
          } catch (QString msg) {
            throw(tr("Error reading logic channel: ") + msg);
          }

          if (result == incomplete)
            break;

          if (result == notProperlyEnded && debugLevel >= OutputLevel::warning) {
            QByteArray semicolonPositionMessage = tr("(enable info messages to show nearest semicolon position)").toUtf8();
            if (debugLevel == OutputLevel::info) {
              semicolonPositionMessage = tr("No semicolon found").toUtf8();
              if (buffer.contains(';') && channel.second.contains(';'))
                semicolonPositionMessage = tr("There are semicolons %1 byte before and %2 after end.").arg(buffer.indexOf(';')).arg(channel.second.length() - channel.second.lastIndexOf(';')).toUtf8();
              else {
                if (channel.second.contains(';'))
                  semicolonPositionMessage = tr("There is semicolon %1 bytes before end.").arg(channel.second.length() - channel.second.lastIndexOf(';')).toUtf8();
                if (buffer.contains(';'))
                  semicolonPositionMessage = tr("There is semicolon %1 bytes after end.").arg(buffer.indexOf(';')).toUtf8();
              }
            }
            emit sendMessage(tr("Logic channel not ended with ';'"), semicolonPositionMessage, MessageLevel::warning, target);
          }

          int zeroIndex = 0, channelBits = channel.first.bytes * 8;

          if (!(channel.first.type == ValueType::Type::unsignedint))
            emit sendMessage(tr("Logic channel warning"), tr("data are not designated as unsigned integer").toUtf8(), MessageLevel::warning, target);

          if (!additionalHeaderParameters.isEmpty()) {
            try {
              channelBits = arrayToUint(additionalHeaderParameters.first());
            } catch (QString msg) {
              throw(tr("Invalid logic channel: ") + tr("Invalid number of bits - ") + msg);
            }
          }
          if (additionalHeaderParameters.length() == 2) {
            try {
              zeroIndex = arrayToUint(additionalHeaderParameters.at(1));
              pendingPointBuffer.clear();
            } catch (QString msg) {
              throw(tr("Invalid logic channel: ") + tr("Invalid zero position - ") + msg);
            }
          }
          // Delší by neprošlo kontrolou při čtení záhlaví

          emit sendLogicChannel(channel, channelTime, channelBits, zeroIndex);

          resetChHeader();
          continue;
        }
      }

      if (currentMode == DataMode::info || currentMode == DataMode::warning) {
        QByteArray message;
        readResult result = bufferPullFull(message);
        emit sendDeviceMessage(message, currentMode == DataMode::warning, result == complete);
        if (result == complete)
          continue;
        break;
      }

      if (currentMode == DataMode::terminal) {
        QByteArray data;
        readResult result = bufferPullFull(data);
        emit sendTerminal(data);
        if (result == complete)
          continue;
        break;
      }

      if (currentMode == DataMode::echo) {
        QByteArray data;
        readResult result = bufferPullFull(data);
        if (replyToEcho)
          emit sendEcho(data);
        if (result == complete)
          continue;
        break;
      }

      if (currentMode == DataMode::initialEcho) {
        QByteArray data;
        readResult result = bufferPullFull(data);
        if (replyToEcho && initialEchoPending)
          emit sendEcho(data);
        if (result == complete)
          continue;
        break;
      }

      if (currentMode == DataMode::settings) {
        readResult result = bufferPullBeforeSemicolon(pendingDataBuffer, true);
        if (result == complete) {
          if (!pendingDataBuffer.isEmpty()) {
            emit sendSettings(pendingDataBuffer, target);
            pendingDataBuffer.clear();
            continue;
          }
        }
        if (result == notProperlyEnded) {
          if (!pendingDataBuffer.isEmpty()) {
            emit sendSettings(pendingDataBuffer, target);
            sendMessageIfAllowed(tr("Missing semicolon ?"), pendingDataBuffer, MessageLevel::warning);
            pendingDataBuffer.clear();
            continue;
          }
        }
        break;
      }

      if (currentMode == DataMode::qmlvar) {
        readResult result = bufferPullBeforeSemicolon(pendingDataBuffer, true);
        if (result == complete) {
          if (!pendingDataBuffer.isEmpty()) {
            emit sendQmlVar(pendingDataBuffer);
            pendingDataBuffer.clear();
            continue;
          }
        }
        if (result == notProperlyEnded) {
          if (!pendingDataBuffer.isEmpty()) {
            emit sendQmlVar(pendingDataBuffer);
            sendMessageIfAllowed(tr("Missing semicolon ?"), pendingDataBuffer, MessageLevel::warning);
            pendingDataBuffer.clear();
            continue;
          }
        }
        break;
      }

      if (currentMode == DataMode::requestfile) {
        readResult result = bufferPullBeforeSemicolon(pendingDataBuffer, true);
        if (result == complete) {
          emit sendFileRequest(pendingDataBuffer, target);
          pendingDataBuffer.clear();
          continue;
        }
        if (result == notProperlyEnded) {
          emit sendFileRequest(pendingDataBuffer, target);
          sendMessageIfAllowed(tr("Missing semicolon ?"), pendingDataBuffer, MessageLevel::warning);
          pendingDataBuffer.clear();
          continue;
        }
        break;
      }

      if (currentMode == DataMode::qml) {
        readResult result = bufferPullBeforeNull(pendingDataBuffer);
        if (result == complete) {
          emit sendQmlCode(pendingDataBuffer);
          pendingDataBuffer.clear();
          continue;
        }
        break;
      }

      if (currentMode == DataMode::qmldirect) {
        readResult result = bufferPullBeforeNull(pendingDataBuffer);
        if (result == complete) {
          emit sendQmlDirectInput(pendingDataBuffer);
          pendingDataBuffer.clear();
          continue;
        }
        break;
      }

      if (currentMode == DataMode::savefile) {
        readResult result = bufferPullBeforeNull(pendingDataBuffer);
        if (result == complete) {
          emit sendFileToSave(pendingDataBuffer);
          pendingDataBuffer.clear();
          continue;
        }
        break;
      }

      if (currentMode == DataMode::deviceerror) {
        readResult result = bufferPullBeforeSemicolon(pendingDataBuffer, true);
        if (result == incomplete)
          break;
        if (result == notProperlyEnded)
          sendMessageIfAllowed(tr("Missing semicolon ?"), pendingDataBuffer, MessageLevel::warning);
        emit deviceError(pendingDataBuffer, target);
        pendingDataBuffer.clear();
        continue;
      }

    } catch (QString message) {
      sendMessageIfAllowed(tr("Parsing error"), message, MessageLevel::error);
      if (!buffer.isEmpty()) {
        if (buffer.contains("$$"))
          buffer.remove(0, buffer.indexOf("$$"));
        else
          buffer.clear();
        if (!pendingDataBuffer.isEmpty())
          pendingDataBuffer.clear();
        if (!pendingPointBuffer.isEmpty())
          pendingPointBuffer.clear();
        resetChHeader();
      }
      changeMode(DataMode::unknown, currentMode, tr("Unknown").toUtf8());
    } catch (...) {
      sendMessageIfAllowed(tr("Fatal error"), QString(""), MessageLevel::error);
    }
  }
}

NewSerialParser::readResult NewSerialParser::bufferReadPoint(QList<QPair<ValueType, QByteArray>> &result) {
  while (!buffer.isEmpty()) {
    // Textové číslo
    if (buffer.at(0) == ' ') {
      buffer.remove(0, 1);
      continue;
    }

    if (buffer.at(0) == ',') {
      if (buffer.length() > 1) {
        if (buffer.at(1) == ',') {
          ValueType valType(false);
          result.append(QPair<ValueType, QByteArray>(valType, ""));
        }
        buffer.remove(0, 1);
      } else
        return incomplete;
    }

    // NaN or Inf
    if (buffer.at(0) == 'n' || buffer.at(0) == 'N' || buffer.at(0) == 'i' || buffer.at(0) == 'I') {
      if (buffer.length() == 1)
        return incomplete;
      if (buffer.length() == 2 && (buffer.at(1) == 'a' || buffer.at(1) != 'A' || buffer.at(1) == 'n' || buffer.at(1) != 'N'))
        return incomplete;

      if (buffer.left(3).toLower() == "nan") {
        ValueType valType(false);
        result.append(QPair<ValueType, QByteArray>(valType, ""));
        sendMessageIfAllowed(tr("Received NaN"), tr("Treated as no value"), MessageLevel::warning);
        buffer.remove(0, 3);
      } else if (buffer.left(3).toLower() == "inf") {
        ValueType valType(false);
        result.append(QPair<ValueType, QByteArray>(valType, ""));
        sendMessageIfAllowed(tr("Received Inf"), tr("Treated as no value"), MessageLevel::warning);
        buffer.remove(0, 3);
      }

      if (buffer.isEmpty())
        continue;
      if (buffer.at(0) == ';') {
        buffer.remove(0, 1);
        return complete;
      }
      if (buffer.length() >= 2)
        if (buffer.left(2) == "$$")
          return notProperlyEnded;
      continue;
    }

    if (IS_NUMERIC_CHAR(buffer.at(0))) {
      // Hledám který znak je nejblýž: , ; nebo $
      // Pokud se nevyskytuje, funkce vrátí -1, což se v unsigned int zmení na max hodnotu int a tedy rozhodně nebude nejblýže
      ValueType valType(false);
      unsigned int comma = buffer.indexOf(',');
      unsigned int semicolon = buffer.indexOf(';');
      unsigned int dollar = buffer.indexOf('$');
      if (comma < semicolon && comma < dollar) {
        // Nejblýže je čárka
        QByteArray value = buffer.left(comma);
        // Samotná pomlčka se považuje za vynechaní kanál
        if (value == "-")
          value.clear();
        if (value.toLower() == "-inf") {
          value.clear();
          sendMessageIfAllowed(tr("Received -Inf"), tr("Treated as no value"), MessageLevel::warning);
        }
        result.append(QPair<ValueType, QByteArray>(valType, value));
        buffer.remove(0, comma + 1);
        continue;
      } else if (semicolon < comma && semicolon < dollar) {
        QByteArray value = buffer.left(semicolon);
        if (value == "-")
          value.clear();
        if (value.toLower() == "-inf") {
          value.clear();
          sendMessageIfAllowed(tr("Received -Inf"), tr("Treated as no value"), MessageLevel::warning);
        }
        result.append(QPair<ValueType, QByteArray>(valType, value));
        buffer.remove(0, semicolon + 1);
        return complete;
      } else if (dollar < comma && dollar < semicolon) {
        QByteArray value = buffer.left(dollar);
        if (value == "-")
          value.clear();
        if (value.toLower() == "-inf") {
          value.clear();
          sendMessageIfAllowed(tr("Received -Inf"), tr("Treated as no value"), MessageLevel::warning);
        }
        result.append(QPair<ValueType, QByteArray>(valType, value));
        buffer.remove(0, dollar);
        return notProperlyEnded;
      } else
        // Žádné není nejmenší (jsou si rovny a jsou na maximu, tedy znak není)
        return incomplete;
    } else {
      if (buffer.length() == 1) {
        if (buffer.at(0) == ';') {
          // End of point
          buffer.remove(0, 1);
          return complete;
        } else
          // V bufferu není celý bod
          return incomplete;
      }

      // Binary data
      int prefixLength = 0;
      ValueType valType = readValuePrefix(buffer, prefixLength);
      if (valType.type == ValueType::Type::invalid) {
        throw(tr("Expected value, but \"%1\" found.").arg(QString(buffer.left(prefixLength))));
      }
      if (buffer.length() < valType.bytes + prefixLength || valType.type == ValueType::Type::incomplete)
        return incomplete;
      buffer.remove(0, prefixLength);
      result.append(QPair<ValueType, QByteArray>(valType, buffer.left(valType.bytes)));
      buffer.remove(0, valType.bytes);
      if (buffer.isEmpty())
        continue;
      if (buffer.at(0) == ';') {
        buffer.remove(0, 1);
        return complete;
      }
      if (buffer.length() >= 2)
        if (buffer.left(2) == "$$")
          return notProperlyEnded;
      continue;
    }
  }
  return incomplete;
}

uint32_t NewSerialParser::arrayToUint(QPair<ValueType, QByteArray> value) {
  // String
  if (!value.first.isBinary) {
    bool isok = true;
    double val = value.second.toUInt(&isok);
    if (!isok)
      throw(tr("Value \"%1\" is not a valid integer.").arg(QString(value.second)));
    if (val < 0)
      throw(tr("Value is negative: %1").arg(QString(value.second)));
    return val;
  }
  if (value.first.type != ValueType::Type::unsignedint)
    throw(QString(tr("Value is not unsigned integer type")));
  if (value.first.bigEndian) {
    // Big endian
    if (value.first.bytes == 1) { // unsigned int 8
      return (double)((uint8_t)value.second.at(0));
    } else if (value.first.bytes == 2) { // unsigned int 16
      char bytes[2] = {value.second.at(1), value.second.at(0)};
      return (double)*((uint16_t *)bytes);
    } else if (value.first.bytes == 3) { // unsigned int 32
      char bytes[4] = {value.second.at(2), value.second.at(1), value.second.at(0), 0};
      return (double)*((uint32_t *)bytes);
    } else if (value.first.bytes == 4) { // unsigned int 32
      char bytes[4] = {value.second.at(3), value.second.at(2), value.second.at(1), value.second.at(0)};
      return (double)*((uint32_t *)bytes);
    }
  } else {
    // Little endian
    if (value.first.bytes == 1) { // unsigned int 8
      return (double)((uint8_t)value.second.at(0));
    } else if (value.first.bytes == 2) { // unsigned int 16
      char bytes[2] = {value.second.at(0), value.second.at(1)};
      return (double)*((uint16_t *)bytes);
    } else if (value.first.bytes == 3) { // unsigned int 32
      char bytes[4] = {value.second.at(0), value.second.at(1), value.second.at(2), 0};
      return (double)*((uint32_t *)bytes);
    } else if (value.first.bytes == 4) { // unsigned int 32
      char bytes[4] = {value.second.at(0), value.second.at(1), value.second.at(2), value.second.at(3)};
      return (double)*((uint32_t *)bytes);
    }
  }
  return 0;
}

void NewSerialParser::parseMode(QChar modeChar) {
  DataMode::enumDataMode previousMode = currentMode;
  QChar modeIdent = modeChar.toUpper();

  if (modeIdent == 'P')
    changeMode(DataMode::point, previousMode, tr("Points").toUtf8());
  else if (modeIdent == 'T')
    changeMode(DataMode::terminal, previousMode, tr("Terminal").toUtf8());
  else if (modeIdent == 'I')
    changeMode(DataMode::info, previousMode, tr("Info").toUtf8());
  else if (modeIdent == 'W')
    changeMode(DataMode::warning, previousMode, tr("Warning").toUtf8());
  else if (modeIdent == 'C')
    changeMode(DataMode::channel, previousMode, tr("Channel").toUtf8());
  else if (modeIdent == 'R')
    changeMode(DataMode::requestfile, previousMode, tr("Request file").toUtf8());
  else if (modeIdent == 'S')
    changeMode(DataMode::settings, previousMode, tr("Settings").toUtf8());
  else if (modeIdent == 'U')
    changeMode(DataMode::unknown, previousMode, tr("Unknown").toUtf8());
  else if (modeIdent == 'E')
    changeMode(DataMode::echo, previousMode, tr("Echo").toUtf8());
  else if (modeIdent == 'A')
    changeMode(DataMode::initialEcho, previousMode, tr("Initial Echo").toUtf8());
  else if (modeIdent == 'L')
    changeMode(DataMode::logicChannel, previousMode, tr("Logic channel").toUtf8());
  else if (modeIdent == 'B')
    changeMode(DataMode::logicPoint, previousMode, tr("Logic points").toUtf8());
  else if (modeIdent == 'X')
    changeMode(DataMode::deviceerror, previousMode, tr("Device error").toUtf8());
  else if (modeIdent == 'Q')
    changeMode(DataMode::qml, previousMode, tr("Qml code").toUtf8());
  else if (modeIdent == 'D')
    changeMode(DataMode::qmldirect, previousMode, tr("Qml direct input").toUtf8());
  else if (modeIdent == 'V')
    changeMode(DataMode::qmlvar, previousMode, tr("Qml variable").toUtf8());
  else if (modeIdent == 'F')
    changeMode(DataMode::savefile, previousMode, tr("Save file").toUtf8());
  else {
    currentMode = DataMode::unknown;
    QByteArray character = QString(modeChar).toLocal8Bit();
    sendMessageIfAllowed(tr("Unknown data type"), character, MessageLevel::error);
  }

  if (!pendingDataBuffer.isEmpty())
    pendingDataBuffer.clear();
  if (!pendingPointBuffer.isEmpty())
    pendingPointBuffer.clear();
  resetChHeader();
}

NewSerialParser::readResult NewSerialParser::bufferPullFull(QByteArray &result) {
  if (buffer.contains("$$")) {
    result.push_back(buffer.left(buffer.indexOf("$$")));
    buffer.remove(0, buffer.indexOf("$$"));
    return complete;
  } else {
    if (buffer.at(buffer.length() - 1) == '$') {
      result.push_back(buffer.left(buffer.length() - 1));
      buffer.remove(0, buffer.length() - 1);
    } else {
      result.push_back(buffer);
      buffer.clear();
    }
    return incomplete;
  }
}

NewSerialParser::readResult NewSerialParser::bufferPullBeforeSemicolon(QByteArray &result, bool removeNewline) {
  int end;
  delimiter ending = none;
  if (buffer.contains(';')) {
    ending = semicolon;
    end = buffer.indexOf(';');
  }
  if (buffer.contains("$$")) {
    int newEnd = buffer.indexOf("$$");
    if (ending == none || newEnd < end) {
      end = newEnd;
      ending = dollar;
    }
  }
  if (ending == none)
    return incomplete;
  result.push_back(buffer.left(end));
  if (ending == semicolon) {
    buffer.remove(0, end + 1);
    if (removeNewline && !buffer.isEmpty())
      if (buffer.at(0) == '\n')
        buffer.remove(0, 1);
    return complete;
  }
  buffer.remove(0, end);
  return notProperlyEnded;
}

NewSerialParser::readResult NewSerialParser::bufferPullBeforeNull(QByteArray &result) {
  int end;
  bool ended = false;
  if (buffer.contains('\0')) {
    ended = true;
    end = buffer.indexOf('\0');
  }

  if (!ended)
    return incomplete;

  result.push_back(buffer.left(end));
  buffer.remove(0, end + 1);
  return complete;
}

void NewSerialParser::printUnknownToTerminalTimerSlot() {
  if (printUnknownToTerminal == puttPending) {
    printUnknownToTerminal = puttYesLF;
    if (!printUnknownToTerminalBuffer.isEmpty())
      emit sendTerminal(printUnknownToTerminalBuffer);
  }
  if (!printUnknownToTerminalBuffer.isEmpty())
    sendMessageIfAllowed(tr("Unknown"), printUnknownToTerminalBuffer, MessageLevel::info);
  printUnknownToTerminalBuffer.clear();
}

NewSerialParser::readResult NewSerialParser::bufferPullChannel(QPair<ValueType, QByteArray> &result) {
  int prefixLength = 0;
  ValueType valType = readValuePrefix(buffer, prefixLength);

  if (valType.type == ValueType::Type::invalid)
    throw(tr("Invalid value type: %1").arg(QString(buffer.left(prefixLength))));
  if (valType.type == ValueType::Type::incomplete)
    return incomplete;

  if ((uint32_t)buffer.length() < channelLength * valType.bytes + prefixLength + 1)
    return incomplete;

  buffer.remove(0, prefixLength);
  result = QPair<ValueType, QByteArray>(valType, buffer.left(channelLength * valType.bytes));
  buffer.remove(0, channelLength * valType.bytes);
  if (buffer.at(0) != ';')
    return notProperlyEnded;
  buffer.remove(0, 1);
  return complete;
}

void NewSerialParser::changeMode(DataMode::enumDataMode mode, DataMode::enumDataMode previousMode, QByteArray modeName) {
  if (mode == previousMode)
    return;
  if (printUnknownToTerminal == puttPending && mode != DataMode::unknown)
    printUnknownToTerminal = puttNo;
  currentMode = mode;
  sendMessageIfAllowed(tr("New data"), modeName, MessageLevel::info);
}
