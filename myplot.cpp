#include "myplot.h"

MyPlot::MyPlot(QWidget *parent) : QCustomPlot(parent) {
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
}

void MyPlot::updateCursors(double *cursorPositions) {
  for (int i = 0; i < 4; i++) {
    if (i < 2) {
      cursors.at(i)->start->setCoords(cursorPositions[i], 0);
      cursors.at(i)->end->setCoords(cursorPositions[i], 1);
    } else {
      cursors.at(i)->start->setCoords(0, cursorPositions[i]);
      cursors.at(i)->end->setCoords(1, cursorPositions[i]);
    }
  }
}

void MyPlot::initcursors() {
  QPen cursorpen;
  cursorpen.setColor(Qt::black);
  for (int i = 0; i < 4; i++) {
    cursors.append(new QCPItemLine(this));
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
    connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onXRangeChanged(QCPRange)));
    connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onYRangeChanged(QCPRange)));
  } else {
    this->setInteraction(QCP::iRangeDrag, false);
    this->setInteraction(QCP::iRangeZoom, false);
    disconnect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onXRangeChanged(QCPRange)));
    disconnect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(onYRangeChanged(QCPRange)));
  }
}

void MyPlot::setCursorsAccess(bool allowed) {
  for (int i = 0; i < 4; i++)
    cursors.at(i)->setVisible(allowed);
  iHaveCursors = allowed;
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
