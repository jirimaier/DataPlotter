#include "myplot.h"

MyPlot::MyPlot(QWidget *parent) : QCustomPlot(parent) { initcursors(); }

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
