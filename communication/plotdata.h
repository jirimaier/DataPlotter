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
#include <QElapsedTimer>
#include <QObject>
#include <QThread>
#include <QTime>
#include <QVector>
#include <QWidget>
#include <QtMath>

#include "global.h"
#include "plots/qcustomplot.h"

class PlotData : public QObject {
  Q_OBJECT
public:
  explicit PlotData(QObject *parent = nullptr);
  ~PlotData();

  int getUpdatesPerSecond() const;
  void setUpdatesPerSecond(int newUpdatesPerSecond);

private:
  QTime qTime;
  QElapsedTimer elapsedTime;
  bool timerRunning;

  QTimer *updatesCounter;
  QMap<int, int> updatesCounters;

  unsigned int logicTargets[LOGIC_GROUPS - 1];
  unsigned int logicBits[LOGIC_GROUPS - 1];
  unsigned int mathFirsts[MATH_COUNT];
  unsigned int mathSeconds[MATH_COUNT];

  bool averagerEnabled = false;

  // unsigned int xyFirst, xySecond;
  double getValue(QPair<ValueType, QByteArray> value, bool &isok);
  OutputLevel::enumOutputLevel debugLevel = OutputLevel::info;
  double lastTime;
  double defaultTimestep = 1;
  void sendMessageIfAllowed(QString header, QByteArray message, MessageLevel::enumMessageLevel type);
  uint32_t getBits(QPair<ValueType, QByteArray> value);
  double unitToMultiple(char unit);

public slots:
  void addPoint(QList<QPair<ValueType, QByteArray>> data);
  void addLogicPoint(QPair<ValueType, QByteArray> timeArray, QPair<ValueType, QByteArray> valueArray, unsigned int bits);
  void addChannel(QPair<ValueType, QByteArray> data, unsigned int ch, QPair<ValueType, QByteArray> timeRaw, int zeroIndex, int bits, QPair<ValueType, QByteArray> min, QPair<ValueType, QByteArray> max);
  void addLogicChannel(QPair<ValueType, QByteArray> data, QPair<ValueType, QByteArray> timeRaw, int bits, int zeroIndex);

  void reset();

  /// Nastavý úroveň výpisu
  void setDebugLevel(OutputLevel::enumOutputLevel debugLevel) { this->debugLevel = debugLevel; }

  void setDigitalChannel(int logicGroup, int ch);

  void setLogicBits(int target, int bits);

  void setMathFirst(int math, int ch);
  void setMathSecond(int math, int ch);

  void setAverager(bool enabled) { averagerEnabled = enabled; }

private slots:
  void updateCounterTimer();

signals:
  /// Pošle zprávu do výpisu
  void sendMessage(QString header, QByteArray message, MessageLevel::enumMessageLevel type, MessageTarget::enumMessageTarget target = MessageTarget::serial1);

  /// Předá data do grafu
  void addVectorToPlot(int ch, QSharedPointer<QCPGraphDataContainer>, bool isMath = false);

  /// Předá data do grafu
  void addPointToPlot(int ch, double time, double value, bool append);
  void clearLogic(int group, int fromBit);
  void addMathData(int mathNumber, bool isFirst, QSharedPointer<QCPGraphDataContainer> in, bool shouldIgnorePause = false);
  void addDataToAverager(int chID, double samplingRate, QSharedPointer<QCPGraphDataContainer> data);
  void addPointToAverager(int ch, double time, double value, bool append);
  void setExpectedRange(int chID, bool known, double min, double max);
  void plotRecommendedXAxisTypeChanged(HAxisType::enumHAxisType recommandedAxisType);
  void dataRateUpdate(int perSec);
};

#endif // PLOTTING_H
