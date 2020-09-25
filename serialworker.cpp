#include "serialworker.h"
#include <QtDebug>

SerialWorker::SerialWorker(QObject *parent) : QObject(parent) {}

SerialWorker::~SerialWorker() { qDebug() << "SerialWorker deleted"; }

void SerialWorker::read() {
  QByteArrayMatcher head;
  QByteArrayMatcher tail;
  head.setPattern(CMD_BEGIN);
  tail.setPattern(CMD_END);
  int begin, end;
  buffer += serial.readAll();
  while (!buffer.isEmpty()) {
    begin = head.indexIn(buffer);
    end = tail.indexIn(buffer);
    if (begin == -1 && end == -1) {
      // if (!somethingNew) {
      // emit newData(buffer);
      // buffer.clear();
      //}
      break;
    }
    if (end != -1 && (end < begin || begin == -1)) {
      emit newData(buffer.left(end));
      buffer.remove(0, end + CMD_END_LENGHT);
      continue;
    }
    if (begin > 0) {
      emit newData(buffer.left(begin));
      buffer.remove(0, begin);
      continue;
    }
    if (begin == 0 && end == -1)
      break;
    if (begin == 0 && end > begin) {
      emit newCommand(buffer.mid(begin + CMD_BEGIN_LENGHT, end - begin - CMD_BEGIN_LENGHT));
      buffer.remove(0, end + CMD_END_LENGHT);
      continue;
    }
  }
}

void SerialWorker::begin(QString portName, int baudRate) {
  serial.setPortName(portName);
  serial.setBaudRate(baudRate);
  serial.setDataBits(QSerialPort::Data8);
  serial.setParity(QSerialPort::NoParity);
  serial.setStopBits(QSerialPort::OneStop);
  serial.setFlowControl(QSerialPort::NoFlowControl);
  serial.open(QIODevice::ReadWrite);
  emit connectionResult(serial.isOpen(), serial.isOpen() ? tr("Connected to ") + serial.portName() + tr(" at ") + QString::number(serial.baudRate()) + tr(" bps") : tr("Error: ") + serial.errorString());
  connect(&serial, &QSerialPort::readyRead, this, &SerialWorker::read);
}

void SerialWorker::end() {
  if (serial.isOpen())
    serial.close();
  emit connectionResult(false, tr("Not connected"));
}

void SerialWorker::write(QByteArray data) {}
