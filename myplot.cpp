#include "myplot.h"

MyPlot::MyPlot(QWidget *parent) : QCustomPlot(parent) {
  // setNoAntialiasingOnDrag(true);
  // setInteraction(QCP::iSelectPlottables, true);
  this->addLayer("cursorLayer", 0, limAbove);
  cursorLayer = this->layer("cursorLayer");
  cursorLayer->setMode(QCPLayer::lmBuffered);
  fixedTickerX = QSharedPointer<QCPAxisTickerFixed>(new QCPAxisTickerFixed);
  fixedTickerY = QSharedPointer<QCPAxisTickerFixed>(new QCPAxisTickerFixed);
  timeTickerX = QSharedPointer<QCPAxisTickerTime>(new QCPAxisTickerTime);
  longTimeTickerX = QSharedPointer<QCPAxisTickerTime>(new QCPAxisTickerTime);
  timeTickerX->setTimeFormat("%m:%s");
  longTimeTickerX->setTimeFormat("%h:%m:%s");
  fixedTickerX->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  fixedTickerY->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  timeTickerX->setTickStepStrategy(QCPAxisTickerTime::tssMeetTickCount);
  longTimeTickerX->setTickStepStrategy(QCPAxisTickerTime::tssMeetTickCount);
  this->xAxis->setTicker(fixedTickerX);
  this->yAxis->setTicker(fixedTickerY);
  initcursors();
  connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onXRangeChanged(QCPRange)));
  connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onYRangeChanged(QCPRange)));
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

void MyPlot::updateCursor(int cursor, double cursorPosition) {
  if (cursor < 2) {
    cursors.at(cursor)->start->setCoords(cursorPosition, 0);
    cursors.at(cursor)->end->setCoords(cursorPosition, 1);
  } else {
    cursors.at(cursor)->start->setCoords(0, cursorPosition);
    cursors.at(cursor)->end->setCoords(1, cursorPosition);
  }
  cursorLayer->replot();
}

void MyPlot::updateGridX() {
  double newGrid = Global::logaritmicSettings[MAX(GlobalFunctions::roundToStandardValue(xAxis->range().upper - xAxis->range().lower) + xGridHint, 0)];
  if (newGrid != lastGridX) {
    lastGridX = newGrid;
    setHorizontalDiv(newGrid);
    replot();
    emit gridChanged();
  }
}

void MyPlot::updateGridY() {
  double newGrid = Global::logaritmicSettings[MAX(GlobalFunctions::roundToStandardValue(yAxis->range().upper - yAxis->range().lower) + yGridHint, 0)];
  if (newGrid != lastGridY) {
    lastGridY = newGrid;
    setVerticalDiv(newGrid);
    replot();
    emit gridChanged();
  }
}

void MyPlot::initcursors() {
  QPen cursorpen;
  cursorpen.setColor(Qt::black);
  for (int i = 0; i < 4; i++) {
    auto line = new QCPItemLine(this);
    line->setLayer(cursorLayer);
    cursors.append(line);
    if (i < 2) {
      cursors.at(i)->start->setTypeX(QCPItemPosition::ptPlotCoords);
      cursors.at(i)->start->setTypeY(QCPItemPosition::ptViewportRatio);
      cursors.at(i)->end->setTypeX(QCPItemPosition::ptPlotCoords);
      cursors.at(i)->end->setTypeY(QCPItemPosition::ptViewportRatio);
    } else {
      cursors.at(i)->start->setTypeX(QCPItemPosition::ptViewportRatio);
      cursors.at(i)->start->setTypeY(QCPItemPosition::ptPlotCoords);
      cursors.at(i)->end->setTypeX(QCPItemPosition::ptViewportRatio);
      cursors.at(i)->end->setTypeY(QCPItemPosition::ptPlotCoords);
    }
    cursors.at(i)->setPen(cursorpen);
    cursors.at(i)->setVisible(false);
  }
}

void MyPlot::setMouseControlls(bool enabled) {
  if (enabled) {
    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);

  } else {
    this->setInteraction(QCP::iRangeDrag, false);
    this->setInteraction(QCP::iRangeZoom, false);
  }
}

void MyPlot::setGridHintX(int hint) {
  xGridHint = hint;
  updateGridX();
}

void MyPlot::setGridHintY(int hint) {
  yGridHint = hint;
  updateGridY();
}

void MyPlot::setVerticalDiv(double value) { fixedTickerY->setTickStep(value); }

void MyPlot::setHorizontalDiv(double value) {
  fixedTickerX->setTickStep(value);
  timeTickerX->setTickCount((xAxis->range().upper - xAxis->range().lower) / value);
  longTimeTickerX->setTickCount((xAxis->range().upper - xAxis->range().lower) / value);
}

void MyPlot::setShowVerticalValues(bool enabled) {
  this->yAxis->setTicks(enabled);
  this->yAxis->setBasePen(enabled ? Qt::SolidLine : Qt::NoPen);
}

void MyPlot::setShowHorizontalValues(int type) {
  bool enabled = type != 0;
  this->xAxis->setTicks(enabled);
  this->xAxis->setBasePen(enabled ? Qt::SolidLine : Qt::NoPen);

  if (enabled) {
    if (type == HAxisType::fixed)
      this->xAxis->setTicker(fixedTickerX);
    if (type == HAxisType::MS)
      this->xAxis->setTicker(timeTickerX);
    if (type == HAxisType::HMS)
      this->xAxis->setTicker(longTimeTickerX);
  }
}
