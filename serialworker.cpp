#include "serialworker.h"
#include <QtDebug>

SerialWorker::SerialWorker(QObject *parent) : QObject(parent) {}

SerialWorker::~SerialWorker() {
  delete timer;
  delete buffer;
  delete serial;
  qDebug() << "SerialWorker deleted";
}

void SerialWorker::init() {
  timer = new QTimer;
  buffer = new QByteArray;
  serial = new QSerialPort;
}

void SerialWorker::read() {
  QByteArrayMatcher head;
  QByteArrayMatcher tail;
  head.setPattern(CMD_BEGIN);
  tail.setPattern(CMD_END);
  int begin, end;
  buffer->append(serial->readAll());
  while (!buffer->isEmpty()) {
    begin = head.indexIn(*buffer);
    end = tail.indexIn(*buffer);
    if (begin == -1 && end == -1) {
      timer->start(lineTimeout);
      break;
    }
    if (end != -1 && (end < begin || begin == -1)) {
      emit newLine(buffer->left(end + CMD_END_LENGTH));
      buffer->remove(0, end + CMD_END_LENGTH);
      continue;
    }
    if (begin > 0) {
      emit newLine(buffer->left(begin));
      buffer->remove(0, begin);
      continue;
    }
    if (begin == 0 && end == -1)
      break;
    if (begin == 0 && end > begin) {
      emit newLine(buffer->mid(begin, end - begin + CMD_END_LENGTH));
      buffer->remove(0, end + CMD_END_LENGTH);
      continue;
    }
  }
}

void SerialWorker::lineTimedOut() {
  qDebug() << "Line timed out";
  if (!buffer->isEmpty()) {
    emit newLine(*buffer + "<timeout>");
    buffer->clear();
  }
}

void SerialWorker::errorOccured() {
  if (serial->isOpen())
    serial->close();
  emit connectionResult(false, tr("Error: ") + serial->errorString());
}

void SerialWorker::begin(QString portName, int baudRate) {
  connect(timer, &QTimer::timeout, this, &SerialWorker::lineTimedOut);
  connect(serial, &QSerialPort::readyRead, this, &SerialWorker::read);
  connect(serial, &QSerialPort::errorOccurred, this, &SerialWorker::errorOccured);
  serial->setPortName(portName);
  serial->setBaudRate(baudRate);
  serial->setDataBits(QSerialPort::Data8);
  serial->setParity(QSerialPort::NoParity);
  serial->setStopBits(QSerialPort::OneStop);
  serial->setFlowControl(QSerialPort::NoFlowControl);
  if (serial->open(QIODevice::ReadWrite))
    emit connectionResult(true, tr("Connected to ") + serial->portName() + tr(" at ") + QString::number(serial->baudRate()) + tr(" bps"));
  else if (serial->open(QIODevice::ReadOnly))
    emit connectionResult(true, tr("Read-only  ") + serial->portName() + tr(" at ") + QString::number(serial->baudRate()) + tr(" bps"));
  else
    emit connectionResult(false, tr("Error: ") + serial->errorString());
}

void SerialWorker::end() {
  if (serial->isOpen())
    serial->close();
  emit connectionResult(false, tr("Not connected"));
}

void SerialWorker::write(QByteArray data) {}
