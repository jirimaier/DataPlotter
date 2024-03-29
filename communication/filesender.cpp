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

#include "filesender.h"
#include <QDebug>

FileSender::FileSender(QObject *parent) : QObject{parent} {}

void FileSender::newFile(const QByteArray &file) {
  buffer = file;
  m_count = 0;
  m_hasEnding = 0;
  m_fillEnd = 0;
}

QByteArray FileSender::nextPart(QByteArray countText, QByteArray args) {
  if (!args.isEmpty()) {
    if (args.toLower() == "0") {
      m_ending = 0;
      m_fillEnd = 0;
    } else if (args.toLower() == "0s") {
      m_ending = 0;
      m_fillEnd = 1;
    } else if (args.toLower() == "eot") {
      m_ending = 4;
      m_fillEnd = 0;
    } else if (args.toLower() == "eots") {
      m_ending = 4;
      m_fillEnd = 1;
    } else if (args.toLower() == "semic") {
      m_ending = ';';
      m_fillEnd = 0;
    } else if (args.toLower() == "semics") {
      m_ending = ';';
      m_fillEnd = 1;
    } else if (args.toLower() == "dollar") {
      m_ending = '$';
      m_fillEnd = 0;
    } else if (args.toLower() == "dollars") {
      m_ending = '$';
      m_fillEnd = 1;
    } else if (args.toLower() == "eof") {
      m_ending = EOF;
      m_fillEnd = 0;
    } else if (args.toLower() == "eofs") {
      m_ending = EOF;
      m_fillEnd = 1;
    } else if (args.toLower() == "lf") {
      m_ending = '\n';
      m_fillEnd = 0;
    } else if (args.toLower() == "lfs") {
      m_ending = '\n';
      m_fillEnd = 1;
    } else if (args.toLower() == "cr") {
      m_ending = '\r';
      m_fillEnd = 0;
    } else if (args.toLower() == "crs") {
      m_ending = '\r';
      m_fillEnd = 1;
    } else
      return (tr("Invalid ending").toUtf8());
    m_hasEnding = true;
  }

  if (!countText.isEmpty()) {
    bool isok = false;
    m_count = countText.toUInt(&isok);
    if (!isok) {
      if (countText.toLower() == "all")
        m_count = m_hasEnding ? (buffer.length() + 1) : buffer.length();
      else
        return (tr("Invalid length").toUtf8());
    }
  }

  if (m_count <= 0)
    return tr("Block length not specified").toUtf8();

  QByteArray part = buffer.left(qMin(m_count, buffer.length()));
  buffer.remove(0, qMin(m_count, buffer.length()));

  if (m_hasEnding) {
    if (m_fillEnd) {
      while (part.length() < m_count)
        part.append(m_ending);
    } else {
      if (part.length() < m_count)
        part.append(m_ending);
    }
  }

  qDebug() << part;
  emit transmit(part);

  return QByteArray({});
}
