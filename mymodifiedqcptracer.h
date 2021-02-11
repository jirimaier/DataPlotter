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

#ifndef MYMODIFIEDQCPTRACER_H
#define MYMODIFIEDQCPTRACER_H

#include "qcustomplot.h"

class MyModifiedQCPTracer : public QCPItemTracer {
  Q_OBJECT
public:
  explicit MyModifiedQCPTracer(QCustomPlot *parentPlot) : QCPItemTracer(parentPlot) { verticalAxis = parentPlot->yAxis; }

  void setCurve(QCPCurve *curve) { mCurve = curve; }
  void setYAxis(QCPAxis *vAxis) { verticalAxis = vAxis; }
  void setPoint(QPoint point) {
    mPoint = point;
    setGraphKey(point.x());
  }

  void updatePosition();

  int sampleNumber() { return posIndex; }

protected:
  QCPCurve *mCurve;
  QPoint mPoint;
  int posIndex = 0;
  QCPAxis *verticalAxis;
};

#endif // MYMODIFIEDQCPTRACER_H
