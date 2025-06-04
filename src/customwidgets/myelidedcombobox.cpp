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

#include "myelidedcombobox.h"

MyElidedComboBox::MyElidedComboBox(QWidget* parent) : QComboBox(parent) {

}

void MyElidedComboBox::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event)

  // If the text is longer than the field, it is truncated and ended with three dots

  // This code is based on:
  // https://stackoverflow.com/questions/41360618/qcombobox-elided-text-on-selected-item

  QStyleOptionComboBox opt;
  initStyleOption(&opt);

  QStylePainter p(this);
  p.drawComplexControl(QStyle::CC_ComboBox, opt);

  QRect textRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
  opt.currentText = p.fontMetrics().elidedText(opt.currentText, Qt::ElideRight, textRect.width());
  p.drawControl(QStyle::CE_ComboBoxLabel, opt);
}
