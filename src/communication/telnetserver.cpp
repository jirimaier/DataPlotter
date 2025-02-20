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

#include "telnetserver.h"
#include <QDebug>

TelnetServer::TelnetServer(QObject *parent) : QObject(parent) {
  m_tcpServer = new QTcpServer(this);
  QObject::connect(m_tcpServer, &QTcpServer::newConnection, this, &TelnetServer::onNewConnection);
}

TelnetServer::~TelnetServer() { disconnect(); }

int TelnetServer::connect(quint16 port) {
  if (m_tcpServer->isListening()) {
    return port;
  }

  while (!m_tcpServer->listen(QHostAddress::Any, port)) {
    if (port == (quint16)(-1))
      return -1;
    port++;
  }

  return port;
}

void TelnetServer::disconnect() {
  if (m_tcpServer->isListening()) {
    m_tcpServer->close();

    // Disconnect and delete all client sockets
    for (QTcpSocket *socket : qAsConst(m_clientSockets)) {
      socket->disconnect();
      socket->deleteLater();
    }
    m_clientSockets.clear();
  }
}

void TelnetServer::onNewConnection() {
  QTcpSocket *clientSocket = m_tcpServer->nextPendingConnection();
  QObject::connect(clientSocket, &QTcpSocket::disconnected, this, &TelnetServer::onDisconnected);
  QObject::connect(clientSocket, &QTcpSocket::readyRead, this, &TelnetServer::onReadyRead);
  m_clientSockets.append(clientSocket);
  emit clientConnected(clientSocket);
}

void TelnetServer::onDisconnected() {
  QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
  if (socket) {
    m_clientSockets.removeOne(socket);
    emit clientDisconnected(socket);
    socket->deleteLater();
  }
}

void TelnetServer::onReadyRead() {
  QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
  if (socket) {
    QByteArray message = socket->readAll();
    emit messageReceived(message);
  }
}

void TelnetServer::write(const QByteArray &message) {
  for (QTcpSocket *socket : qAsConst(m_clientSockets)) {
    socket->write(message);
    socket->flush();
  }
}
