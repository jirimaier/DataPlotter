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
  requestInterruption();
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
  while (!isInterruptionRequested()) {
    if (serial.waitForReadyRead(SERIAL_READ_LOOP_TIMEOUT)) {
      buffer += serial.readAll();
      forever {
        int begin = head.indexIn(buffer);
        int end = tail.indexIn(buffer);
        if (begin == -1 && end == -1) {
          if (serial.waitForReadyRead(SERIAL_LINE_TIMEOUT))
            break;
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
      }
    }
  }
  emit connectionResult(false, tr("Not connected"));
  if (serial.isOpen())
    serial.close();
  mutex.lock();
  mutex.unlock();
}
