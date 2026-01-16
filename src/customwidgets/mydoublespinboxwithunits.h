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

#ifndef MYDOUBLESPINBOXWITHUNITS_H
#define MYDOUBLESPINBOXWITHUNITS_H

#include "global.h"
#include "math/expressionparser.h"
#include <QDoubleSpinBox>
#include <QJSEngine>
#include <QJSValue>
#include <QWidget>

/// Double spin box, který zobrazuje předpony jednotek (milli, mikro ...). Samotná jednotka se nastaví jako suffix.
class MyDoubleSpinBoxWithUnits : public QDoubleSpinBox {
  Q_OBJECT
public:
  explicit MyDoubleSpinBoxWithUnits(QWidget *parent = nullptr);

  /// Pokud jsou v počtu zobrazených cifer na konci jen nuly (např. 5.100), je zobrazeno (5.1)
  bool trimDecimalZeroes = false;

  /// Výchozí hodnota pokud uživatel pole vymaže nebo zadá neplatnou hodnotu
  double emptyDefaultValue = 0;

  /// Nastaví jednotku
  void setUnit(UnitOfMeasure unit);

  bool adaptiveStep() const;
  void setAdaptiveStep(bool newAdaptiveStep);

  void setStepRelativeToRange(double range, int ordersBelow = 2);

protected:
  void showEvent(QShowEvent *event) override;
  virtual void stepBy(int steps) override;
  bool m_adaptiveStep = false;

private:
  QValidator::State validate(QString &input, int &pos) const;
  QString textFromValue(double val) const;
  double valueFromText(const QString &text) const;
  bool useUnitPrefix = true;

signals:
};

#endif // MYDOUBLESPINBOXWITHUNITS_H
