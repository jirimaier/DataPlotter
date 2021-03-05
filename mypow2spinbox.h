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

#ifndef MYPOW2SPINBOX_H
#define MYPOW2SPINBOX_H

#include <QObject>
#include <QSpinBox>

#include "enums_defines_constants.h"

class MyPow2Spinbox : public QSpinBox {
  Q_OBJECT
 public:
  explicit MyPow2Spinbox(QWidget* parent = nullptr);

 public slots:
  void stepBy(int steps);
  void setValue(int val);

 private:
  int valueFromText(const QString& text) const;

 signals:

};

#endif // MYPOW2SPINBOX_H
