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

#include "plotmath.h"

PlotMath::PlotMath(QObject *parent) : QObject(parent) {
  firsts.resize(MATH_COUNT);
  seconds.resize(MATH_COUNT);
  for (int i = 0; i < MATH_COUNT; i++) operations[i] = MathOperations::add;
}

PlotMath::~PlotMath() {}

void PlotMath::addMathData(int mathNumber, bool isFirst, QSharedPointer<QCPGraphDataContainer> in, bool shouldIgnorePause) {
  QSharedPointer<QCPGraphDataContainer> &first = firsts[mathNumber];
  QSharedPointer<QCPGraphDataContainer> &second = seconds[mathNumber];
  if (isFirst)
    first = in;
  else
    second = in;

  if (!first.isNull() && !second.isNull()) {
    if (first->size() != second->size()) {
      emit sendMessage(tr("Math error"), tr("Channels have different length, can not use math").toUtf8());
      first.clear();
      second.clear();
      return;
    }
    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
    for (int i = 0; i < first->size(); i++) {
      double resultSample = first->at(i)->value;
      if (operations[mathNumber] == MathOperations::add)
        resultSample += second->at(i)->value;
      else if (operations[mathNumber] == MathOperations::subtract)
        resultSample -= second->at(i)->value;
      else if (operations[mathNumber] == MathOperations::multiply)
        resultSample *= second->at(i)->value;
      else if (operations[mathNumber] == MathOperations::divide)
        resultSample /= second->at(i)->value;
      result->add(QCPGraphData(first->at(i)->key, resultSample));
    }
    emit sendResult(GlobalFunctions::getAnalogChId(mathNumber + 1, ChannelType::math), result, shouldIgnorePause);
    first.clear();
    second.clear();
  }
}

void PlotMath::clearMath(int math) {
  firsts[math - 1].clear();
  seconds[math - 1].clear();
}

void PlotMath::clearXY() {
  XYx.clear();
  XYy.clear();
}

void PlotMath::addXYData(bool isFirst, QSharedPointer<QCPGraphDataContainer> in, bool shouldIgnorePause) {
  if (isFirst)
    XYx = in;
  else
    XYy = in;

  if (!XYx.isNull() && !XYy.isNull()) {
    if (XYx->size() != XYy->size()) {
      emit sendMessage(tr("XY error"), tr("Channels have different length, can not use XY mode").toUtf8());
      XYx.clear();
      XYy.clear();
      return;
    }
    auto result = QSharedPointer<QCPCurveDataContainer>(new QCPCurveDataContainer());
    for (int i = 0; i < XYx->size(); i++) {
      result->add(QCPCurveData(XYx->at(i)->key, XYx->at(i)->value, XYy->at(i)->value));
    }
    emit sendResultXY(result, shouldIgnorePause);
    XYx.clear();
    XYy.clear();
  }
}

void PlotMath::resetMath(int mathNumber, MathOperations::enumerator mode, QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2) {
  operations[mathNumber - 1] = mode;
  seconds[mathNumber - 1].clear();                // Vymaže druhá data
  firsts[mathNumber - 1] = in1;                   // Vloží první data
  addMathData(mathNumber - 1, false, in2, true);  // Vloží druhá data, spočítá a pošle do grafu
}

void PlotMath::resetXY(QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2) {
  XYy.clear();                  // Vymaže druhá data
  XYx = in1;                    // Vloží první data
  addXYData(false, in2, true);  // Vloží druhá data, spočítá a pošle do grafu
}
