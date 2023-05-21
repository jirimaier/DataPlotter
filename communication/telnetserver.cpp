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
