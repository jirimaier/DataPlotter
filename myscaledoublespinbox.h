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

#ifndef MYSCALEDOUBLESPINBOX_H
#define MYSCALEDOUBLESPINBOX_H

#include "enumsDefinesConstants.h"
#include <QDoubleSpinBox>
#include <QWidget>

#define MULTIPLY QChar((uint16_t)0x00D7)
#define DIVIDE QChar((uint16_t)0x00F7)

class MyScaleDoubleSpinBox : public QDoubleSpinBox {
  Q_OBJECT
 public:
  explicit MyScaleDoubleSpinBox(QWidget* parent = nullptr);

 private slots:
  void stepBy(int steps);

 private:
  QValidator::State validate(QString& input, int& pos) const;
  QString textFromValue(double val) const;
  double valueFromText(const QString& text) const;

 signals:

};

#endif // MYSCALEDOUBLESPINBOX_H
