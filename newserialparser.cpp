//  Copyright (C) 2020-2021  Jiří Maier

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

NewSerialParser::~NewSerialParser() {}

void NewSerialParser::resetChHeader() {
  channelHeaderRead = false;
  channelLength = 0;
  channelNumber = 0;
  channelTime.second.clear();
  aditionalHeaderParameters.clear();
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
    emit sendMessage("Buffer (string)", pendingDataBuffer + buffer, MessageLevel::info, target);
    emit sendMessage("Buffer (hex)", pendingDataBuffer + buffer.toHex(' '), MessageLevel::info, target);
  }
  if (!pendingPointBuffer.isEmpty()) {
    QByteArray pointBuffer;
    emit sendMessage("Point buffer content", pointBuffer, MessageLevel::info, target);
    foreach (auto line, pendingPointBuffer)
      emit sendMessage("->" + valueTypeToString(line.first).toUtf8(), (line.first.isBinary ? line.second.toHex() : line.second), MessageLevel::info, target);
  }
}

void NewSerialParser::getReady() {
  clearBuffer();
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
      if (buffer.right(1) == "$")
        break;
      if (buffer.length() >= 3)
        if (buffer.left(2) == "$$") {
          if (currentMode == DataMode::info || currentMode == DataMode::warning)
            emit sendDeviceMessage("", false, true); // Pokud byl předchozí režim výpis zprávy, ohlásí její konec
          parseMode(buffer.at(2));
          buffer.remove(0, 3);
          continue;
        }

      if (currentMode == DataMode::point) {
        readResult result = bufferReadPoint(pendingPointBuffer);
        if (result == complete) {
          if (!pendingPointBuffer.isEmpty()) {
            emit sendPoint(pendingPointBuffer);
            pendingPointBuffer.clear();
            continue;
          }
        }
        if (result == notProperlyEnded) {
          if (!pendingPointBuffer.isEmpty()) {
            sendMessageIfAllowed(tr("Missing semicolumn ?"), pendingPointBuffer.last().second, MessageLevel::warning);
            emit sendPoint(pendingPointBuffer);
            pendingPointBuffer.clear();
            continue;
          }
        }
        break;
      }

      if (currentMode == DataMode::unknown) {
        QByteArray dropped;
        readResult result = bufferPullFull(dropped);
        if (!dropped.isEmpty())
          sendMessageIfAllowed(tr("Unknown"), dropped, MessageLevel::info);
        if (result == complete)
          continue;
        break;
      }

      if (currentMode == DataMode::channel) {
        if (!channelHeaderRead) {
          readResult result = bufferReadPoint(pendingPointBuffer);
          if (result == complete) {
            if (pendingPointBuffer.length() >= 3 && pendingPointBuffer.length() <= 7) {
              channelHeaderRead = true;
              try {
                channelNumber = arrayToUint(pendingPointBuffer.at(0));
                if (channelNumber == 0 || channelNumber > ANALOG_COUNT)
                  throw(tr("out of range (1 - %1): %2").arg(ANALOG_COUNT).arg(channelNumber));
              } catch (QString msg) {
                throw(tr("Invallid channel: ") + tr("Invalid channel number - ") + msg);
              }

              channelTime = pendingPointBuffer.at(1);

              try {
                channelLength = arrayToUint(pendingPointBuffer.at(2));
              } catch (QString msg) {
                throw(tr("Invallid channel: ") + tr("Invalid channel length - ") + msg);
              }

              for (int i = 3; i < pendingPointBuffer.length(); i++) {
                aditionalHeaderParameters.append(pendingPointBuffer.at(i));
              }

              pendingPointBuffer.clear();

            } else
              throw(tr("Invallid channel: ") + tr("Wrong header length (%1 entries)").arg(pendingPointBuffer.length()));
          }
          if (result == incomplete)
            break;
          if (result == notProperlyEnded)
            throw(tr("Invallid channel: ") + tr("Header not properly ended"));
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
            QByteArray semicolumPositionMessage = tr("No semicolum found").toUtf8();
            if (buffer.contains(';') && channel.second.contains(';'))
              semicolumPositionMessage = tr("There are semicolums %1 byte before and %2 after end.").arg(buffer.indexOf(';')).arg(channel.second.length() - channel.second.lastIndexOf(';')).toUtf8();
            else {
              if (channel.second.contains(';'))
                semicolumPositionMessage = tr("There is semicolum %1 bytes before end.").arg(channel.second.length() - channel.second.lastIndexOf(';')).toUtf8();
              if (buffer.contains(';'))
                semicolumPositionMessage = tr("There is semicolum %1 bytes after end.").arg(buffer.indexOf(';')).toUtf8();
            }
            sendMessage(tr("Channel not ended with ';'"), semicolumPositionMessage, MessageLevel::warning, target);
          }

          int zeroIndex = 0, channelBits = channel.first.bytes * 8;
          QPair<ValueType, QByteArray> channelMin, channelMax;

          if (channel.first.type == ValueType::Type::floatingpoint) {
            if (!aditionalHeaderParameters.isEmpty()) {
              if (aditionalHeaderParameters.size() == 1) {
                try {
                  zeroIndex = arrayToUint(aditionalHeaderParameters.first());
                  pendingPointBuffer.clear();
                } catch (QString msg) {
                  throw(tr("Invallid channel: ") + tr("Invalid zero position - ") + msg);
                }
              } else
                throw(tr("Invallid channel: ") + tr("To many header entries for floating point type"));
            }
          } else if (channel.first.type == ValueType::Type::unsignedint) {
            if (!aditionalHeaderParameters.isEmpty()) {
              try {
                channelBits = arrayToUint(aditionalHeaderParameters.first());
              } catch (QString msg) {
                throw(tr("Invallid channel: ") + tr("Invalid number of bits - ") + msg);
              }
            }
            if (aditionalHeaderParameters.length() == 2)
              channelMax = aditionalHeaderParameters.at(1);
            if (aditionalHeaderParameters.length() > 2) {
              channelMin = aditionalHeaderParameters.at(1);
              channelMax = aditionalHeaderParameters.at(2);
            }
            if (aditionalHeaderParameters.length() == 4) {
              try {
                zeroIndex = arrayToUint(aditionalHeaderParameters.at(3));
                pendingPointBuffer.clear();
              } catch (QString msg) {
                throw(tr("Invallid channel: ") + tr("Invalid zero position - ") + msg);
              }
            }
            // Delší by neprošlo kontrolou při čtení záhlaví
          } else if (channel.first.type == ValueType::Type::floatingpoint) {
            if (!aditionalHeaderParameters.isEmpty()) {
              if (aditionalHeaderParameters.size() == 1) {
                try {
                  zeroIndex = arrayToUint(aditionalHeaderParameters.first());
                  pendingPointBuffer.clear();
                } catch (QString msg) {
                  throw(tr("Invallid channel: ") + tr("Invalid zero position - ") + msg);
                }
              } else
                throw(tr("Invallid channel: ") + tr("To many header entries for signed integer type"));
            }
          } else if (channel.first.type == ValueType::Type::floatingpoint) {
            if (!aditionalHeaderParameters.isEmpty()) {
              if (aditionalHeaderParameters.size() == 1) {
                try {
                  zeroIndex = arrayToUint(aditionalHeaderParameters.first());
                  pendingPointBuffer.clear();
                } catch (QString msg) {
                  throw(tr("Invallid channel: ") + tr("Invalid zero position - ") + msg);
                }
              } else
                throw(tr("Invallid channel: ") + tr("To many header entries for floating point type"));
            }
          }
          emit sendChannel(channel, channelNumber, channelTime, zeroIndex, channelBits, channelMin, channelMax);

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
        emit sendEcho(data);
        if (result == complete)
          continue;
        break;
      }

      if (currentMode == DataMode::settings) {
        readResult result = bufferPullBeforeSemicolumn(pendingDataBuffer, true);
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
            sendMessageIfAllowed(tr("Missing semicolumn ?"), pendingDataBuffer, MessageLevel::warning);
            pendingDataBuffer.clear();
            continue;
          }
        }
        break;
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
    if (IS_NUMERIC_CHAR(buffer.at(0))) {
      // Hledám který znak je nejblýž: , ; nebo $
      // Pokud se nevyskytuje, funkce vrátí -1, což se v unsigned int zmení na max hodnotu int a tedy rozhodně nebude nejblýže
      ValueType valType;
      valType.isBinary = false;
      unsigned int comma = buffer.indexOf(',');
      unsigned int semicolon = buffer.indexOf(';');
      unsigned int dollar = buffer.indexOf('$');
      if (comma < semicolon && comma < dollar) {
        // Nejblýže je čárka
        result.append(QPair<ValueType, QByteArray>(valType, buffer.left(comma)));
        buffer.remove(0, comma + 1);
        continue;
      } else if (semicolon < comma && semicolon < dollar) {
        result.append(QPair<ValueType, QByteArray>(valType, buffer.left(semicolon)));
        buffer.remove(0, semicolon + 1);
        return complete;
      } else if (dollar < comma && dollar < semicolon) {
        result.append(QPair<ValueType, QByteArray>(valType, buffer.left(dollar)));
        buffer.remove(0, dollar);
        return notProperlyEnded;
      } else
        // Žádné není nejmenší (jsou si rovny a jsou na maximu, tedy znak není)
        return incomplete;
    }

    else {
      // Binární data
      if (buffer.length() == 1) {
        if (buffer.at(0) == ';') {
          // Konec bodu
          buffer.remove(0, 1);
          return complete;
        } else
          // V bufferu není celý bod
          return incomplete;
      }

      int prefixLength = 0;
      ValueType valType = readValuePrefix(buffer, &prefixLength);
      if (valType.type == ValueType::Type::invalid)
        throw(tr("Invalid value type: %1").arg(QString(buffer.left(prefixLength))));
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
      throw(tr("Value is not a valid integer: %1").arg(QString(value.second)));
    if (val < 0)
      throw(tr("Value is negative: %1").arg(QString(value.second)));
    return val;
  }
  if (value.first.type != ValueType::Type::unsignedint)
    throw(QString("Value is not unsigned integer type"));
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
    if (value.first.bytes == 1) {
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
  else if (modeIdent == 'S')
    changeMode(DataMode::settings, previousMode, tr("Settings").toUtf8());
  else if (modeIdent == 'U')
    changeMode(DataMode::unknown, previousMode, tr("Unknown").toUtf8());
  else if (modeIdent == 'E')
    changeMode(DataMode::echo, previousMode, tr("Echo").toUtf8());
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
    result.push_back(buffer);
    buffer.clear();
    return incomplete;
  }
}

NewSerialParser::readResult NewSerialParser::bufferPullBeforeSemicolumn(QByteArray &result, bool removeNewline) {
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

NewSerialParser::readResult NewSerialParser::bufferPullChannel(QPair<ValueType, QByteArray> &result) {
  int prefixLength = 0;
  ValueType valType = readValuePrefix(buffer, &prefixLength);

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
  currentMode = mode;
  sendMessageIfAllowed(tr("Type of data changed"), modeName, MessageLevel::info);
}
