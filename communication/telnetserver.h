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

#ifndef TELNETSERVER_H
#define TELNETSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class TelnetServer : public QObject {
  Q_OBJECT
public:
  explicit TelnetServer(QObject *parent = nullptr);
  ~TelnetServer();

signals:
  void clientConnected(QTcpSocket *socket);
  void clientDisconnected(QTcpSocket *socket);
  void messageReceived(QByteArray &message);

public slots:
  int connect(quint16 port);
  void disconnect();
  void write(const QByteArray &message);

private slots:
  void onNewConnection();
  void onDisconnected();
  void onReadyRead();

private:
  QTcpServer *m_tcpServer;
  QList<QTcpSocket *> m_clientSockets;
};

#endif // TELNETSERVER_H
