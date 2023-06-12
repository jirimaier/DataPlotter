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

#include "myaxistickerwithunit.h"

void MyAxisTickerWithUnit::setUnit(QString unit, UnitMode::enumUnitMode mode) {
  this->unit = unit;
  this->unitMode = mode;

  if (unitMode == UnitMode::time) {
    this->unit = "s";
    QRegularExpression regex("\\(([^)]+)\\)");
    QRegularExpressionMatch match = regex.match(unit);

    if (match.hasMatch()) {
      timeUnitFormat = match.captured(1);
    } else {
      timeUnitFormat = ""; // Empty string if no brackets found
    }
  }
}

QString MyAxisTickerWithUnit::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) {

  if (unitMode == UnitMode::noPrefix) {
  noPrefix:
    if (unit.isEmpty())
      return (locale.toString(tick, formatChar.toLatin1(), precision));
    return (locale.toString(tick, formatChar.toLatin1(), precision) + " " + unit);
  }

  if (unitMode == UnitMode::index) {
    if (qFuzzyCompare(round(tick), tick))
      return QString::number(static_cast<long>(tick));
    else
      return "";
  }

  if (unitMode == UnitMode::time && timeUnitFormat.isEmpty()) {
    if (tick > 60.0 || qFuzzyCompare(tick, 60)) {
      int minutes;

      double rounded = qRound(tick);
      if (qFuzzyCompare(rounded, tick))
        minutes = rounded / 60;
      else
        minutes = floor(tick) / 60;
      double sec = (tick - static_cast<double>(minutes) * 60);
      int hours = minutes / 60;
      minutes = minutes % 60;

      QString HH = hours == 0 ? "" : QString("%1:").arg(hours, 2, 10, QLatin1Char('0'));
      QString MM = QString("%1:").arg(minutes, 2, 10, QLatin1Char('0'));
      QString SS = locale.toString(sec, formatChar.toLatin1(), precision);
      if (SS.length() < 2 || SS.at(1) == '.')
        SS.prepend('0');
      return HH + MM + SS;
    }
  }

  QString text = "", postfix = "";
  int unitOrder = tickStepOrder;
  bool showTenths = true;

  // Předpona jednotky vychází z řádu o jedna větší, než řád kroku mřížky,
  // například 100 se zobrazí jako 0.1 kilo.
  unitOrder++;

  // Pokud je řád o jedna menší než násobek 3 (desetiny, stovky),
  // je nutné zobrazit desetiny (0.1,0.2kilo), jindy ne (100, 100kilo)
  // +3000 je kvůli tomu aby to nebylo záporné číslo
  if (((tickStepOrder + 3000) % 3) != 2)
    showTenths = false;

  if (qFuzzyIsNull(tick)) {
    tick = 0;
    postfix = " ";
  } else if (unitOrder >= 21)
    goto noPrefix;
  else if (unitOrder >= 18) {
    postfix = " E";
    tick /= 1e18;
  } else if (unitOrder >= 15) {
    postfix = " P";
    tick /= 1e15;
  } else if (unitOrder >= 12) {
    postfix = " T";
    tick /= 1e12;
  } else if (unitOrder >= 9) {
    postfix = " G";
    tick /= 1e9;
  } else if (unitOrder >= 6) {
    postfix = " M";
    tick /= 1e6;
  } else if (unitOrder >= 3) {
    postfix = " k";
    tick /= 1e3;
  } else if (unitOrder >= 0) {
    postfix = " ";
  } else if (unitOrder >= -3) {
    postfix = " m";
    tick /= 1e-3;
  } else if (unitOrder >= -6) {
    postfix = " " + QString::fromUtf8("\xc2\xb5"); // mikro
    tick /= 1e-6;
  } else if (unitOrder >= -9) {
    postfix = " n";
    tick /= 1e-9;
  } else if (unitOrder >= -12) {
    postfix = " p";
    tick /= 1e-12;
  } else if (unitOrder >= -15) {
    postfix = " f";
    tick /= 1e-15;
  } else if (unitOrder >= -18) {
    postfix = " a";
    tick /= 1e-18;
  } else {
    goto noPrefix;
  }
  text = QString::number(tick, 'f', showTenths ? 1 : 0);

  text.append(postfix);
  text.append(unit);
  return text;
}

void MyAxisTickerWithUnit::setTickStep(double value) {
  tickStepOrder = intLog10(value);

  this->QCPAxisTickerFixed::setTickStep(value);
}
