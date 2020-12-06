#include "newserialparser.h"

NewSerialParser::NewSerialParser(MessageTarget::enumerator target, QObject *parent) : QObject(parent) {
  this->target = target;
  qDebug() << "SerialParser created from " << QThread::currentThreadId();
  resetChHeader();
}

NewSerialParser::~NewSerialParser() { qDebug() << "SerialParser deleted from " << QThread::currentThreadId(); }

void NewSerialParser::init() { qDebug() << "SerialParser initialised from " << QThread::currentThreadId(); }

void NewSerialParser::resetChHeader() {
  channelHeaderOK = false;
  channelLength = 0;
  channelNumber = 0;
  channelTime = "";
}

void NewSerialParser::fatalError(QString header, QByteArray &message) {
  sendMessageIfAllowed((tr("Error, ") + header), message, MessageLevel::error);
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
  changeMode(DataMode::unknown, currentMode, "Unknown");
}

void NewSerialParser::sendMessageIfAllowed(QString header, QByteArray &message, MessageLevel::enumerator type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message, type, target);
}

void NewSerialParser::sendMessageIfAllowed(QString header, QString &message, MessageLevel::enumerator type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message.toUtf8(), type, target);
}

void NewSerialParser::showBuffer() {
  QByteArray pointBuffer;
  foreach (QByteArray line, pendingPointBuffer)
    pointBuffer.append(" " + line);
  emit sendMessage("Buffer content", "", MessageLevel::error, target);
  emit sendMessage("-> Main buffer", buffer, MessageLevel::info, target);
  emit sendMessage("-> Point buffer", pointBuffer, MessageLevel::info, target);
  emit sendMessage("-> Segment buffer", pendingDataBuffer, MessageLevel::info, target);
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
  changeMode(DataMode::unknown, currentMode, "Unknown");
  resetChHeader();
}

