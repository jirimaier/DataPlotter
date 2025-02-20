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

#ifndef FILESENDER_H
#define FILESENDER_H

#include <QObject>

class FileSender : public QObject {
  Q_OBJECT
public:
  explicit FileSender(QObject *parent = nullptr);

private:
  QByteArray buffer;
  int m_count;
  bool m_hasEnding;
  bool m_fillEnd;
  char m_ending;

public slots:
  void newFile(const QByteArray &file);
  QByteArray nextPart(QByteArray countText, QByteArray args);

signals:
  void transmit(QByteArray message);
};

#endif // FILESENDER_H
