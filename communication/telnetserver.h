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
