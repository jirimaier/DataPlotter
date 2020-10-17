#include "serialworker.h"

SerialWorker::SerialWorker(QObject *parent) : QObject(parent) { qDebug() << "SerialWorker created from " << QThread::currentThreadId(); }

SerialWorker::~SerialWorker() {
  delete lineTimeouter;
  delete buffer;
  delete serial;
  delete commandBeginningMatcher;
  delete commandEndMatcher;
  qDebug() << "SerialWorker deleted from " << QThread::currentThreadId();
}

void SerialWorker::init() {
  buffer = new QByteArray;
  serial = new QSerialPort;
  lineTimeouter = new QTimer;
  commandBeginningMatcher = new QByteArrayMatcher(CMD_BEGIN);
  commandEndMatcher = new QByteArrayMatcher(CMD_END);
  lineTimeouter->setSingleShot(true);
  connect(lineTimeouter, &QTimer::timeout, this, &SerialWorker::lineTimedOut);
  connect(serial, &QSerialPort::readyRead, this, &SerialWorker::read);
  connect(serial, &QSerialPort::bytesWritten, this, &SerialWorker::finishedWriting);
  qDebug() << "SerialWorker initialised from " << QThread::currentThreadId();

// V starším Qt (Win XP) není signál pro error
#if QT_VERSION >= 0x050800
  connect(serial, &QSerialPort::errorOccurred, this, &SerialWorker::errorOccurred);
#endif
}

void SerialWorker::read() {
  int begin, end;
  buffer->append(serial->readAll());
  while (!buffer->isEmpty()) {
    begin = commandBeginningMatcher->indexIn(*buffer);
    end = commandEndMatcher->indexIn(*buffer);
    if (begin == -1 && end == -1) {
      lineTimeouter->start(lineTimeout);
      break;
    } else {
      lineTimeouter->stop();
    }
    if (end != -1 && (end < begin || begin == -1)) {
      emit newLine(buffer->left(end), DataLineType::dataEnded);
      buffer->remove(0, end + CMD_END_LENGTH);
      continue;
    }
    if (begin > 0) {
      emit newLine(buffer->left(begin), DataLineType::dataImplicitEnded);
      buffer->remove(0, begin);
      continue;
    }
    if (begin == 0 && end == -1)
      break;
    if (begin == 0 && end > begin) {
      emit newLine(buffer->mid(begin + CMD_BEGIN_LENGTH, end - begin - CMD_BEGIN_LENGTH), DataLineType::command);
      buffer->remove(0, end + CMD_END_LENGTH);
      continue;
    }
  }
}

void SerialWorker::lineTimedOut() {
  qDebug() << "Line timed out";
  if (!buffer->isEmpty()) {
    emit newLine(*buffer, DataLineType::dataTimeouted);
    buffer->clear();
  }
}

void SerialWorker::errorOccurred() {
  if (serial->isOpen())
    serial->close();
  if (lineTimeouter->isActive())
    lineTimeouter->stop();
  emit connectionResult(false, tr("Error: ") + serial->errorString());
}

void SerialWorker::changeLineTimeout(int value) { lineTimeout = value; }

void SerialWorker::requestedBufferDebug() { emit bufferDebug(*buffer); }

void SerialWorker::begin(QString portName, int baudRate) {
  serial->setPortName(portName);
  serial->setBaudRate(baudRate);
  serial->setDataBits(QSerialPort::Data8);
  serial->setParity(QSerialPort::NoParity);
  serial->setStopBits(QSerialPort::OneStop);
  serial->setFlowControl(QSerialPort::NoFlowControl);
  if (serial->open(QIODevice::ReadWrite))
    emit connectionResult(true, tr("Connected to ") + serial->portName() + tr(" at ") + QString::number(serial->baudRate()) + tr(" bps"));
  else if (serial->open(QIODevice::ReadOnly))
    emit connectionResult(true, tr("Read-only ") + serial->portName() + tr(" at ") + QString::number(serial->baudRate()) + tr(" bps"));
  else
    emit connectionResult(false, tr("Error: ") + serial->errorString());
  if (serial->isOpen()) {
    if (serial->bytesAvailable() > 0)
      serial->readAll();
  }
}

void SerialWorker::end() {
  serial->clear();
  if (serial->isOpen())
    serial->close();
  if (lineTimeouter->isActive())
    lineTimeouter->stop();
  emit connectionResult(false, tr("Not connected"));
  buffer->clear();
}

void SerialWorker::write(QByteArray data) { serial->write(data); }
