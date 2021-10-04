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

#include "mycursorslider.h"

myCursorSlider::myCursorSlider(QWidget* parent) : QSlider(parent) {
  realPos = value();
  connect(this, &QAbstractSlider::valueChanged, this, &myCursorSlider::positionChanged);
}

void myCursorSlider::positionChanged(int newpos) {
  realPos = newpos;
  setStyleSheet("");
  emit realValueChanged(realPos);
}

void myCursorSlider::updateRange(int min, int max) {
  blockSignals(true);
  setMinimum(min);
  setMaximum(max);
  if (realPos < min || realPos > max || min == max)
    setStyleSheet("QSlider::handle:horizontal {background-color:gray;}");
  else
    setStyleSheet("");
  setValue(realPos);
  setSingleStep(MAX((max - min) / 150, 1));
  setPageStep(MAX((max - min) / 150, 1));
  blockSignals(false);
}

void myCursorSlider::setRealValue(int newValue) {
  if (newValue < 0)
    return;
  blockSignals(true);
  if (newValue < minimum() || newValue > maximum())
    setStyleSheet("QSlider::handle:horizontal {background-color:gray;}");
  else {
    setStyleSheet("");
  }
  setValue(newValue);
  blockSignals(false);
  realPos = newValue;
  emit realValueChanged(realPos);
}
