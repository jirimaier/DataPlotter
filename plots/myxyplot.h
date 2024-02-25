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
  explicit MyXYPlot(QWidget *parent = nullptr);
  ~MyXYPlot();
  QByteArray exportCSV(char separator, char decimal, int precision);
  QCPCurve *graphXY;
  UnitOfMeasure tUnit = UnitOfMeasure("s");

  QColor getClr1() const;
  QColor getClr2() const;

private:
  QSharedPointer<QCPCurveDataContainer> pauseBuffer;
  bool autoSize = true;
  void autoset();
  void updateTracerText();
  bool rangeUnknown = true;

  void setMouseCursorStyle(QMouseEvent *event);
  QColor clr1, clr2;

private slots:
  void mouseMoved(QMouseEvent *event);
  void mousePressed(QMouseEvent *event);

public slots:
  void newData(QSharedPointer<QCPCurveDataContainer> data);
  void clear();
  void setStyle(int style);
  void setColor(QColor clr, int theme);

signals:
  void moveTimeCursorXY(Cursors::enumCursors cursor, double pos);
  void setCursorPosXY(Cursors::enumCursors cursor, double x, double y);

  // MyPlot interface
public:
  void setTheme(QColor fnt, QColor bck, int chClrThemeId);
};

#endif // MYXYPLOT_H
