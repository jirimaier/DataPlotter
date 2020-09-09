#include "plotting.h"

Plotting::Plotting(QCustomPlot *plotWidget, QScrollBar *horizontalPos) {
  plot = plotWidget;
  this->horizontalPos = horizontalPos;
  for (int i = 1; i <= CHANNEL_COUNT; i++)
    channels.append(new Channel(i, plot, defaultColors[(i > 8) ? 7 : i - 1]));
  plot->setOpenGl(false);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(100); // ms
  plot->yAxis->setBasePen(Qt::NoPen);
  plot->xAxis->setBasePen(Qt::NoPen);
  plot->xAxis->setRange(0, 1000);
  initCursors();
}

Plotting::~Plotting() {}

void Plotting::newDataString(QByteArray data) {
  QByteArrayList pointList = data.split(';');
  for (long i = 0; i < pointList.length(); i++)
    addPoint(pointList.at(i));
  if (plottingStatus == PLOT_STATUS_SINGLETRIGER) {
    plottingStatus = PLOT_STATUS_PAUSE;
    emit showPlotStatus(plottingStatus);
  }
}

void Plotting::addPoint(QByteArray point) {
  if (point.length() > 0) {
    QByteArrayList values = point.split(',');
    for (uint8_t i = 0; i < values.length() - 1 && i < CHANNEL_COUNT; i++)
      channels.at(i)->addValue(values.at(i + 1), values.at(0));
  }
}

double Plotting::minTime() {
  double min = INFINITY;
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (!channels.at(i)->isEmpty())
      if (channels.at(i)->firstTime() < min)
        min = channels.at(i)->firstTime();
  }
  return min;
}

double Plotting::maxTime() {
  double max = -INFINITY;
  for (int i = 0; i < CHANNEL_COUNT; i++)
    if (!channels.at(i)->isEmpty())
      if (channels.at(i)->upperTimeRange() > max)
        max = channels.at(i)->upperTimeRange();
  return max;
}

void Plotting::initCursors() {
  cursorX1 = new QCPItemLine(plot);
  cursorX2 = new QCPItemLine(plot);
  cursorY1 = new QCPItemLine(plot);
  cursorY2 = new QCPItemLine(plot);
  QPen cursorpen;
  cursorpen.setColor(Qt::black);
  cursorX1->setPen(cursorpen);
  cursorX2->setPen(cursorpen);
  cursorY1->setPen(cursorpen);
  cursorY2->setPen(cursorpen);
  cursorX1->end->setTypeX(QCPItemPosition::ptPlotCoords);
  cursorX2->end->setTypeX(QCPItemPosition::ptPlotCoords);
  cursorY1->end->setTypeX(QCPItemPosition::ptViewportRatio);
  cursorY2->end->setTypeX(QCPItemPosition::ptViewportRatio);
  cursorX1->start->setTypeX(QCPItemPosition::ptPlotCoords);
  cursorX2->start->setTypeX(QCPItemPosition::ptPlotCoords);
  cursorY1->start->setTypeX(QCPItemPosition::ptViewportRatio);
  cursorY2->start->setTypeX(QCPItemPosition::ptViewportRatio);
  cursorX1->end->setTypeY(QCPItemPosition::ptViewportRatio);
  cursorX2->end->setTypeY(QCPItemPosition::ptViewportRatio);
  cursorY1->end->setTypeY(QCPItemPosition::ptPlotCoords);
  cursorY2->end->setTypeY(QCPItemPosition::ptPlotCoords);
  cursorX1->start->setTypeY(QCPItemPosition::ptViewportRatio);
  cursorX2->start->setTypeY(QCPItemPosition::ptViewportRatio);
  cursorY1->start->setTypeY(QCPItemPosition::ptPlotCoords);
  cursorY2->start->setTypeY(QCPItemPosition::ptPlotCoords);
  cursorX1->setVisible(false);
  cursorX2->setVisible(false);
  cursorY1->setVisible(false);
  cursorY2->setVisible(false);
  updateCursors(curX1, curX2, curY1, curY2);
}

void Plotting::clearChannels() {
  for (int ch = 0; ch < CHANNEL_COUNT; ch++)
    channels.at(ch)->clear();
}

void Plotting::autoset() {}

void Plotting::updateCursors(double x1, double x2, double y1, double y2) {
  curX1 = x1;
  curX2 = x2;
  curY1 = y1;
  curY2 = y2;
  cursorX1->start->setCoords(curX1, 0);
  cursorX1->end->setCoords(curX1, 1);
  cursorX2->start->setCoords(curX2, 0);
  cursorX2->end->setCoords(curX2, 1);
  cursorY1->start->setCoords(0, curY1);
  cursorY1->end->setCoords(1, curY1);
  cursorY2->start->setCoords(0, curY2);
  cursorY2->end->setCoords(1, curY2);
}

