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

#include "serialreader.h"

SerialReader::SerialReader(QObject* parent) : QObject(parent) {}

SerialReader::~SerialReader() {
  if (serial->isOpen())
    serial->close();
  delete serial;
}

void SerialReader::init() {
  // QSerialPrort musí být vytvořen tady (fungkce init zavolána po spuštění vlákna), ne v konstruktoru, protože pak by SerialPort byl v GUI vláknu.
  serial = new QSerialPort;
  connect(serial, &QSerialPort::bytesWritten, this, &SerialReader::finishedWriting);
  // V starším Qt (Win XP) není signál pro error
#if QT_VERSION >= 0x050800
  connect(serial, &QSerialPort::errorOccurred, this, &SerialReader::errorOccurred);
#endif
}

void SerialReader::begin(QString portName, int baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControll) {
  if (serial->isOpen())
    end(); // Pokud je port otevřen, tak ho zavře
  serial->setPortName(portName);
  serial->setBaudRate(baudRate);
  serial->setDataBits(dataBits);
  serial->setParity(parity);
  serial->setStopBits(stopBits);
  serial->setFlowControl(flowControll);
  if (serial->open(QIODevice::ReadWrite)) {
    emit connectionResult(true, tr("Connected"), "");
  } else if (serial->open(QIODevice::ReadOnly)) {
    emit connectionResult(true, tr("Read only"), tr("Connected as read only"));
  } else {
    auto error = serial->error();

    QString errorText;
    if (error == QSerialPort::PermissionError)
      errorText = tr("Access denied");
    else
      errorText = tr("Error");

    emit connectionResult(false, errorText, serial->errorString());
  }
  if (serial->isOpen()) {
    serial->clear();
    emit started(); // Parser si vymaže buffer a odpoví že je připraven, teprve po odpovědi se začnou číst data.
  }
}

void SerialReader::write(QByteArray data) {
  if (serial->isOpen())
    serial->write(data);
}

void SerialReader::parserReady() { connect(serial, &QSerialPort::readyRead, this, &SerialReader::read); }

void SerialReader::changeBaud(qint32 baud) {
  if (!serial->isOpen())
    return;

  serial->close();
  disconnect(serial, &QSerialPort::readyRead, this, &SerialReader::read);

  serial->setBaudRate(baud);
  if (!serial->open(QIODevice::ReadWrite)) {
    auto error = serial->error();

    QString errorText;
    if (error == QSerialPort::PermissionError)
      errorText = tr("Access denied");
    else
      errorText = tr("Error");

    emit connectionResult(false, errorText, serial->errorString());
  }
  if (serial->isOpen()) {
    serial->clear();
    emit started(); // Parser si vymaže buffer a odpoví že je připraven, teprve po odpovědi se začnou číst data.
  }
}

void SerialReader::end() {
  disconnect(serial, &QSerialPort::readyRead, this, &SerialReader::read);
  emit connectionResult(false, tr("Not connected"), "");
  if (!serial->isOpen())
    return;
  serial->close();
}

void SerialReader::errorOccurred() {
  auto error = serial->error();

  if (error == QSerialPort::NoError)
    return;

  QString errorText;
  if (error == QSerialPort::PermissionError)
    errorText = tr("Access denied");
  else
    errorText = tr("Error");

  disconnect(serial, &QSerialPort::readyRead, this, &SerialReader::read);

  if (serial->isOpen()) {
    serial->close();
  }

  emit connectionResult(false, errorText, serial->errorString());
}

void SerialReader::toggle(QString portName, int baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControll) {
  if (!serial->isOpen())
    begin(portName, baudRate, dataBits, parity, stopBits, flowControll);
  else
    end();
}

void SerialReader::read() {
  if (serialMonitor) {
    QByteArray newdata = serial->readAll();
    emit sendData(newdata);
    emit monitor(newdata);
  } else
    emit sendData(serial->readAll());
}
