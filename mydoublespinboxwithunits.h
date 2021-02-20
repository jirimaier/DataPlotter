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

#ifndef MYDOUBLESPINBOXWITHUNITS_H
#define MYDOUBLESPINBOXWITHUNITS_H

#include "enums_defines_constants.h"
#include <QDoubleSpinBox>
#include <QWidget>

class MyDoubleSpinBoxWithUnits : public QDoubleSpinBox {
  Q_OBJECT
public:
  explicit MyDoubleSpinBoxWithUnits(QWidget *parent = nullptr);

private:
  QValidator::State validate(QString &input, int &pos) const;
  QString textFromValue(double val) const;
  double valueFromText(const QString &text) const;
  QString validchars = "Mkmu";
signals:
};

#endif // MYDOUBLESPINBOXWITHUNITS_H
