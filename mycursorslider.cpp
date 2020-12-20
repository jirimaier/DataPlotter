//  Copyright (C) 2020  Jiří Maier

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

myCursorSlider::myCursorSlider(QWidget *parent) : QSlider(parent) {
  realValue = value();
  connect(this, &QAbstractSlider::valueChanged, this, &myCursorSlider::positionChanged);
}

void myCursorSlider::updateRange(int min, int max) {
  blockSignals(true);
  setMinimum(min);
  setMaximum(max);
  setValue(realValue);
  if (realValue < min || realValue > max)
    setStyleSheet("QSlider::handle:horizontal {background-color:gray;}");
  else
    setStyleSheet("");
  setSingleStep(MAX((max - min) / 150, 1));
  setPageStep(MAX((max - min) / 150, 1));
  blockSignals(false);
}

void myCursorSlider::positionChanged(int newpos) {
  realValue = newpos;
  setStyleSheet("");
  emit realValueChanged(realValue);
}

void myCursorSlider::setRealValue(int newValue) {
  if (newValue < 0)
    return;
  realValue = newValue;
  blockSignals(true);
  setValue(realValue);
  if (realValue < minimum() || realValue > maximum())
    setStyleSheet("QSlider::handle:horizontal {background-color:gray;}");
  else
    setStyleSheet("");
  blockSignals(false);
  emit realValueChanged(realValue);
}