void NewSerialParser::parse(QByteArray newData) {
  buffer.push_back(newData);
  while (!buffer.isEmpty()) {
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
      readResult result = bufferPullPoint(pendingPointBuffer);
      if (result == complete) {
        if (!pendingPointBuffer.isEmpty()) {
          emit sendPoint(pendingPointBuffer);
          pendingPointBuffer.clear();
          continue;
        }
      }
      if (result == notProperlyEnded) {
        if (!pendingPointBuffer.isEmpty()) {
          sendMessageIfAllowed(tr("Missing semicolumn ?"), pendingPointBuffer.last(), MessageLevel::warning);
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
      if (!channelHeaderOK) {
        readResult result = bufferPullPoint(pendingPointBuffer);
        if (result == complete) {
          if (pendingPointBuffer.length() == 3) {
            channelHeaderOK = true;
            QByteArray chnum = pendingPointBuffer.at(0);
            channelTime = pendingPointBuffer.at(1);
            QByteArray lengthBytes = pendingPointBuffer.at(2);
            pendingPointBuffer.clear();
            bool isok;
            channelNumber = arrayToUint(chnum, isok);
            if (channelNumber > ANALOG_COUNT || channelNumber == 0)
              isok = false;
            if (!isok) {
              fatalError(tr("invalid channel number"), chnum);
              resetChHeader();
              continue;
            }
            channelLength = arrayToUint(lengthBytes, isok);
            if (!isok) {
              fatalError(tr("invalid channel length"), lengthBytes);
              resetChHeader();
              continue;
            }
            continue;
          } else {
            fatalError(tr("Invalid channel header"), buffer);
            if (!pendingPointBuffer.isEmpty())
              pendingPointBuffer.clear();
          }
        }
        if (result == incomplete)
          break;
        if (result == notProperlyEnded) {
          fatalError(tr("Invalid channel header"), buffer);
          if (!pendingPointBuffer.isEmpty())
            pendingPointBuffer.clear();
          resetChHeader();
          continue;
        }
      }
      if (channelHeaderOK) {
        QByteArray channel;
        readResult result = bufferPullChannel(channel);
        if (result == incomplete)
          break;
        if (result == notProperlyEnded) {
          QByteArray semicolumPositionMessage = tr("No semicolum found").toUtf8();
          if (buffer.contains(';') && channel.contains(';'))
            semicolumPositionMessage = tr("There are semicolums %1 byte before and %2 after end.").arg(buffer.indexOf(';')).arg(channel.length() - channel.lastIndexOf(';')).toUtf8();
          else {
            if (channel.contains(';'))
              semicolumPositionMessage = tr("There is semicolum %1 bytes before end.").arg(channel.length() - channel.lastIndexOf(';')).toUtf8();
            if (buffer.contains(';'))
              semicolumPositionMessage = tr("There is semicolum %1 bytes after end.").arg(buffer.indexOf(';')).toUtf8();
          }
          sendMessageIfAllowed(tr("Channel not ended with ';'"), semicolumPositionMessage, MessageLevel::warning);
        }
        emit sendChannel(channel, channelNumber, channelTime);
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
  }
}

NewSerialParser::readResult NewSerialParser::bufferPullPoint(QByteArrayList &result) {
  while (!buffer.isEmpty()) {
    // Textová data
    if (isdigit(buffer.at(0)) || buffer.at(0) == '-') {
      int end;
      delimiter ending = none;
      if (buffer.contains(',')) {
        ending = comma;
        end = buffer.indexOf(',');
      }
      if (buffer.contains(';')) {
        int newEnd = buffer.indexOf(';');
        if (ending == none || newEnd < end) {
          end = newEnd;
          ending = semicolon;
        }
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
      result.append(buffer.left(end));
      if (ending == comma) {
        buffer.remove(0, end + 1);
        continue;
      }
      if (ending == semicolon) {
        buffer.remove(0, end + 1);
        return complete;
      }
      if (ending == dollar) {
        buffer.remove(0, end);
        return notProperlyEnded;
      }
    }
    // Binární data
    else {
      if (buffer.length() < 2)
        return incomplete;
      bool isok;
      QByteArray numOfBytes = buffer.mid(1, 1);
      int bytes = numOfBytes.toUInt(&isok, 16);
      if (!isok) {
        fatalError("wrong value type", numOfBytes);
        return incomplete;
      }
      if (buffer.length() < 2 + bytes)
        return incomplete;
      result.append(buffer.left(2 + bytes));
      buffer.remove(0, 2 + bytes);
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

uint32_t NewSerialParser::arrayToUint(QByteArray array, bool &isok) {
  isok = true;
  // String
  if (isdigit(array.at(0))) {
    double val = array.toUInt(&isok);
    return val;
    // Little endian
  } else if (array.left(2) == "u1") { // unsigned int 8
    return ((uint8_t)array.at(2));
  } else if (array.left(2) == "u2") { // unsigned int 16
    char bytes[2] = {array.at(2), array.at(3)};
    return (*((uint16_t *)bytes));
  } else if (array.left(2) == "u4") { // unsigned int 32
    char bytes[4] = {array.at(2), array.at(3), array.at(4), array.at(5)};
    return (*((uint32_t *)bytes));
    // Big endian
  } else if (array.left(2) == "U1") { // unsigned int 8
    return ((uint8_t)array.at(2));
  } else if (array.left(2) == "U2") { // unsigned int 16
    char bytes[2] = {array.at(3), array.at(2)};
    return (*((uint16_t *)bytes));
  } else if (array.left(2) == "U4") { // unsigned int 32
    char bytes[4] = {array.at(5), array.at(4), array.at(3), array.at(2)};
    return (*((uint32_t *)bytes));
  }
  isok = false;
  return 0;
}

void NewSerialParser::parseMode(QChar modeChar) {
  DataMode::enumerator previousMode = currentMode;
  QChar modeIdent = modeChar.toUpper();
  if (modeIdent == 'P')
    changeMode(DataMode::point, previousMode, "Points");
  else if (modeIdent == 'T')
    changeMode(DataMode::terminal, previousMode, "Terminal");
  else if (modeIdent == 'I')
    changeMode(DataMode::info, previousMode, "Info");
  else if (modeIdent == 'W')
    changeMode(DataMode::warning, previousMode, "Warning");
  else if (modeIdent == 'C')
    changeMode(DataMode::channel, previousMode, "Channel");
  else if (modeIdent == 'S')
    changeMode(DataMode::settings, previousMode, "Settings");
  else if (modeIdent == 'U')
    changeMode(DataMode::unknown, previousMode, "Unknown");
  else {
    currentMode = DataMode::unknown;
    QByteArray character = QString(modeChar).toLocal8Bit();
    sendMessageIfAllowed(tr("Unknown mode"), character, MessageLevel::error);
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

NewSerialParser::readResult NewSerialParser::bufferPullChannel(QByteArray &result) {
  if (buffer.length() < 2)
    return incomplete;
  bool isok;
  QByteArray type = buffer.left(2);
  uint8_t bytesPerValue = type.right(1).toUInt(&isok, 16);
  if (!isok) {
    fatalError(tr("invalid type"), type);
    return incomplete;
  }
  if ((uint32_t)buffer.length() < channelLength * bytesPerValue + 3)
    return incomplete;
  result = buffer.left(channelLength * bytesPerValue + 2);
  buffer.remove(0, channelLength * bytesPerValue + 2);
  if (buffer.at(0) != ';')
    return notProperlyEnded;
  buffer.remove(0, 1);
  return complete;
}

void NewSerialParser::changeMode(DataMode::enumerator mode, DataMode::enumerator previousMode, QByteArray modeName) {
  if (mode == previousMode)
    return;
  currentMode = mode;
  sendMessageIfAllowed(tr("Mode changed"), modeName, MessageLevel::info);
}
