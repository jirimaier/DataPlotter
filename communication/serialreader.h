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

#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QDebug>
#include <QObject>
#include <QSerialPort>
#include <QThread>
#include "manualinputdialog.h"

class SerialReader : public QObject {
  Q_OBJECT
 public:
  explicit SerialReader(QObject* parent = nullptr);
  ~SerialReader();
  void setSimInputDialog(QSharedPointer<ManualInputDialog> simIn);

 private:
  QSerialPort* serial;
  bool serialMonitor;
  QSharedPointer<ManualInputDialog> simulatedInputDialog;
  void startSimulatedInput();
  void newData(QByteArray data);
  void endSim();
  bool simConnected = false;

 private slots:
  void read();
  void errorOccurred();
 signals:
  /// Pošle informaci jestli je port připojen
  void connectionResult(bool connected, QString caption, QString details);
  /// Oznámý dokončení zápisu dat do portu
  void finishedWriting();
  /// Počle přečtená data
  void sendData(QByteArray data);
  /// Oznámí připojení (očekává odpověď že parser je připravený)
  void started();
  /// Přeposílá data
  void monitor(QByteArray data);
 public slots:
  /// Vytvoří instanci QSerialPortu
  void init();
  /// Připojí nebo odpojí port
  void toggle(QString portName,
              int baudRate,
              QSerialPort::DataBits dataBits,
              QSerialPort::Parity parity,
              QSerialPort::StopBits stopBits,
              QSerialPort::FlowControl flowControll);
  /// Pokusí se otevřít port
  void begin(QString portName,
             int baudRate,
             QSerialPort::DataBits dataBits,
             QSerialPort::Parity parity,
             QSerialPort::StopBits stopBits,
             QSerialPort::FlowControl flowControll);
  /// Zavře port
  void end();
  /// Zapíše data do portu
  void write(QByteArray data);
  /// Zahájí přeposílání dat
  void parserReady();
  /// Zapne předávání dat
  void enableMonitoring(bool en) { serialMonitor = en; }
  /// Pokud je port připojen, změní baud bez odpojení
  void changeBaud(qint32 baud);
};

#endif  // SERIALREADER_H
