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

#include "myxyplot.h"

MyXYPlot::MyXYPlot(QWidget *parent) : MyPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  graphXY = new QCPCurve(this->xAxis, this->yAxis);
  this->xAxis->setRange(-100, 100);
  this->yAxis->setRange(-100, 100);
  setGridHintX(-3);
  setGridHintY(-3);
  tracer->setCurve(graphXY);
  this->setInteraction(QCP::iRangeDrag, true);
  this->setInteraction(QCP::iRangeZoom, true);
}

MyXYPlot::~MyXYPlot() {}

void MyXYPlot::newData(QSharedPointer<QCPCurveDataContainer> data) {
  graphXY->setData(data);
  if (autoSize)
    autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);

  // Přepsat text u traceru
  if (tracer->visible())
    updateTracerText();
}

void MyXYPlot::clear() {
  graphXY->data().data()->clear();
  rangeUnknown = true;
  setMaxZoomX(QCPRange(-10, 10), true);
  setMaxZoomY(QCPRange(-10, 10), true);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyXYPlot::setStyle(int style) {
  if (style == GraphStyle::line) {
    graphXY->setScatterStyle(QCPScatterStyle::ssNone);
    graphXY->setLineStyle(QCPCurve::lsLine);
  } else if (style == GraphStyle::point) {
    graphXY->setScatterStyle(POINT_STYLE);
    graphXY->setLineStyle(QCPCurve::lsNone);
  } else if (style == GraphStyle::linePoint) {
    graphXY->setScatterStyle(POINT_STYLE);
    graphXY->setLineStyle(QCPCurve::lsLine);
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyXYPlot::setColor(QColor clr, int theme) {
  if (theme == 1)
    clr1 = clr;
  if (theme == 2)
    clr2 = clr;
  if (theme == chClrTheme) {
    graphXY->setPen(clr);
    this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  }
}

void MyXYPlot::setTheme(QColor fnt, QColor bck, int chClrThemeId) {
  MyPlot::setTheme(fnt, bck, chClrThemeId);
  graphXY->setPen(chClrTheme == 1 ? clr1 : clr2);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

QByteArray MyXYPlot::exportCSV(char separator, char decimal, int precision) {
  if (graphXY->data()->isEmpty())
    return "";
  QByteArray output = (QString("X%1Y\n").arg(separator)).toUtf8();
  for (QCPCurveDataContainer::iterator it = graphXY->data()->begin(); it != graphXY->data()->end(); it++) {
    output.append(QString::number(it->key, 'f', precision).replace('.', decimal).toUtf8());
    output.append(separator);
    output.append(QString::number(it->value, 'f', precision).replace('.', decimal).toUtf8());
    output.append('\n');
  }
  return output;
}

void MyXYPlot::autoset() {
  bool foundrange, foundrange2;
  QCPRange yRange = graphXY->data()->valueRange(foundrange);
  QCPRange xRange = graphXY->data()->keyRange(foundrange2);
  if (foundrange && foundrange2) {
    yRange.lower = floorToNiceValue(yRange.lower);
    yRange.upper = ceilToNiceValue(yRange.upper);
    xRange.lower = floorToNiceValue(xRange.lower);
    xRange.upper = ceilToNiceValue(xRange.upper);
    setMaxZoomX(xRange, rangeUnknown || xRange.lower < maxZoomX.lower || xRange.upper > maxZoomX.upper);
    setMaxZoomY(yRange, rangeUnknown || yRange.lower < maxZoomY.lower || yRange.upper > maxZoomY.upper);
    rangeUnknown = false;
  } else
    rangeUnknown = true;
}

void MyXYPlot::updateTracerText() {
  QString tracerTextStr;
  if (true) // TODO
    tracerTextStr.append("X: " + floatToNiceString(tracer->position->key(), 4, true, false) + getXUnit().text + "\n");
  else
    tracerTextStr.append("X: " + QString::number(+tracer->position->key(), 'g', 4) + " " + getXUnit().text + "\n");

  if (true) // TODO
    tracerTextStr.append("Y: " + floatToNiceString(tracer->position->value(), 4, true, false) + getYUnit().text + "\n");
  else
    tracerTextStr.append("Y: " + QString::number(tracer->position->value(), 'g', 4) + " " + getYUnit().text + "\n");

  if (tUnit.isEmpty())
    tracerTextStr.append("t: " + QString::number(graphXY->data().data()->at(tracer->sampleNumber())->t, 'g', 4));
  else
    tracerTextStr.append("t: " + floatToNiceString(graphXY->data().data()->at(tracer->sampleNumber())->t, 4, true, false) + tUnit);

  tracerText->setText(tracerTextStr);
  checkIfTracerTextFits();
  tracerLayer->replot();
}

void MyXYPlot::mouseMoved(QMouseEvent *event) {
  if (mouseDrag == MouseDrag::nothing) {
    // Nic není taženo myší
    if ((unsigned int)graphXY->selectTest(event->pos(), false) < TRACER_MOUSE_DISTANCE) {
      tracer->setVisible(true);
      tracer->setPoint(event->pos());
      tracerText->setVisible(true);
      updateTracerText();
      this->QWidget::setCursor(defaultMouseCursor); // Cursor myši, ne ten grafový
    } else {
      if (tracer->visible())
        hideTracer();
      setMouseCursorStyle(event);
    }
  } else {
    if (tracer->visible())
      hideTracer();

    if (mouseDrag == MouseDrag::cursorY1)
      emit moveValueCursor(Cursors::Cursor1, yAxis->pixelToCoord(event->pos().y()));
    else if (mouseDrag == MouseDrag::cursorY2)
      emit moveValueCursor(Cursors::Cursor2, yAxis->pixelToCoord(event->pos().y()));
    else if (mouseDrag == MouseDrag::cursorX1)
      emit moveTimeCursorXY(Cursors::Cursor1, xAxis->pixelToCoord(event->pos().x()));
    else if (mouseDrag == MouseDrag::cursorX2)
      emit moveTimeCursorXY(Cursors::Cursor2, xAxis->pixelToCoord(event->pos().x()));
  }
}

void MyXYPlot::mousePressed(QMouseEvent *event) {
  if ((unsigned int)graphXY->selectTest(event->pos(), false) < TRACER_MOUSE_DISTANCE) {
    tracer->setPoint(event->pos());
    tracer->updatePosition();
    if (event->button() == Qt::RightButton) {
      mouseDrag = MouseDrag::cursorX2;
      emit setCursorPosXY(Cursors::Cursor2, tracer->position->key(), tracer->position->value());
    } else {
      mouseDrag = MouseDrag::cursorX1;
      emit setCursorPosXY(Cursors::Cursor1, tracer->position->key(), tracer->position->value());
    }
    return;
  }

  // Kursory svislé
  unsigned int cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsKey.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsKey.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsKey.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsKey.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist <= cur2dist) {
    if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
      mouseDrag = MouseDrag::cursorX1;
      this->setInteraction(QCP::iRangeDrag, false);
      return;
    }
  } else if (cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    mouseDrag = MouseDrag::cursorX2;
    this->setInteraction(QCP::iRangeDrag, false);
    return;
  }

  // Kursory vodorovné
  cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsVal.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsVal.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsVal.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsVal.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist <= cur2dist) {
    if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
      mouseDrag = MouseDrag::cursorY1;
      this->setInteraction(QCP::iRangeDrag, false);
      return;
    }
  } else if (cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    mouseDrag = MouseDrag::cursorY2;
    this->setInteraction(QCP::iRangeDrag, false);
    return;
  }
}

void MyXYPlot::setMouseCursorStyle(QMouseEvent *event) {
  // Kursory svislé
  unsigned int cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsKey.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsKey.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsKey.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsKey.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE || cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    this->QWidget::setCursor(Qt::SizeHorCursor); // Cursor myši, ne ten grafový
    return;
  }

  // Kursory vodorovné
  cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsVal.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsVal.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsVal.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsVal.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE || cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    this->QWidget::setCursor(Qt::SizeVerCursor); // Cursor myši, ne ten grafový
    return;
  }

  // Nic
  this->QWidget::setCursor(defaultMouseCursor); // Cursor myši, ne ten grafový
}

QColor MyXYPlot::getClr2() const { return clr2; }

QColor MyXYPlot::getClr1() const { return clr1; }
