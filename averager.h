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

#ifndef AVERAGER_H
#define AVERAGER_H

#include <QObject>
#include "qcustomplot.h"

class Averager : public QObject {
  Q_OBJECT
 public:
  explicit Averager(QObject* parent = nullptr);

 private:
  QList<QCPGraphDataContainer> list;
  int count = 5;
  int channel;

 public slots:
  void reset(int chID, bool enabled);
  void setCount(int count);
  void newDataVector(QSharedPointer<QCPGraphDataContainer> data);

 signals:
  /// Předá data do grafu
  void addVectorToPlot(int ch, QSharedPointer<QCPGraphDataContainer>, bool isMath = false);

};

#endif // AVERAGER_H
