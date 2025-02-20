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

#ifndef AVERAGER_H
#define AVERAGER_H

#include <QObject>
#include "plots/qcustomplot.h"
#include "global.h"

class Averager : public QObject {
  Q_OBJECT
 public:
  explicit Averager(QObject* parent = nullptr);

 private:
  QVector<QList<QVector<double>>> listsChannels;
  QVector<QVector<double>> channelsResults;
  int averageCount[ANALOG_COUNT];
  double lastChannelSamplingPeriod[ANALOG_COUNT];
  QVector<QList<QPair<double, double>>> points;
  double pointResults[ANALOG_COUNT];

 public slots:
  void reset();
  void setCount(int chID, int count);
  void newDataVector(int chID, double timeStep, QSharedPointer<QCPGraphDataContainer> data);
  void newDataPoint(int chID, double time, double value, bool append);

 signals:
  /// Předá data do grafu
  void addVectorToPlot(int ch, QSharedPointer<QCPGraphDataContainer>, bool isMath = false);
  /// Předá data do grafu
  void addPointToPlot(int ch, double time, double value, bool append);

};

#endif // AVERAGER_H
