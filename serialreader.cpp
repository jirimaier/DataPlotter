#include "serialreader.h"

SerialReader::SerialReader(QObject *parent) : QObject(parent) { qDebug() << "SerialReader created from " << QThread::currentThreadId(); }

SerialReader::~SerialReader() {
  end();
  delete serial;
  qDebug() << "SerialParser deleted from " << QThread::currentThreadId();
}

void SerialReader::init() {
  serial = new QSerialPort;
  connect(serial, &QSerialPort::readyRead, this, &SerialReader::read);
  connect(serial, &QSerialPort::bytesWritten, this, &SerialReader::finishedWriting);
  // V starším Qt (Win XP) není signál pro error
#if QT_VERSION >= 0x050800
  connect(serial, &QSerialPort::errorOccurred, this, &SerialReader::errorOccurred);
#endif
  qDebug() << "SerialReader initialised from " << QThread::currentThreadId();
}

void SerialReader::begin(QString portName, int baudRate) {
  parserReadyFlag = false;
  if (serial->isOpen())
    end();
  serial->setPortName(portName);
  serial->setBaudRate(baudRate);
  serial->setDataBits(QSerialPort::Data8);
  serial->setParity(QSerialPort::NoParity);
  serial->setStopBits(QSerialPort::OneStop);
  serial->setFlowControl(QSerialPort::NoFlowControl);
  if (serial->open(QIODevice::ReadWrite)) {
    emit connectionResult(true, tr("Connected (") + serial->portName() + tr(", ") + QString::number(serial->baudRate()) + ")");
  } else if (serial->open(QIODevice::ReadOnly)) {
    emit connectionResult(true, tr("Connected (") + serial->portName() + tr(", ") + QString::number(serial->baudRate()) + ")");
  } else
    emit connectionResult(false, tr("Error: ") + serial->errorString());
  emit started();
}

void SerialReader::write(QByteArray data) {
  if (serial->isOpen())
    serial->write(data);
}

void SerialReader::parserReady() {
  if (serial->isOpen()) {
    if (serial->bytesAvailable() > 0)
      serial->readAll();
  }
  parserReadyFlag = true;
}

void SerialReader::end() {
  if (!serial->isOpen())
    return;
  serial->clear();
  serial->close();
  emit connectionResult(false, tr("Not connected"));
}

void SerialReader::errorOccurred() {
  if (serial->isOpen()) {
    serial->close();
    serial->clear();
  }
  emit connectionResult(false, tr("Error: ") + serial->errorString());
}

void SerialReader::toggle(QString portName, int baudRate) {
  if (!serial->isOpen())
    begin(portName, baudRate);
  else
    end();
}

void SerialReader::read() {
  if (parserReadyFlag)
    emit sendData(serial->readAll());
}