void Plotting::update() {
  horizontalPos->setMaximum(1000 - zoom / 2);
  horizontalPos->setMinimum(zoom / 2);
  horizontalPos->setPageStep(zoom);
  double minT = 0;
  double maxT = 10;
  if (plottingRangeType != PLOT_RANGE_FREE) {
    minT = minTime();
    maxT = maxTime();
    plot->yAxis->setRange((verticalCenter * 0.01 - 1) * 0.5 * verticalRange, (verticalCenter * 0.01 + 1) * 0.5 * verticalRange);
  }
  if (plottingRangeType == PLOT_RANGE_FIXED) {
    double dataLenght = maxT - minT;
    plot->xAxis->setRange(minT + dataLenght * 0.001 * (horizontalPos->value() - zoom / 2), minT + dataLenght * 0.001 * (horizontalPos->value() + zoom / 2));
  } else if (plottingRangeType == PLOT_RANGE_ROLLING) {
    plot->xAxis->setRange(maxT - rollingRange, maxT);
  }

  if (plottingStatus == PLOT_STATUS_RUN || plottingStatus == PLOT_STATUS_SINGLETRIGER) {
    for (int ch = 0; ch < CHANNEL_COUNT; ch++) {
      channels.at(ch)->draw();
    }
  }
  emit setCursorBounds(plot->xAxis->range().lower, plot->xAxis->range().upper, plot->yAxis->range().lower, plot->yAxis->range().upper, minT, maxT, verticalCenter - verticalRange / 2, verticalCenter + verticalRange / 2);
  emit setVDivLimits(plot->yAxis->range().upper - plot->yAxis->range().lower);
  emit setHDivLimits(plot->xAxis->range().upper - plot->xAxis->range().lower);
  plot->replot();
}

void Plotting::setRangeType(int type) {
  this->plottingRangeType = type;
  if (type == PLOT_RANGE_FREE)
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  else {
    plot->setInteraction(QCP::iRangeDrag, false);
    plot->setInteraction(QCP::iRangeZoom, false);
  }
}

void Plotting::pauseClicked() {
  if (plottingStatus == PLOT_STATUS_RUN)
    plottingStatus = PLOT_STATUS_PAUSE;
  else if (plottingStatus == PLOT_STATUS_SINGLETRIGER)
    plottingStatus = PLOT_STATUS_RUN;
  else if (plottingStatus == PLOT_STATUS_PAUSE)
    plottingStatus = PLOT_STATUS_RUN;
  emit showPlotStatus(plottingStatus);
}

void Plotting::singleTrigerClicked() {
  plottingStatus = PLOT_STATUS_SINGLETRIGER;
  emit showPlotStatus(plottingStatus);
}

void Plotting::setVerticalDiv(double value) {
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  plot->yAxis->setTicker(fixedTicker);
  fixedTicker->setTickStep(value);
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
}

void Plotting::setHorizontalDiv(double value) {
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  plot->xAxis->setTicker(fixedTicker);
  fixedTicker->setTickStep(value);
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
}

void Plotting::setShowVerticalValues(bool enabled) { plot->yAxis->setTicks(enabled); }

void Plotting::setShowHorizontalValues(bool enabled) { plot->xAxis->setTicks(enabled); }

void Plotting::setCurXen(bool en) {
  cursorX1->setVisible(en);
  cursorX2->setVisible(en);
}

void Plotting::setCurYen(bool en) {
  cursorY1->setVisible(en);
  cursorY2->setVisible(en);
}

void Plotting::newDataBin(QByteArray data, int bits, double valMin, double valMax, double timeStep, int numCh, int firstCh, bool continuous) {
  int bytes = ceil(bits / 8.0f);
  if (data.length() % bytes != 0)
    data = data.left(data.length() - data.length() % bytes);
  if (!continuous)
    for (int ch = firstCh; ch < firstCh + numCh; ch++)
      this->channel(ch)->clear();
  for (int i = 0; i < data.length() - 1; i += bytes) {
    quint64 value = 0;
    for (int byte = 0; byte < bytes; byte++) {
      value = value << 8;
      value |= (quint8)data.at(i + byte);
    }
    double value_d = value;
    value_d = (value_d / (1 << bits) * (valMax - valMin)) + valMin;
    channels.at(firstCh + ((i / bytes) % numCh) - 1)->addValue(value_d, channels.at(firstCh + (i % numCh) - 1)->lastTime() + timeStep);
    if (plottingStatus == PLOT_STATUS_SINGLETRIGER) {
      plottingStatus = PLOT_STATUS_PAUSE;
      emit showPlotStatus(plottingStatus);
    }
  }
}
