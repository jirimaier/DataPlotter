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

#include "myplot.h"

MyPlot::MyPlot(QWidget* parent) : QCustomPlot(parent) {
  this->setAntialiasedElement(QCP::AntialiasedElement::aePlottables);
  this->addLayer("cursorLayer", 0, limAbove);
  this->addLayer("tracerLayer", 0, limAbove);
  cursorLayer = this->layer("cursorLayer");
  tracerLayer = this->layer("tracerLayer");
  cursorLayer->setMode(QCPLayer::lmBuffered);
  tracerLayer->setMode(QCPLayer::lmBuffered);
  unitTickerX = QSharedPointer<MyAxisTickerWithUnit>(new MyAxisTickerWithUnit);
  unitTickerY = QSharedPointer<MyAxisTickerWithUnit>(new MyAxisTickerWithUnit);
  timeTickerX = QSharedPointer<QCPAxisTickerTime>(new QCPAxisTickerTime);
  longTimeTickerX = QSharedPointer<QCPAxisTickerTime>(new QCPAxisTickerTime);
  timeTickerX->setTimeFormat("%m:%s");
  longTimeTickerX->setTimeFormat("%h:%m:%s");
  unitTickerX->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  unitTickerY->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  timeTickerX->setTickStepStrategy(QCPAxisTickerTime::tssMeetTickCount);
  longTimeTickerX->setTickStepStrategy(QCPAxisTickerTime::tssMeetTickCount);
  this->xAxis->setNumberFormat("gb"); // Formát s hezkým 10^něco
  this->yAxis->setNumberFormat("gb"); // Formát s hezkým 10^něco
  this->xAxis->setTicker(unitTickerX);
  this->yAxis->setTicker(unitTickerY);

  initcursors();

  initTracer();

  connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onXRangeChanged(QCPRange)));
  connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onYRangeChanged(QCPRange)));
  connect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
  connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressed(QMouseEvent*)));
  connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleased(QMouseEvent*)));
}

void MyPlot::onXRangeChanged(QCPRange range) {
  bool changed = false;
  if (range.lower < -MAX_PLOT_ZOOMOUT) {
    range.lower = -MAX_PLOT_ZOOMOUT;
    changed = true;
  }
  if (range.upper > MAX_PLOT_ZOOMOUT) {
    range.upper = MAX_PLOT_ZOOMOUT;
    changed = true;
  }
  if (changed)
    this->xAxis->setRange(range);
  updateGridX();
}

void MyPlot::initTracer() {
  tracer = new MyModifiedQCPTracer(this);
  tracerText = new QCPItemText(this);
  tracer->setLayer(tracerLayer);
  tracer->setStyle(QCPItemTracer::tsPlus);
  tracer->setSize(50);
  tracer->setVisible(false);
  tracerText->setVisible(false);
  tracerText->setLayer(tracerLayer);
  tracerText->position->setParentAnchor(tracer->position);
  tracerText->setTextAlignment(Qt::AlignLeft);
  tracerText->setPadding(QMargins(2, 2, 2, 2));
  changeTracerTextPosition(TR);
  tracerText->setBrush(transparentWhite);
  tracerText->setFont(QFont("Courier New"));
  tracerText->setClipToAxisRect(false);
}

void MyPlot::onYRangeChanged(QCPRange range) {
  bool changed = false;
  if (range.lower < -MAX_PLOT_ZOOMOUT) {
    range.lower = -MAX_PLOT_ZOOMOUT;
    changed = true;
  }
  if (range.upper > MAX_PLOT_ZOOMOUT) {
    range.upper = MAX_PLOT_ZOOMOUT;
    changed = true;
  }
  if (changed)
    this->yAxis->setRange(range);
  updateGridY();
}

