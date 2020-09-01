#include "serialHandler.h"

#include "buffer.h"
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

SerialHandler::SerialHandler() {
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
  if (list.length() != 7)
    return;
  emit changedBitMode(list.at(1).toInt(), list.at(2).toDouble(), list.at(3).toDouble(), list.at(4).toDouble(), list.at(5).toInt(), list.at(6).toInt());
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
    if (mode == DATA_MODE_DATA_STRING)
      emit newDataString(line.second);
    if (mode == DATA_MODE_MESSAGE_INFO)
      emit printMessage(line.second, false);
    if (mode == DATA_MODE_MESSAGE_WARNING)
      emit printMessage(line.second, true);
    if (mode == DATA_MODE_TERMINAL)
      emit printToTerminal(line.second);
    if (mode == DATA_MODE_DATA_BINARY)
      emit newDataBin(line.second, bits, valueMin, valueMax, timeStep, numCh, firstCh);
  } else {
    if (line.second == "data")
      emit changedMode(DATA_MODE_DATA_STRING);
    else if (line.second == "info")
      emit changedMode(DATA_MODE_MESSAGE_INFO);
    else if (line.second == "warning")
      emit changedMode(DATA_MODE_MESSAGE_WARNING);
    else if (line.second == "terminal")
      emit changedMode(DATA_MODE_TERMINAL);
    else if (line.second.left(4) == "data") {
      emit changedMode(DATA_MODE_DATA_BINARY);
      parseBinaryDataHeader(line.second);
    }
  }
}

void SerialHandler::serialErrorOccurred() {
  disconnectSerial();
  emit serialErrorOccurredSignal();
}
