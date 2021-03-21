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

#include "myscaledoublespinbox.h"

MyScaleDoubleSpinBox::MyScaleDoubleSpinBox(QWidget* parent) : QDoubleSpinBox(parent) {}

QValidator::State MyScaleDoubleSpinBox::validate(QString& input, int& pos) const {
  // Tato funkce kontroluje, jestli to co uživatel zadává je platná hodnota
  input.replace(',', '.');
  if (!input.isEmpty()) {
    if (input.at(0) == '.')
      input.push_front('0');
  }
  if (input.count('.') > 1)
    return QValidator::State::Invalid;
  Q_UNUSED(pos)

  for (int i = 0; i < input.length(); i++) {
    if (input.at(i).isDigit())
      continue;
    else if (i == 0 && (input.at(i) == '*' || input.at(i) == MULTIPLY))
      continue;
    else if (i == 0 && (input.at(i) == '/' || input.at(i) == DIVIDE))
      continue;
    else if (i == 1 && input.at(i) == ' ')
      continue;
    else if (input.at(i) == '.')
      continue;
    else
      return QValidator::State::Invalid;
  }
  return QValidator::State::Acceptable;
}

QString MyScaleDoubleSpinBox::textFromValue(double val) const {
  // Zobrazení hodnoty
  if (val < 1)
    return DIVIDE + QString(' ') + toSignificantDigits(1.0 / val, 3);
  else
    return MULTIPLY + QString(' ') + toSignificantDigits(val, 3);
}

void MyScaleDoubleSpinBox::stepBy(int steps) {
  if (steps == 0)
    return;

  // Chování při přechodu mezi násobením a dělením je řešeno samostatně
  if (steps < 0) {
    if (value() >= 1.0 && value() < 1.1) {
      setValue(1.0 / 1.1); return;
    }
  } else {
    if (value() < 1.0 && value() >= 1.0 / 1.1) {
      setValue(1.0); return;
    }
  }

  double val;
  double dividing;
  if (this->value() >= 1) {
    val = value();
    dividing = false;
  } else {
    val = 1.0 / value();
    dividing = true;
  }

  int orderOfValue = intLog10(val);

  if (dividing) {
    if (steps > 0) {
      val -= std::pow(10, orderOfValue - 1);
    } else {
      val += std::pow(10, orderOfValue - 1);
    }
    val = 1.0 / val;
  } else {
    if (steps > 0) {
      val += std::pow(10, orderOfValue - 1);
    } else {
      val -= std::pow(10, orderOfValue - 1);
    }
  }

  setValue(val);
}

double MyScaleDoubleSpinBox::valueFromText(const QString& text) const {
  if (text.isEmpty())
    return 1;

  // Zpracování hodnoty napsané uživatelem
  bool isok = false;
  double val;

  QChar first = text.at(0);
  if (!first.isDigit()) {
    if (first == '/' || first == DIVIDE)
      val = 1.0 / text.midRef(1).toDouble(&isok);
    else if (first == '*' || first == MULTIPLY)
      val = text.midRef(1).toDouble(&isok);
    else
      isok = false;
  } else {
    val = text.toDouble(&isok);
  }

  if (!isok)
    return 1;
  return val;
}
