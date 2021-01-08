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

#ifndef PLOTTING_H
#define PLOTTING_H

#include <QDebug>
#include <QObject>
#include <QThread>
#include <QVector>
#include <QWidget>
#include <QtMath>

#include "enums_defines_constants.h"
#include "qcustomplot.h"

using namespace ValueType;
class PlotData : public QObject {
  Q_OBJECT
public:
  explicit PlotData(QObject *parent = nullptr);
  ~PlotData();
  void init();

private:
  unsigned int logicTargets[LOGIC_GROUPS];
  unsigned int logicBits[LOGIC_GROUPS];
  unsigned int mathFirsts[MATH_COUNT];
  unsigned int mathSeconds[MATH_COUNT];
  // unsigned int xyFirst, xySecond;
  double getValue(QByteArray number, ValueType::enumerator type);
  ValueType::enumerator getType(QByteArray array);
  OutputLevel::enumerator debugLevel = OutputLevel::info;
  double lastTimes[ANALOG_COUNT];
  double defaultTimestep = 1;
  void sendMessageIfAllowed(QString header, QByteArray message, MessageLevel::enumerator type);
  double arrayToDouble(QByteArray &array, bool &isok);
  uint32_t getBits(QByteArray data, ValueType::enumerator type);

public slots:
  void addPoint(QByteArrayList data);
  void addChannel(QByteArray data, unsigned int ch, QByteArray timeRaw, int bits, QByteArray min, QByteArray max);
  void reset();
  /// Nastavý úroveň výpisu
  void setDebugLevel(OutputLevel::enumerator debugLevel) { this->debugLevel = debugLevel; }

  void setDigitalChannel(int logicGroup, int ch);

  void setLogicBits(int target, int bits);

  void setMathFirst(int math, int ch) { mathFirsts[math - 1] = ch; }
  void setMathSecond(int math, int ch) { mathSeconds[math - 1] = ch; }
  // void setXYFirst(int ch) { xyFirst = ch; }
  // void setXYSecond(int ch) { xySecond = ch; }

signals:
  /// Pošle zprávu do výpisu
  void sendMessage(QString header, QByteArray message, MessageLevel::enumerator type, MessageTarget::enumerator target = MessageTarget::serial1);

  /// Předá data do grafu
  void addVectorToPlot(int ch, QSharedPointer<QCPGraphDataContainer>, bool isMath = false);

  /// Předá data do grafu
  void addPointToPlot(int ch, double time, double value, bool append);

  void clearLogic(int group, int fromBit);

  void addMathData(int mathNumber, bool isFirst, QSharedPointer<QCPGraphDataContainer> in, bool shouldIgnorePause = false);
  // void addXYData(bool isFirst, QSharedPointer<QCPGraphDataContainer> in, bool shouldIgnorePause = false);

  void clearXY();
};

#endif // PLOTTING_H
