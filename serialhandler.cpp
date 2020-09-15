#include "serialhandler.h"

SerialHandler::SerialHandler(Settings *in_settings) {
  this->settings = in_settings;
  this->serial = new QSerialPort(this);
  this->buffer = new Buffer();
  connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
  connect(serial, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(serialErrorOccurred()));
  connect(buffer, SIGNAL(newEntry()), this, SLOT(readBuffer()));
}

SerialHandler::~SerialHandler() {
  delete serial;
  delete buffer;
}

bool SerialHandler::connectSerial(int portIndex, long baud) {
  serial->setPortName(portList.at(portIndex));
  serial->setBaudRate(baud);
  serial->setDataBits(QSerialPort::Data8);
  serial->setParity(QSerialPort::NoParity);
  serial->setStopBits(QSerialPort::OneStop);
  serial->setFlowControl(QSerialPort::NoFlowControl);
  serial->open(QIODevice::ReadWrite);
  return serial->isOpen();
}

QStringList SerialHandler::refresh() {
  QStringList portsWithDescription;
  portList.clear();
  foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
    portsWithDescription.append(serialPortInfo.portName() + " - " + serialPortInfo.description());
    portList.append(serialPortInfo.portName());
  }
  return portsWithDescription;
}

void SerialHandler::parseBinaryDataHeader(QByteArray data) {
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

void SerialHandler::write(QByteArray data) {
  if (serial->isOpen())
    serial->write(data, data.length());
}

void SerialHandler::disconnectSerial() {
  if (serial->isOpen())
    serial->close();
}

void SerialHandler::readData() { buffer->add(serial->readAll()); }

void SerialHandler::readBuffer() {
  QPair<bool, QByteArray> line = buffer->next();
  emit newProcessedCommand(line);
  if (line.first == false) {
    if (settings->dataMode == DATA_MODE_DATA_STRING)
      emit newDataString(line.second);
    if (settings->dataMode == DATA_MODE_MESSAGE_INFO)
      emit printMessage(line.second, false);
    if (settings->dataMode == DATA_MODE_MESSAGE_WARNING)
      emit printMessage(line.second, true);
    if (settings->dataMode == DATA_MODE_TERMINAL)
      emit printToTerminal(line.second);
    if (settings->dataMode == DATA_MODE_DATA_BINARY)
      emit newDataBin(line.second);
  } else {
    if (line.second == "data")
      emit changedMode(DATA_MODE_DATA_STRING);
    else if (line.second == "info")
      emit changedMode(DATA_MODE_MESSAGE_INFO);
    else if (line.second == "warning")
      emit changedMode(DATA_MODE_MESSAGE_WARNING);
    else if (line.second == "terminal")
      emit changedMode(DATA_MODE_TERMINAL);
    else if (line.second.left(3) == "bin") {
      emit changedMode(DATA_MODE_DATA_BINARY);
      parseBinaryDataHeader(line.second);
    }
  }
}

void SerialHandler::serialErrorOccurred() {
  disconnectSerial();
  emit serialErrorOccurredSignal();
}
