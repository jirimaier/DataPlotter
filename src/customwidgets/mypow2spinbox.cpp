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

#include "mypow2spinbox.h"

MyPow2Spinbox::MyPow2Spinbox(QWidget* parent): QSpinBox(parent) {

}

void MyPow2Spinbox::stepBy(int steps) {
  if (steps > 0)
    this->QSpinBox::setValue(value() * 2);
  else if (steps < 0)
    this->QSpinBox::setValue(value() / 2);
}

int MyPow2Spinbox::valueFromText(const QString& text) const {
  return (nextPow2(QSpinBox::valueFromText(text)));
}

void MyPow2Spinbox::setValue(int val) {
  this->QSpinBox::setValue(nextPow2(val));
}
