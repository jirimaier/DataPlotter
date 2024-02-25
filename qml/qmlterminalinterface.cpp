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

#include "qmlterminalinterface.h"

QmlTerminalInterface::QmlTerminalInterface(QObject *parent) : QObject(parent) {}

void QmlTerminalInterface::transmitToSerial(QVariant data, const int bytes) {
  qDebug() << data.toList();
  qDebug() << data.toByteArray();

  char buffer[16];

  if (bytes > 16 || bytes < 0) {
    qWarning() << "transmitToSerial method received invalid value of bytes";
    return;
  }

  if (bytes > 0) {
    qDebug() << data.constData();
    memcpy(&buffer, data.constData(), bytes);
  }

  emit dataTransmitted(data.toByteArray());
}

void QmlTerminalInterface::sendToParser(QVariant data) { emit dataSentToParser(data.toByteArray()); }

void QmlTerminalInterface::directInput(QByteArray data) { emit receivedFromSerial(data); }

bool QmlTerminalInterface::darkThemeIsUsed() const { return m_darkThemeIsUsed; }

void QmlTerminalInterface::setDarkThemeIsUsed(bool newDarkThemeIsUsed) {
  if (m_darkThemeIsUsed == newDarkThemeIsUsed)
    return;
  m_darkThemeIsUsed = newDarkThemeIsUsed;
  emit darkThemeIsUsedChanged();
}

const QString &QmlTerminalInterface::tabBackground() const { return m_tabBackground; }

void QmlTerminalInterface::setTabBackground(const QString &newTabBackground) {
  if (m_tabBackground == newTabBackground)
    return;
  m_tabBackground = newTabBackground;
  emit tabBackgroundChanged();
}
