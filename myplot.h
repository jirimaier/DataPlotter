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

#ifndef MYPLOT_H
#define MYPLOT_H

#include "enums_defines_constants.h"
#include "myaxistickerwithunit.h"
#include "mymodifiedqcptracer.h"
#include "qcustomplot.h"
#include <QWidget>

class MyPlot : public QCustomPlot {
  Q_OBJECT
private:
  void updateGridX();
  void updateGridY();
  int xGridHint = -3;
  int yGridHint = -3;
  double lastGridX = 0, lastGridY = 0;
  QSharedPointer<QCPAxisTickerTime> timeTickerX, longTimeTickerX;
  QSharedPointer<MyAxisTickerWithUnit> unitTickerX, unitTickerY;
  void setVerticalDiv(double value);
  void setHorizontalDiv(double value);
  bool isFreeMove = false;
  enum TracerTextPos { TR, TL, BR, BL } tracerTextPos;
  void changeTracerTextPosition(TracerTextPos pos);
  QColor transparentWhite = QColor::fromRgbF(1, 1, 1, 0.8);
  QString xUnit, yUnit;

public:
  explicit MyPlot(QWidget *parent = nullptr);
  void updateCursor(int cursor, double cursorPosition, QString label);
  void setCursorVisible(int cursor, bool visible);
  double getVDiv() { return unitTickerY->tickStep(); }
  double getHDiv() { return unitTickerX->tickStep(); }
  QImage toPNG() { return (this->toPixmap().toImage()); };
  void setXUnit(QString unit) {
    xUnit = unit;
    unitTickerX->unit = unit;
  }
  void setYUnit(QString unit) {
    yUnit = unit;
    unitTickerY->unit = unit;
  }
  QString getXUnit() { return xUnit; }
  QString getYUnit() { return yUnit; }

protected:
  QCPLayer *cursorLayer;
  QVector<QCPItemLine *> cursors;
  QVector<QCPItemText *> curNums, curKeys, curVals;
  MyModifiedQCPTracer *tracer;
  QCPItemText *tracerText;
  QCPLayer *tracerLayer;
  void initcursors();
  void initTracer();
  void setMouseControlls(bool enabled);
  bool mouseIsPressed = false;
  void checkIfTracerTextFits();
  enum MouseDrag { cursor1, cursor2, nothing, zeroline } mouseDrag = nothing; // Vyšší čísla jsou číslo kanálu pro nulo

protected slots:
  virtual void moveTracer(QMouseEvent *event) = 0;
  virtual void mouseReleased(QMouseEvent *);
  virtual void mousePressed(QMouseEvent *event);

public slots:
  void setShowVerticalValues(bool enabled);
  void setShowHorizontalValues(int type);
  void setXTitle(QString title);
  void setYTitle(QString title);
  void setGridHintX(int hint);
  void setGridHintY(int hint);
  void hideTracer();
  void enableMouseCursorControll(bool enabled);

private slots:
  void onXRangeChanged(QCPRange range);
  void onYRangeChanged(QCPRange range);

signals:
  void gridChanged();
  void moveCursor(int chid, int cursor, int sample);
};
#endif // MYPLOT_H
