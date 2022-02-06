#include "qmlterminalinterface.h"

QmlTerminalInterface::QmlTerminalInterface(QObject* parent) : QObject(parent) {

}

const QString& QmlTerminalInterface::testText() const {
  return m_testText;
}

void QmlTerminalInterface::setTestText(const QString& newTestText) {
  if (m_testText == newTestText)
    return;
  m_testText = newTestText;
  emit testTextChanged();
}

void QmlTerminalInterface::transmitToSerial(QVariant data) {
  emit dataTransmitted(data.toByteArray());
}

void QmlTerminalInterface::sendToParser(QVariant data) {
  emit dataSentToParser(data.toByteArray());
}

void QmlTerminalInterface::directInput(QByteArray data) {
  emit receivedFromSerial(data);
}
