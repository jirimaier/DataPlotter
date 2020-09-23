#include "serialthread.h"
#include <QDebug>

SerialThread::SerialThread() {
  head.setPattern(CMD_BEGIN);
  tail.setPattern(CMD_END);
}

SerialThread::~SerialThread() {}

void SerialThread::begin(QString portName, int baudRate) {
  const QMutexLocker locker(&mutex);
  port = portName;
  baud = baudRate;
  start();
}

void SerialThread::end() {
  const QMutexLocker locker(&mutex);
  qDebug() << "closed";
}

void SerialThread::run() {

  mutex.lock();
  serial.setPortName(port);
  serial.setBaudRate(baud);
  mutex.unlock();
  serial.setDataBits(QSerialPort::Data8);
  serial.setParity(QSerialPort::NoParity);
  serial.setStopBits(QSerialPort::OneStop);
  serial.setFlowControl(QSerialPort::NoFlowControl);
  serial.open(QIODevice::ReadWrite);
  emit connectionResult(serial.isOpen(), serial.isOpen() ? tr("Connected to ") + serial.portName() + tr(" at ") + QString::number(serial.baudRate()) + tr(" bps") : tr("Error: ") + serial.errorString());
  connect(&serial, &QSerialPort::readyRead, this, &SerialThread::readyRead);
  // exec();
  // serial.close();
  // emit connectionResult(false, tr("Not connected"));
}

void SerialThread::readyRead() {
  char buffer[1000];
  serial.read(buffer, 1000);
  qDebug() << buffer;
  /*buffer += ;
  forever {
    int begin = head.indexIn(buffer);
    int end = tail.indexIn(buffer);
    if (begin == -1 && end == -1) {
      continue;
      emit newData(buffer);
      buffer.clear();
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
  }*/
}