void MyPlot::updateTimeCursor(Cursors::enumCursors cursor, double cursorPosition, QString label, QCPGraph* graph) {
  cursorsKey.at(cursor)->start->setCoords(cursorPosition, 0);
  cursorsKey.at(cursor)->end->setCoords(cursorPosition, 1);
  curKeys.at(cursor)->setText(label);
  cursorLayer->replot();
  if (cursor == Cursors::Cursor1)
    cur1Graph = graph; else cur2Graph = graph;
}

void MyPlot::updateValueCursor(Cursors::enumCursors cursor, double cursorPosition, QString label, QCPAxis* relativeToAxis) {
  cursorsVal.at(cursor)->start->setCoords(0, cursorPosition);
  cursorsVal.at(cursor)->end->setCoords(1, cursorPosition);
  cursorsVal.at(cursor)->start->setAxes(xAxis, relativeToAxis);
  cursorsVal.at(cursor)->end->setAxes(xAxis, relativeToAxis);
  curVals.at(cursor)->setText(label);
  cursorLayer->replot();
  if (cursor == Cursors::Cursor1)
    cur1YAxis = relativeToAxis; else cur2YAxis = relativeToAxis;
}

void MyPlot::setTimeCursorVisible(Cursors::enumCursors cursor, bool visible) {
  if (cursorsKey.at(cursor)->visible() != visible) {
    cursorsKey.at(cursor)->setVisible(visible);
    curNums.at(cursor)->setVisible(visible);
    curKeys.at(cursor)->setVisible(visible);
    cursorLayer->replot();
  }
}

void MyPlot::setValueCursorVisible(Cursors::enumCursors cursor, bool visible) {
  if (cursorsVal.at(cursor)->visible() != visible) {
    cursorsVal.at(cursor)->setVisible(visible);
    curVals.at(cursor)->setVisible(visible);
    cursorLayer->replot();
  }
}

void MyPlot::updateGridX() {
  double newGrid = logaritmicSettings[MAX(indexOfStandardValuesCeil(xAxis->range().upper - xAxis->range().lower) + xGridHint, 0)];
  if (newGrid != lastGridX) {
    lastGridX = newGrid;
    setHorizontalDiv(newGrid);
    replot();
    emit gridChanged();
  }
}

void MyPlot::updateGridY() {
  double newGrid = logaritmicSettings[MAX(indexOfStandardValuesCeil(yAxis->range().upper - yAxis->range().lower) + yGridHint, 0)];
  if (newGrid != lastGridY) {
    lastGridY = newGrid;
    setVerticalDiv(newGrid);
    replot();
    emit gridChanged();
  }
}

