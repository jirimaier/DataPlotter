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

#ifndef MYXYPLOT_H
#define MYXYPLOT_H

#include "myplot.h"

class MyXYPlot : public MyPlot {
  Q_OBJECT
public:
  explicit MyXYPlot(QWidget *parent = nullptr);
  ~MyXYPlot();
  void newData(QSharedPointer<QCPCurveDataContainer> data, bool ignorePause);
  QByteArray exportCSV(char separator, char decimal, int precision);
  QCPCurve *graphXY;

private:
  PlotStatus::enumerator plottingStatus = PlotStatus::run;
  QSharedPointer<QCPCurveDataContainer> pauseBuffer;
  void resume();
  void pause();
  double timeRange = 100;
  double range = 10;
  double center = 10;
  bool useRange = false;
  bool autoSize = true;

  void autoset();

public slots:
  void setAutoSize(bool en);
  void clear();
  void setStyle(int style);
  void togglePause();
  void setTimeRange(double interval);
  void setUseTimeRange(bool en);
  void setCenter(double value);
  void setRange(double value);
};

#endif // MYXYPLOT_H
