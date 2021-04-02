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

#include "mydoublespinboxwithunits.h"

MyDoubleSpinBoxWithUnits::MyDoubleSpinBoxWithUnits(QWidget* parent) : QDoubleSpinBox(parent) {}

QValidator::State MyDoubleSpinBoxWithUnits::validate(QString& input, int& pos) const {
  // Tato funkce kontroluje, jestli to co uživatel zadává je platná hodnota
  input.replace(',', '.');
  if (!input.isEmpty()) {
    if (input.at(0) == '.')
      input.push_front('0');
  }
  if (input.count('.') > 1)
    return QValidator::State::Invalid;
  QString inputCopy(input);
  Q_UNUSED(pos)
  inputCopy.remove(suffix());
  for (int i = 0; i < inputCopy.length(); i++) {
    if (inputCopy.at(i).isDigit())
      continue;
    if (i == 0 && (inputCopy.at(i) == '+' || inputCopy.at(i) == '-'))
      continue;
    if (i == inputCopy.length() - 1 && validchars.contains(inputCopy.at(i)))
      continue;
    if (i == inputCopy.length() - 2 && inputCopy.at(i) == ' ')
      continue;
    if (inputCopy.at(i) == '.')
      continue;
    if (inputCopy.at(i) == ' ')
      continue;
    return QValidator::State::Invalid;
  }
  return QValidator::State::Acceptable;
}

QString MyDoubleSpinBoxWithUnits::textFromValue(double val) const {
  // Zobrazení hodnoty
  if (suffix().length() == 0)
    return (QString::number(val, 'g', 3));
  if (suffix().length() >= 2) {
    if (suffix().left(2) == "dB")
      return (QString::number(val, 'g', 3) + " ");
  }

  return (floatToNiceString(val, 4, false, false, trimDecimalZeroes));
}

double MyDoubleSpinBoxWithUnits::valueFromText(const QString& text) const {
  // Zpracování hodnoty napsané uživatelem
  if (text.isEmpty())
    return emptyDefaultValue;

  bool isok = false;
  double val;
  if (text.contains('k'))
    val = 1e3 * text.leftRef(text.indexOf('k')).toDouble(&isok);
  else if (text.contains('M'))
    val = 1e6 * text.leftRef(text.indexOf('M')).toDouble(&isok);
  else if (text.contains('m'))
    val = 1e-3 * text.leftRef(text.indexOf('m')).toDouble(&isok);
  else if (text.contains('u'))
    val = 1e-6 * text.leftRef(text.indexOf('u')).toDouble(&isok);
  else
    val = text.leftRef(text.indexOf(suffix())).toDouble(&isok);
  if (!isok)
    return emptyDefaultValue;
  return val;
}

void MyDoubleSpinBoxWithUnits::setUnit(QString suffix, bool useUnitPrefix) {
  QDoubleSpinBox::setSuffix(suffix);
  this->useUnitPrefix = useUnitPrefix;
}
