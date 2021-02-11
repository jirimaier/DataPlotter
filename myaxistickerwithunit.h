//  Copyright (C) 2020-2021  Jiří Maier

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

#ifndef MYAXISTICKERWITHUNIT_H
#define MYAXISTICKERWITHUNIT_H

#include "enums_defines_constants.h"
#include "qcustomplot.h"

class MyAxisTickerWithUnit : public QCPAxisTickerFixed {
public:
  QString unit = "";
  QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision);
};

#endif // MYAXISTICKERWITHUNIT_H