void MyPlot::initcursors() {
  QPen cursorPen;
  cursorPen.setColor(Qt::black);
  cursorPen.setStyle(Qt::SolidLine);

  for (int i = 0; i < 2; i++) {
    // Svislí kursor
    auto line = new QCPItemLine(this);

    line->start->setTypeX(QCPItemPosition::ptPlotCoords);
    line->start->setTypeY(QCPItemPosition::ptViewportRatio);
    line->end->setTypeX(QCPItemPosition::ptPlotCoords);
    line->end->setTypeY(QCPItemPosition::ptViewportRatio);
    line->start->setCoords(0, 0);
    line->end->setCoords(0, 1);
    line->setVisible(false);
    line->setLayer(cursorLayer);
    line->setPen(cursorPen);
    cursorsKey.append(line);

    auto curNum = new QCPItemText(this);
    curNum->setLayer(cursorLayer);
    curNum->setText(i ? QString::fromUtf8("2") : QString::fromUtf8("1"));
    // Číslo v kroužku (Není moc hezké)
    // curNum->setText(i ? QString::fromUtf8("\xe2\x91\xa1") : QString::fromUtf8("\xe2\x91\xa0"));
    curNum->setTextAlignment(Qt::AlignRight);
    curNum->setPositionAlignment(Qt::AlignTop | Qt::AlignRight);
    curNum->position->setParentAnchorX(line->start);
    curNum->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
    curNum->position->setCoords(-2, 0);
    curNum->setVisible(false);
    curNum->setBrush(transparentWhite);
    curNum->setPadding(QMargins(2, 2, 2, 2));
    curNums.append(curNum);

    auto curKey = new QCPItemText(this);
    curKey->setLayer(cursorLayer);
    curKey->setTextAlignment(Qt::AlignLeft);
    curKey->setPositionAlignment(Qt::AlignTop | Qt::AlignLeft);
    curKey->position->setParentAnchorX(line->start);
    curKey->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
    curKey->position->setCoords(4, 0);
    curKey->setVisible(false);
    curKey->setBrush(transparentWhite);
    curKey->setPadding(QMargins(2, 2, 2, 2));
    curKeys.append(curKey);

  }
  for (int i = 0; i < 2; i++) {
    // Vodorovný kursor
    auto line = new QCPItemLine(this);
    line->start->setTypeX(QCPItemPosition::ptViewportRatio);
    line->start->setTypeY(QCPItemPosition::ptPlotCoords);
    line->end->setTypeX(QCPItemPosition::ptViewportRatio);
    line->end->setTypeY(QCPItemPosition::ptPlotCoords);
    line->start->setCoords(0, 0);
    line->end->setCoords(1, 0);
    line->setVisible(false);
    line->setLayer(cursorLayer);
    line->setPen(cursorPen);
    cursorsVal.append(line);

    auto curVal = new QCPItemText(this);
    curVal->setLayer(cursorLayer);
    curVal->setTextAlignment(Qt::AlignRight);
    curVal->setPositionAlignment(Qt::AlignBottom | Qt::AlignRight);
    curVal->position->setParentAnchorY(line->start);
    curVal->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
    curVal->position->setCoords(1, -2);
    curVal->setVisible(false);
    curVal->setBrush(transparentWhite);
    curVal->setPadding(QMargins(2, 2, 2, 2));
    curVals.append(curVal);
  }
}

void MyPlot::setMouseControlls(bool enabled) {
  if (enabled) {
    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);
    //defaultMouseCursor = Qt::SizeAllCursor;
  } else {
    this->setInteraction(QCP::iRangeDrag, false);
    this->setInteraction(QCP::iRangeZoom, false);
    //defaultMouseCursor = Qt::ArrowCursor;
  }
  isFreeMove = enabled;
}

void MyPlot::checkIfTracerTextFits() {
  int clearTop = tracer->position->pixelPosition().y();
  int clearRight = width() - tracer->position->pixelPosition().x();

  // takhle spočítaná šířka neodpovídá těm vypočteným vzdálenostem, nevím proč :-(
  int textW = tracerText->bottomRight->pixelPosition().x() - tracerText->topLeft->pixelPosition().x();
  int textH = tracerText->bottomRight->pixelPosition().y() - tracerText->topLeft->pixelPosition().y();

  // int textH = 30;
  // int textW = 50;

  bool topok = (textH <= clearTop);
  bool rightok = (textW <= clearRight);

  if (topok && rightok) {
    if (!(tracerTextPos == TR))
      changeTracerTextPosition(TR);
  } else {
    int clearBottom = height() - tracer->position->pixelPosition().y();
    int clearLeft = tracer->position->pixelPosition().x();

    bool bottomok = (textH <= clearBottom);
    bool leftok = (textW <= clearLeft);

    if (bottomok && leftok) {
      if (!(tracerTextPos == BL))
        changeTracerTextPosition(BL);
    } else if (bottomok && rightok) {
      if (!(tracerTextPos == BR))
        changeTracerTextPosition(BR);
    } else if (topok && leftok) {
      if (!(tracerTextPos == TL))
        changeTracerTextPosition(TL);
    }
  }
}

