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

#include "mydoublespinboxwithunits.h"
#include "global.h"
#include "math/simpleexpressionparser.h"
#include <QDebug>

MyDoubleSpinBoxWithUnits::MyDoubleSpinBoxWithUnits(QWidget *parent) : QDoubleSpinBox(parent) {}

QValidator::State MyDoubleSpinBoxWithUnits::validate(QString &input, int &pos) const {
  Q_UNUSED(pos);

  QJSEngine engine;

  QString expr = input;
  if (!suffix().isEmpty())
    expr = expr.left(expr.lastIndexOf(suffix()));
  if (expr.trimmed().isEmpty())
    return QValidator::State::Intermediate;
  SimpleExpressionParser parser;
  if (!parser.validate(expr))
    return QValidator::State::Invalid;
  bool isOK;
  (void)parser.evaluate(expr, &isOK);
  return isOK ? QValidator::State::Acceptable : QValidator::State::Intermediate;
}

QString MyDoubleSpinBoxWithUnits::textFromValue(double val) const {
  if (suffix().length() == 0)
    return (QString::number(val, 'g', 3));
  if (suffix().length() >= 2) {
    if (suffix().trimmed().left(2) == "dB")
      return (QString::number(val, 'g', 3) + " ");
  }

  return (floatToNiceString(val, 4, false, false, trimDecimalZeroes));
}

double MyDoubleSpinBoxWithUnits::valueFromText(const QString &text) const {
  QString expr = text;
  SimpleExpressionParser parser;
  if (!suffix().isEmpty())
    expr = expr.left(expr.lastIndexOf(suffix()));
  bool isOK;
  double newValue = parser.evaluate(expr, &isOK);
  if (isOK)
    return newValue;
  else
    return value();
}

void MyDoubleSpinBoxWithUnits::stepBy(int steps) {
  if (!adaptiveStep()) {
    QDoubleSpinBox::stepBy(steps);
    return;
  }

  // Get the current value
  double currentValue = value();

  // Calculate the adaptive step based on the current value
  Q_ASSERT(!qFuzzyIsNull(currentValue));

  // Calculate the magnitude of the current value
  double magnitude = std::pow(10, std::floor(std::log10(std::fabs(currentValue * 1.000001))));

  // Calculate the adaptive step as one order of magnitude less than the value
  double adaptiveStep = magnitude / 10.0;

  // Adjust the step value by the adaptive step
  setSingleStep(adaptiveStep);

  // Call the base class implementation to perform the step
  QDoubleSpinBox::stepBy(steps);
}

void MyDoubleSpinBoxWithUnits::setUnit(UnitOfMeasure unit) {
  QDoubleSpinBox::setSuffix(unit.text);
  this->useUnitPrefix = unit.mode == UnitOfMeasure::time || UnitOfMeasure::usePrefix;
}

void MyDoubleSpinBoxWithUnits::showEvent(QShowEvent *event) {
  // Call the base class implementation
  QWidget::showEvent(event);

  setDecimals(12);
  if (!qFuzzyIsNull(maximum())) {
    if (maximum() > 0)
      setMaximum(1e30);
    else
      setMaximum(-1e-11);
  }

  if (!qFuzzyIsNull(minimum())) {
    if (minimum() < 0)
      setMinimum(-1e30);
    else
      setMinimum(1e-11);
  }
}

bool MyDoubleSpinBoxWithUnits::adaptiveStep() const { return m_adaptiveStep; }

void MyDoubleSpinBoxWithUnits::setAdaptiveStep(bool newAdaptiveStep) { m_adaptiveStep = newAdaptiveStep; }

void MyDoubleSpinBoxWithUnits::setStepRelativeToRange(double range, int ordersBelow) { setSingleStep(pow(10.0, std::floor(log10(range * 1.000001) - ordersBelow))); }
