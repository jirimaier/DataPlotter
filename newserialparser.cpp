#include "newserialparser.h"

NewSerialParser::NewSerialParser(QObject *parent) : QObject(parent) {
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

void NewSerialParser::fatalError(const char *header, QByteArray &message) {
  sendMessageIfAllowed((tr("Fatal Error") + ", " + QString(header)).toUtf8(), message, MessageLevel::error);
  sendMessageIfAllowed(tr("In part").toUtf8(), bufferDebug, MessageLevel::warning);
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

void NewSerialParser::sendMessageIfAllowed(const char *header, QByteArray &message, MessageLevel::enumerator type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message, type);
}

void NewSerialParser::sendMessageIfAllowed(const char *header, QString &message, MessageLevel::enumerator type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message.toUtf8(), type);
}

void NewSerialParser::showBuffer() {
  QByteArray pointBuffer;
  foreach (QByteArray line, pendingPointBuffer)
    pointBuffer.append(" " + line);
  emit sendMessage("Buffer content", "", MessageLevel::error);
  emit sendMessage("-> Main buffer", buffer, MessageLevel::info);
  emit sendMessage("-> Point buffer", pointBuffer, MessageLevel::info);
  emit sendMessage("-> Segment buffer", pendingDataBuffer, MessageLevel::info);
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
  buffer.append(newData);
  while (!buffer.isEmpty()) {
    bufferDebug = buffer;
    if (buffer.right(1) == "$")
      break;
    if (buffer.length() >= 3)
      if (buffer.left(2) == "$$") {
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
          sendMessageIfAllowed(tr("Missing semicolumn ?").toUtf8(), pendingPointBuffer.last(), MessageLevel::warning);
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
        sendMessageIfAllowed("Unknown", dropped, MessageLevel::info);
      if (result == complete)
        continue;
      break;
    }

    if (currentMode == DataMode::channel) {
      if (!channelHeaderOK) {
        readResult result = bufferPullPoint(pendingPointBuffer);
        if (result == complete)
          if (pendingPointBuffer.length() == 3) {
            channelHeaderOK = true;
            QByteArray chnum = pendingPointBuffer.at(0);
            channelTime = pendingPointBuffer.at(1);
            QByteArray lengthBytes = pendingPointBuffer.at(2);
            pendingPointBuffer.clear();
            bool isok;
            unsigned int ch = arrayToUint(chnum, isok);
            if (ch > CHANNEL_COUNT || ch == 0)
              isok = false;
            if (!isok) {
              fatalError("invalid channel number", chnum);
              resetChHeader();
              continue;
            }
            channelLength = arrayToUint(lengthBytes, isok);
            if (!isok) {
              fatalError("invalid channel length", lengthBytes);
              resetChHeader();
              continue;
            }
            continue;
          }
        if (result == incomplete)
          break;
        if (result == notProperlyEnded) {
          fatalError("Invalid channel header", buffer);
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
        emit sendChannel(channel, channelNumber, channelTime);
        pendingPointBuffer.clear();
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
          emit sendSettings(pendingDataBuffer);
          pendingDataBuffer.clear();
          continue;
        }
      }
      if (result == notProperlyEnded) {
        if (!pendingDataBuffer.isEmpty()) {
          emit sendSettings(pendingDataBuffer);
          sendMessageIfAllowed(tr("Missing semicolumn ?").toUtf8(), pendingDataBuffer, MessageLevel::warning);
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
    QByteArray character = QString(modeChar).toUtf8();
    sendMessageIfAllowed("Unknown mode", character, MessageLevel::error);
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
  if ((unsigned long)buffer.length() < channelLength + 2)
    return incomplete;
  result = buffer.left(channelLength + 2);
  buffer.remove(0, channelLength + 2);
  return complete;
}

void NewSerialParser::changeMode(DataMode::enumerator mode, DataMode::enumerator previousMode, QByteArray modeName) {
  if (mode == previousMode)
    return;
  currentMode = mode;
  sendMessageIfAllowed("Mode changed", modeName, MessageLevel::info);
}