int MyPlot::keyToNearestSample(QCPGraph* mGraph, double keyCoord) {
  // Převzato z funkce pro originální tracer v QCustomPlot a upraveno

  auto begin =  mGraph->data()->begin();

  QCPGraphDataContainer::const_iterator it = mGraph->data()->findBegin(keyCoord);
  if (it != mGraph->data()->constEnd()) // mGraphKey is not exactly on last iterator,
    // but somewhere between iterators
  {
    QCPGraphDataContainer::const_iterator prevIt = it;
    ++it; // won't advance to constEnd because we handled that case
    // (mGraphKey >= last->key) before

    // find iterator with key closest to mGraphKey:
    if (keyCoord < (prevIt->key + it->key) * 0.5)
      return prevIt - begin; // Vrátí index vzorku
    else
      return it -  begin; // Vrátí index vzorku
  } else // mGraphKey is exactly on last iterator (should actually be
    // caught when comparing first/last keys, but this is a
    // failsafe for fp uncertainty)
    return it - begin;  // Vrátí index vzorku
}

void MyPlot::mouseReleased(QMouseEvent* event) {
  Q_UNUSED(event);
  mouseDrag = MouseDrag::nothing;
  if (isFreeMove)
    this->setInteraction(QCP::iRangeDrag, true);

}

void MyPlot::mousePressed(QMouseEvent* event) {
  mouseMoved(event);
}

void MyPlot::setGridHintX(int hint) {
  xGridHint = hint;
  updateGridX();
}

void MyPlot::setGridHintY(int hint) {
  yGridHint = hint;
  updateGridY();
}

void MyPlot::hideTracer() {
  tracer->setVisible(false);
  tracerText->setVisible(false);
  tracerLayer->replot();
}

void MyPlot::enableMouseCursorControll(bool enabled) {
  if (enabled) {
    connect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
    connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressed(QMouseEvent*)));
    connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleased(QMouseEvent*)));
  } else {
    disconnect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
    disconnect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressed(QMouseEvent*)));
    disconnect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleased(QMouseEvent*)));
    this->setCursor(defaultMouseCursor);
  }
}

void MyPlot::setVerticalDiv(double value) { unitTickerY->setTickStep(value);}

void MyPlot::setHorizontalDiv(double value) {
  unitTickerX->setTickStep(value);
  timeTickerX->setTickCount((xAxis->range().upper - xAxis->range().lower) / value);
  longTimeTickerX->setTickCount((xAxis->range().upper - xAxis->range().lower) / value);
}

void MyPlot::changeTracerTextPosition(MyPlot::TracerTextPos pos) {
  if (pos == TR) {
    tracerText->position->setCoords(3, -3);
    tracerText->setPositionAlignment(Qt::AlignBottom | Qt::AlignLeft);
  } else if (pos == TL) {
    tracerText->position->setCoords(-3, -3);
    tracerText->setPositionAlignment(Qt::AlignBottom | Qt::AlignRight);
  } else if (pos == BL) {
    tracerText->position->setCoords(-3, 3);
    tracerText->setPositionAlignment(Qt::AlignTop | Qt::AlignRight);
  } else {
    tracerText->position->setCoords(3, 3);
    tracerText->setPositionAlignment(Qt::AlignTop | Qt::AlignLeft);
  }
  tracerTextPos = pos;
}

void MyPlot::setShowVerticalValues(bool enabled) {
  this->yAxis->setTicks(enabled);
  this->yAxis->setBasePen(enabled ? Qt::SolidLine : Qt::NoPen);
  replot();
}

void MyPlot::setShowHorizontalValues(int type) {
  bool enabled = type != 0;
  this->xAxis->setTicks(enabled);
  this->xAxis->setBasePen(enabled ? Qt::SolidLine : Qt::NoPen);

  if (enabled) {
    if (type == HAxisType::normal)
      this->xAxis->setTicker(unitTickerX);
    if (type == HAxisType::MS)
      this->xAxis->setTicker(timeTickerX);
    if (type == HAxisType::HMS)
      this->xAxis->setTicker(longTimeTickerX);
  }
  replot();
}

void MyPlot::setXTitle(QString title) {
  this->xAxis->setLabel(title);
  replot();
}

void MyPlot::setYTitle(QString title) {
  this->yAxis->setLabel(title);
  replot();
}
