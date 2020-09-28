#include "serialparser.h"

SerialParser::SerialParser() {}

SerialParser::~SerialParser() { qDebug() << "SerialParser deleted"; }

void SerialParser::init() {}

void SerialParser::parseBinaryDataHeader(QByteArray data) {
  if (modeChangeAllowed) {
    QByteArrayList list = data.split(',');
    BinDataSettings_t newSettings = binDataSettings;
    // bin,bits,chNum,maxVal,minVal,time,firstch
    if (list.length() >= 2)
      if (list.at(1).length() > 0)
        newSettings.bits = list.at(1).toInt();
    if (list.length() >= 3)
      if (list.at(2).length() > 0)
        newSettings.numCh = list.at(2).toInt();
    if (list.length() >= 4)
      if (list.at(3).length() > 0)
        newSettings.valueMax = list.at(3).toDouble();
    if (list.length() >= 5)
      if (list.at(4).length() > 0)
        newSettings.valueMin = list.at(4).toDouble();
    if (list.length() >= 6)
      if (list.at(5).length() > 0)
        newSettings.timeStep = list.at(5).toDouble();
    if (list.length() >= 7)
      if (list.at(6).length() > 0)
        newSettings.firstCh = list.at(6).toInt();

    binDataSettings = newSettings;
    emit changedBinSettings(newSettings);
  }
}

void SerialParser::changeMode(int mode) {
  if (modeChangeAllowed) {
    dataMode = mode;
    emit changedDataMode(mode);
  }
}

void SerialParser::parseLine(QByteArray line) {
  if (line.left(CMD_BEGIN_LENGTH) == CMD_BEGIN) {
    line = line.mid(CMD_BEGIN_LENGTH, line.length() - CMD_END_LENGTH - CMD_BEGIN_LENGTH);
    if (line == "data")
      changeMode(DATA_MODE_DATA_STRING);
    else if (line == "info")
      changeMode(DATA_MODE_MESSAGE_INFO);
    else if (line == "warning")
      changeMode(DATA_MODE_MESSAGE_WARNING);
    else if (line == "terminal")
      changeMode(DATA_MODE_TERMINAL);
    else if (line.left(3) == "bin") {
      changeMode(DATA_MODE_DATA_BINARY);
      parseBinaryDataHeader(line);
    }
  } else {
    if (line.endsWith(TIMEOUT_SYMBOL))
      line = line.left(line.length() - TIMEOUT_SYMBOL_LENGTH);
    if (line.endsWith(CMD_END))
      line = line.left(line.length() - CMD_END_LENGTH);
    if (dataMode == DATA_MODE_DATA_STRING)
      emit newDataString(line);
    if (dataMode == DATA_MODE_MESSAGE_INFO)
      emit printMessage(line, false);
    if (dataMode == DATA_MODE_MESSAGE_WARNING)
      emit printMessage(line, true);
    if (dataMode == DATA_MODE_TERMINAL)
      emit printToTerminal(line);
    if (dataMode == DATA_MODE_DATA_BINARY)
      emit newDataBin(line, binDataSettings);
  }
}

void SerialParser::setBinParameters(BinDataSettings_t settings) {
  binDataSettings = settings;
  emit changedBinSettings(this->binDataSettings);
}

void SerialParser::setMode(int mode) {
  dataMode = mode;
  emit changedDataMode(this->dataMode);
}
