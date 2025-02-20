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

void QmlTerminalInterface::transmitToSerial(QVariant data, QString type) {
  auto list = data.toList();
  if (list.isEmpty()) {
    list.append(data);
  }

  if (type.at(0) == 's') {
    for (const auto &element : qAsConst(list)) {
      emit dataTransmitted(element.toByteArray());
      qDebug() << "QML terminal transmitted: " << element.toByteArray();
    }
    return;
  }

  bool bigendian = type.at(0).isUpper();
  type = type.toLower();
  char buffer[8] = {0};
  int bytes = 0;

  bool isok = false;
  for (const auto &element : qAsConst(list)) {
    if (type == "uint8" || type == "u8") {
      quint8 i = element.toUInt(&isok);
      bytes = 1;
      memcpy(buffer, &i, bytes);
    } else if (type == "uint16" || type == "u16") {
      quint16 i = element.toUInt(&isok);
      bytes = 2;
      memcpy(buffer, &i, bytes);
    } else if (type == "uint24" || type == "u24") {
      quint32 i = element.toUInt(&isok);
      bytes = 3;
      memcpy(buffer, &i, bytes);
    } else if (type == "uint32" || type == "u32") {
      quint32 i = element.toUInt(&isok);
      bytes = 4;
      memcpy(buffer, &i, bytes);
    } else if (type == "uint64" || type == "u64") {
      quint64 i = element.toUInt(&isok);
      bytes = 8;
      memcpy(buffer, &i, bytes);
    } else if (type == "int8" || type == "i8") {
      qint8 i = element.toInt(&isok);
      bytes = 1;
      memcpy(buffer, &i, bytes);
    } else if (type == "int16" || type == "i16") {
      qint16 i = element.toInt(&isok);
      bytes = 2;
      memcpy(buffer, &i, bytes);
    } else if (type == "int24" || type == "i24") {
      qint32 i = element.toInt(&isok);
      bytes = 3;
      memcpy(buffer, &i, bytes);
    } else if (type == "int32" || type == "i32") {
      qint32 i = element.toInt(&isok);
      bytes = 4;
      memcpy(buffer, &i, bytes);
    } else if (type == "int64" || type == "i64") {
      qint64 i = element.toInt(&isok);
      bytes = 8;
      memcpy(buffer, &i, bytes);
    } else if (type == "float" || type == "f") {
      float f = element.toFloat(&isok);
      bytes = 4;
      memcpy(buffer, &f, bytes);
    } else if (type == "double" || type == "d") {
      double d = element.toDouble(&isok);
      bytes = 8;
      memcpy(buffer, &d, bytes);
    } else {
      qDebug() << "QML terminal transmit ERROR: invalid format";
      return;
    }

    if (!isok) {
      qDebug() << "QML terminal transmit ERROR: conversion error";
      return;
    }

    if (bigendian) {
      if (bytes == 2) {
        std::swap(buffer[0], buffer[1]);
      } else if (bytes == 3) {
        std::swap(buffer[0], buffer[2]);
      } else if (bytes == 4) {
        std::swap(buffer[0], buffer[3]);
        std::swap(buffer[1], buffer[2]);
      } else if (bytes == 8) {
        std::swap(buffer[0], buffer[7]);
        std::swap(buffer[1], buffer[6]);
        std::swap(buffer[2], buffer[5]);
        std::swap(buffer[3], buffer[4]);
      }
    }

    emit dataTransmitted(QByteArray(buffer, bytes));
    qDebug() << "QML terminal transmitted: " << QByteArray(buffer, bytes);
  }
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
