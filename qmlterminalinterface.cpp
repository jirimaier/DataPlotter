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

bool QmlTerminalInterface::darkThemeIsUsed() const
{
    return m_darkThemeIsUsed;
}

void QmlTerminalInterface::setDarkThemeIsUsed(bool newDarkThemeIsUsed)
{
    if (m_darkThemeIsUsed == newDarkThemeIsUsed)
        return;
    m_darkThemeIsUsed = newDarkThemeIsUsed;
    emit darkThemeIsUsedChanged();
}

const QPalette &QmlTerminalInterface::themePalette() const
{
    return m_themePalette;
}

void QmlTerminalInterface::setThemePalette(const QPalette &newThemePalette)
{
    if (m_themePalette == newThemePalette)
        return;
    m_themePalette = newThemePalette;
    emit themePaletteChanged();
}
