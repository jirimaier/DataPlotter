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

// This class is implementing Consistent Overhead Byte Stuffing (COBS)
// https://blog.mbedded.ninja/programming/serialization-formats/consistent-overhead-byte-stuffing-cobs/

#ifndef COBS_H
#define COBS_H

#include <QByteArray>

struct COBS {
public:
  static QByteArray encode(QByteArray data);
  static QByteArray decode(QByteArray data);

private:
  COBS() {}
};

#endif // COBS_H
