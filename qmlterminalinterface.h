#ifndef QMLTERMINALINTERFACE_H
#define QMLTERMINALINTERFACE_H

#include <QObject>
#include <QQuickItem>

class QmlTerminalInterface : public QObject {
  Q_OBJECT
 public:
  explicit QmlTerminalInterface(QObject* parent = nullptr);
  ~QmlTerminalInterface() {}

  const QString& testText() const;
  void setTestText(const QString& newTestText);

  Q_INVOKABLE void transmitToSerial(QVariant data);
  Q_INVOKABLE void sendToParser(QVariant data);

  void directInput(QByteArray data);

 signals:
  void testTextChanged();
  void receivedFromSerial(QByteArray data);
  void dataTransmitted(QByteArray data);
  void dataSentToParser(QByteArray data);

 private:
  QString m_testText = "helloWorld;";
  Q_PROPERTY(QString testText READ testText WRITE setTestText NOTIFY testTextChanged)
};

#endif // QMLTERMINALINTERFACE_H
