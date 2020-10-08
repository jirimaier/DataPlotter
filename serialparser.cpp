#include "serialparser.h"

SerialParser::SerialParser() { qDebug() << "SerialParser created from " << QThread::currentThreadId(); }

SerialParser::~SerialParser() { qDebug() << "SerialParser deleted from " << QThread::currentThreadId(); }

void SerialParser::init() { qDebug() << "SerialParser initialised from " << QThread::currentThreadId(); }

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

    if (newSettings.bits <= 0) {
      emit parseError((tr("Invalid settings: ") + QString("bits = %1").arg(newSettings.bits)).toUtf8());
      newSettings.bits = binDataSettings.bits;
    }
    if (newSettings.firstCh < 1 || newSettings.firstCh > CHANNEL_COUNT) {
      emit parseError((tr("Invalid settings: ") + QString("firstCh = %1").arg(newSettings.bits)).toUtf8());
      newSettings.firstCh = binDataSettings.firstCh;
    }
    if (newSettings.numCh < 1 || newSettings.numCh > CHANNEL_COUNT - newSettings.firstCh + 1) {
      emit parseError((tr("Invalid settings: ") + QString("bits = %1").arg(newSettings.bits)).toUtf8());
      newSettings.numCh = binDataSettings.numCh;
    }
    if (newSettings.timeStep <= 0) {
      emit parseError((tr("Invalid settings: ") + QString("bits = %1").arg(newSettings.bits)).toUtf8());
      newSettings.timeStep = binDataSettings.timeStep;
    }
    if (newSettings.valueMax < newSettings.valueMin) {
      double min = newSettings.valueMin;
      newSettings.valueMin = newSettings.valueMax;
      newSettings.valueMax = min;
    }

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

void SerialParser::parseLine(QByteArray line, int type) {
  if (type == DataLineType::command) {
    if (line == "data")
      changeMode(DataMode::string);
    else if (line == "info")
      changeMode(DataMode::info);
    else if (line == "unknown")
      changeMode(DataMode::unknown);
    else if (line == "warning")
      changeMode(DataMode::warning);
    else if (line == "terminal")
      changeMode(DataMode::terminal);
    else if (line == "settings")
      changeMode(DataMode::settings);
    else if (line.left(3) == "bin") {
      changeMode(DataMode::binData);
      parseBinaryDataHeader(line);
    } else
      emit parseError((tr("Invalid data type: ")).toUtf8() + line);
  }
  if (type == DataLineType::dataEnded || type == DataLineType::dataTimeouted || type == DataLineType::dataImplicitEnded) {
    if (dataMode == DataMode::string)
      emit newDataString(line);
    if (dataMode == DataMode::info)
      emit printMessage(line, false);
    if (dataMode == DataMode::warning)
      emit printMessage(line, true);
    if (dataMode == DataMode::terminal)
      emit printToTerminal(line);
    if (dataMode == DataMode::binData)
      emit newDataBin(line, binDataSettings);
    if (dataMode == DataMode::settings)
      emit changeGUISettings(line);
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
