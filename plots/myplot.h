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

#ifndef MYPLOT_H
#define MYPLOT_H

#include "global.h"
#include "myaxistickerwithunit.h"
#include "mymodifiedqcptracer.h"
#include "plots/qcustomplot.h"
#include <QWidget>

class MyPlot : public QCustomPlot {
  Q_OBJECT
private:
  void updateGridX();
  void updateGridY();
  int xGridHint = -3;
  int yGridHint = -3;
  double lastGridX = 0, lastGridY = 0;
  void setVerticalDiv(double value);
  void setHorizontalDiv(double value);
  enum TracerTextPos { TR, TL, BR, BL } tracerTextPos;
  void changeTracerTextPosition(TracerTextPos pos);
  UnitOfMeasure xUnit, yUnit;

  bool clipRange(QCPRange &newRange, const QCPRange &limits);

  QTimer *regularUpdateTimer;

public:
  explicit MyPlot(QWidget *parent = nullptr);
  ~MyPlot();
  void updateTimeCursor(Cursors::enumCursors cursor, double cursorPosition, QString label, int graphIndex);
  void updateValueCursor(Cursors::enumCursors cursor, double cursorPosition, QString label, QCPAxis *relativeToAxis);
  void setValueCursorVisible(Cursors::enumCursors cursor, bool visible);
  void setTimeCursorVisible(Cursors::enumCursors cursor, bool visible);
  double getVDiv() { return unitTickerY->tickStep(); }
  double getHDiv() { return unitTickerX->tickStep(); }
  void setXUnit(UnitOfMeasure unit);
  void setYUnit(UnitOfMeasure unit);
  UnitOfMeasure getXUnit() { return xUnit; }
  UnitOfMeasure getYUnit() { return yUnit; }
  double getValueCursorPosition(Cursors::enumCursors cursor) { return cursorsVal.at(cursor)->start->coords().y(); }
  bool getValueCursorVisible(Cursors::enumCursors cursor) { return cursorsVal.at(cursor)->visible(); }
  double getTimeCursorPosition(Cursors::enumCursors cursor) { return cursorsKey.at(cursor)->start->coords().x(); }
  int keyToNearestSample(QCPGraph *graph, double keyCoord);

  QCPRange getMaxZoomX() const;
  void setMaxZoomX(const QCPRange &newMaxZoomX, bool reset = false);

  QCPRange getMaxZoomY() const;
  void setMaxZoomY(const QCPRange &newMaxZoomY, bool reset = false);

  virtual void setTheme(QColor fnt, QColor bck, int chClrThemeId);

protected:
  QCPLayer *cursorLayer;
  QVector<QCPItemLine *> cursorsKey, cursorsVal;
  QVector<QCPItemText *> curNums, curKeys, curVals;
  MyModifiedQCPTracer *tracer;
  QCPItemText *tracerText;
  QCPLayer *tracerLayer;
  void initcursors();
  void initTracer();
  void checkIfTracerTextFits();
  enum MouseDrag { cursorX1, cursorX2, cursorY1, cursorY2, nothing, zeroline } mouseDrag = nothing; // Vyšší čísla jsou číslo kanálu pro nulo
  int cur1Graph = -1, cur2Graph = -1;
  QCPAxis *cur1YAxis = yAxis, *cur2YAxis = yAxis;
  QCursor defaultMouseCursor = Qt::ArrowCursor;

  QSharedPointer<MyAxisTickerWithUnit> unitTickerX, unitTickerY;

  virtual void leaveEvent(QEvent *event);

  QCPRange maxZoomX = {-MAX_PLOT_ZOOMOUT, MAX_PLOT_ZOOMOUT};
  QCPRange maxZoomY = {-MAX_PLOT_ZOOMOUT, MAX_PLOT_ZOOMOUT};

  int chClrTheme = 1;

protected slots:
  virtual void mouseMoved(QMouseEvent *event) = 0;
  virtual void mouseReleased(QMouseEvent *);
  virtual void mousePressed(QMouseEvent *event) = 0;

public slots:
  void setShowVerticalValues(bool enabled);
  void setShowHorizontalValues(bool enabled);
  void setXTitle(QString title);
  void setYTitle(QString title);
  void setGridHintX(int hint);
  void setGridHintY(int hint);
  void hideTracer();
  void enableMouseCursorControll(bool enabled);

protected slots:
  virtual void onXRangeChanged(QCPRange newRange, QCPRange oldRange);
  virtual void onYRangeChanged(QCPRange newRange, QCPRange oldRange);

signals:
  void gridChanged();
  void moveTimeCursor(Cursors::enumCursors cursor, int sample, double value);
  void moveValueCursor(Cursors::enumCursors cursor, double value);
  void setCursorPos(int chid, Cursors::enumCursors cursor, int sample);
};
#endif // MYPLOT_H
