#include "qmlterminalinterface.h"

QmlTerminalInterface::QmlTerminalInterface(QObject* parent) : QObject(parent) {

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

bool QmlTerminalInterface::darkThemeIsUsed() const {
  return m_darkThemeIsUsed;
}

void QmlTerminalInterface::setDarkThemeIsUsed(bool newDarkThemeIsUsed) {
  if (m_darkThemeIsUsed == newDarkThemeIsUsed)
    return;
  m_darkThemeIsUsed = newDarkThemeIsUsed;
  emit darkThemeIsUsedChanged();
}

const QString& QmlTerminalInterface::tabBackground() const {
  return m_tabBackground;
}

void QmlTerminalInterface::setTabBackground(const QString& newTabBackground) {
  if (m_tabBackground == newTabBackground)
    return;
  m_tabBackground = newTabBackground;
  emit tabBackgroundChanged();
}