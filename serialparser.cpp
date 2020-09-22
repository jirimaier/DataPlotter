#include "serialparser.h"

SerialParser::SerialParser(Settings *in_settings) { this->settings = in_settings; }

SerialParser::~SerialParser() {}

void SerialParser::parseBinaryDataHeader(QByteArray data) {
  QByteArrayList list = data.split(',');
  Settings::binDataSettings_t newSettings = settings->binDataSettings;
  // bin,bits,chNum,maxVal,minVal,time,cont,firstch
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
      newSettings.continuous = list.at(6).toInt() == 1;
  if (list.length() >= 8)
    if (list.at(7).length() > 0)
      newSettings.firstCh = list.at(7).toInt();
  emit changedBinSettings(newSettings);
}

void SerialParser::parseData(QByteArray line) {
  emit newProcessedCommand(QPair<bool, QByteArray>(false, line));
  if (settings->dataMode == DATA_MODE_DATA_STRING)
    emit newDataString(line);
  if (settings->dataMode == DATA_MODE_MESSAGE_INFO)
    emit printMessage(line, false);
  if (settings->dataMode == DATA_MODE_MESSAGE_WARNING)
    emit printMessage(line, true);
  if (settings->dataMode == DATA_MODE_TERMINAL)
    emit printToTerminal(line);
  if (settings->dataMode == DATA_MODE_DATA_BINARY)
    emit newDataBin(line);
}

void SerialParser::parseCommand(QByteArray line) {
  emit newProcessedCommand(QPair<bool, QByteArray>(true, line));
  if (line == "data")
    emit changedMode(DATA_MODE_DATA_STRING);
  else if (line == "info")
    emit changedMode(DATA_MODE_MESSAGE_INFO);
  else if (line == "warning")
    emit changedMode(DATA_MODE_MESSAGE_WARNING);
  else if (line == "terminal")
    emit changedMode(DATA_MODE_TERMINAL);
  else if (line.left(3) == "bin") {
    emit changedMode(DATA_MODE_DATA_BINARY);
    parseBinaryDataHeader(line);
  }
}
