//  Copyright (C) 2020-2024  Jiří Maier

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

#ifndef SERIALREADER_H
#define SERIALREADER_H

#include "communication/telnetserver.h"
#include "manualinputdialog.h"
#include <QDebug>
#include <QObject>
#include <QSerialPort>
#include <QThread>

class SerialReader : public QObject {
  Q_OBJECT
public:
  explicit SerialReader(QObject *parent = nullptr);
  ~SerialReader();
  void setSimInputDialog(QSharedPointer<ManualInputDialog> simIn);

private:
  QSerialPort *serial;
  bool serialMonitor;
  QSharedPointer<ManualInputDialog> simulatedInputDialog;
  void startSimulatedInput();
  void newData(QByteArray data);
  void endSim();
  bool simConnected = false;
  bool telnetConnected = false;
  TelnetServer *telnet;

private slots:
  void read();
  void errorOccurred();
signals:
  /// Sends information whether the port is connected
  void connectionResult(bool connected, QString caption, QString details);
  /// Notifies that writing to the port has finished
  void finishedWriting();
  /// Sends the read data
  void sendData(QByteArray data);
  /// Reports connection (expects a reply that the parser is ready)
  void started();
  /// Forwards data
  void monitor(QByteArray data);

  void stopManualInputData();
public slots:
  /// Creates a QSerialPort instance
  void init();
  /// Connects or disconnects the port
  void toggle(QString portName, int baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControll);
  /// Attempts to open the port
  void begin(QString portName, int baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControll);
  /// Closes the port
  void end();
  /// Writes data to the port
  void write(QByteArray data);
  /// Starts forwarding data
  void parserReady();
  /// Enables data forwarding
  void enableMonitoring(bool en) { serialMonitor = en; }
  /// If the port is connected, change baud without disconnecting
  void changeBaud(qint32 baud);
};

#endif // SERIALREADER_H
