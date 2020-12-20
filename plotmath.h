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

#ifndef PLOTMATH_H
#define PLOTMATH_H

#include "enums_defines_constants.h"
#include <QDebug>
#include <QObject>
#include <QThread>

#include "qcustomplot.h"

class PlotMath : public QObject {
  Q_OBJECT
public:
  explicit PlotMath(QObject *parent = nullptr);
  ~PlotMath();

private:
  QVector<QSharedPointer<QCPGraphDataContainer>> firsts, seconds = QVector<QSharedPointer<QCPGraphDataContainer>>();
  QSharedPointer<QCPGraphDataContainer> XYx, XYy;
  MathOperations::enumetrator operations[MATH_COUNT];
public slots:
  void addMathData(int mathNumber, bool isFirst, QSharedPointer<QCPGraphDataContainer> in, bool shouldIgnorePause);
  void clearMath(int math);
  void clearXY();
  void addXYData(bool isFirst, QSharedPointer<QCPGraphDataContainer> in, bool shouldIgnorePause);
  void resetMath(int mathNumber, MathOperations::enumetrator mode, QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2);
  void resetXY(QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2);
signals:
  void sendResult(int chNumber, QSharedPointer<QCPGraphDataContainer> result, bool ignorePause);
  void sendResultXY(QSharedPointer<QCPCurveDataContainer> result, bool ignorePause);
  void sendMessage(QString header, QByteArray message, MessageLevel::enumerator type = MessageLevel::error, MessageTarget::enumerator target = MessageTarget::serial1);
};

#endif // PLOTMATH_H
