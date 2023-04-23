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

#ifndef MYXYPLOT_H
#define MYXYPLOT_H

#include "myplot.h"

class MyXYPlot : public MyPlot {
  Q_OBJECT
 public:
  explicit MyXYPlot(QWidget* parent = nullptr);
  ~MyXYPlot();
  QByteArray exportCSV(char separator, char decimal, int precision);
  QCPCurve* graphXY;
  QString tUnit = "s";

 private:
  QSharedPointer<QCPCurveDataContainer> pauseBuffer;
  bool autoSize = true;
  void autoset();
  void updateTracerText();

  void setMouseCursorStyle(QMouseEvent* event);
 private slots:
  void mouseMoved(QMouseEvent* event);
  void mousePressed(QMouseEvent* event);

 public slots:
  void newData(QSharedPointer<QCPCurveDataContainer> data);
  void setAutoSize(bool en);
  void clear();
  void setStyle(int style);

 signals:
  void moveTimeCursorXY(Cursors::enumCursors cursor, double pos);
  void setCursorPosXY(Cursors::enumCursors cursor, double x, double y);
};

#endif // MYXYPLOT_H