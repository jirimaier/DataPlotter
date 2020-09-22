#include "serialthread.h"
#include <QDebug>

SerialThread::SerialThread() {}

SerialThread::~SerialThread() { terminate(); }

void SerialThread::begin(QString portName, int baudRate) {
  const QMutexLocker locker(&mutex);
  port = portName;
  baud = baudRate;
  if (!isRunning())
    start();
}

void SerialThread::end() {
  const QMutexLocker locker(&mutex);
  quit_flag = true;
}

void SerialThread::run() {
  QByteArrayMatcher head;
  QByteArrayMatcher tail;
  head.setPattern(CMD_BEGIN);
  tail.setPattern(CMD_END);
  QSerialPort serial;
  QByteArray buffer;
  serial.setPortName(port);
  serial.setBaudRate(baud);
  serial.setDataBits(QSerialPort::Data8);
  serial.setParity(QSerialPort::NoParity);
  serial.setStopBits(QSerialPort::OneStop);
  serial.setFlowControl(QSerialPort::NoFlowControl);
  serial.open(QIODevice::ReadWrite);
  emit connectionResult(serial.isOpen(), serial.isOpen() ? tr("Connected to ") + serial.portName() + tr(" at ") + QString::number(serial.baudRate()) + tr(" bps") : tr("Error: ") + serial.errorString());
  if (!serial.isOpen())
    return;
  while (!quit_flag) {
    if (serial.waitForReadyRead(1000)) {
      buffer += serial.readAll();
      forever {
        int begin = head.indexIn(buffer);
        int end = tail.indexIn(buffer);
        if (begin == -1 && end == -1) {
          if (serial.waitForReadyRead(1))
            break;
          emit newData(buffer);
          buffer.clear();
          break;
        }
        if (end != -1 && (end < begin || begin == -1)) {
          buffer.remove(0, end + CMD_END_LENGHT);
          continue;
        }
        if (begin > 0) {
          emit newData(buffer.left(begin));
          // emit newEntry();
          buffer.remove(0, begin);
          continue;
        }
        if (begin == 0 && end == -1)
          break;
        if (begin == 0 && end > begin) {
          emit newCommand(buffer.mid(begin + head.pattern().length(), end - begin - head.pattern().length()));
          // emit newEntry();
          buffer.remove(0, end + CMD_END_LENGHT);
          continue;
        }
      }
    }
  }
  emit connectionResult(false, tr("Not connected"));
  if (serial.isOpen())
    serial.close();
  mutex.lock();
  quit_flag = false;
  mutex.unlock();
}
