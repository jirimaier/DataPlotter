#include "myplot.h"

myPlot::myPlot(QWidget *parent) : QCustomPlot(parent) {}

void myPlot::initCursors() {
  cursorX1 = new QCPItemLine(this);
  cursorX2 = new QCPItemLine(this);
  cursorY1 = new QCPItemLine(this);
  cursorY2 = new QCPItemLine(this);
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

void myPlot::initZeroLines() {
  QPen zeroPen;
  zeroPen.setWidth(1);
  zeroPen.setStyle(Qt::DotLine);
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    zeroPen.setColor(settings->channelSettings.at(i)->color);
    QCPItemLine *zeroLine = new QCPItemLine(this);
    zeroLine->setPen(zeroPen);
    zeroLine->start->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine->start->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine->end->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine->end->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine->start->setCoords(0, settings->channelSettings.at(i)->offset);
    zeroLine->end->setCoords(1, settings->channelSettings.at(i)->offset);
    zeroLine->setVisible(settings->channelSettings.at(i)->offset != 0);
    zeroLines.append(zeroLine);
  }
}

double myPlot::minTime() {
  QVector<double> times;
  for (int i = 0; i < CHANNEL_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty())
      times.append(this->graph(i)->data()->begin()->key);
  if (times.isEmpty())
    return 0;
  else
    return *std::min_element(times.begin(), times.end());
}

double myPlot::maxTime() {
  QVector<double> times;
  for (int i = 0; i < CHANNEL_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty()) {
      times.append(graphLastTime(i));
    }
  if (times.isEmpty())
    return 1;
  else
    return *std::max_element(times.begin(), times.end());
}

double myPlot::graphLastTime(quint8 i) {
  QCPGraphDataContainer::iterator it = this->graph(i)->data()->end();
  return (--it)->key;
}

void myPlot::updateCursors(double x1, double x2, double y1, double y2) {
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

void myPlot::init(Settings *in_settings) {
  this->settings = in_settings;
  resetChannels();
  initZeroLines();
  initCursors();
  updateVisuals();
  connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update()));
  updateTimer.start(100);
}

void myPlot::update() {
  emit requestNewData();
  if (plottingRangeType != PLOT_RANGE_FREE) {
    this->yAxis->setRange((settings->plotSettings.verticalCenter * 0.01 - 1) * 0.5 * settings->plotSettings.verticalRange, (settings->plotSettings.verticalCenter * 0.01 + 1) * 0.5 * settings->plotSettings.verticalRange);
    if (plottingRangeType == PLOT_RANGE_FIXED) {
      double dataLenght = maxT - minT;
      this->xAxis->setRange(minT + dataLenght * 0.001 * (settings->plotSettings.horizontalPos - settings->plotSettings.zoom / 2), minT + dataLenght * 0.001 * (settings->plotSettings.horizontalPos + settings->plotSettings.zoom / 2));
    } else if (plottingRangeType == PLOT_RANGE_ROLLING) {
      this->xAxis->setRange(maxT - settings->plotSettings.rollingRange, maxT);
    }
  }

  emit setCursorBounds(this->xAxis->range().lower, this->xAxis->range().upper, this->yAxis->range().lower, this->yAxis->range().upper, minT, maxT, settings->plotSettings.verticalCenter - settings->plotSettings.verticalRange / 2,
                       settings->plotSettings.verticalCenter + settings->plotSettings.verticalRange / 2);
  emit setVDivLimits(this->yAxis->range().upper - this->yAxis->range().lower);
  emit setHDivLimits(this->xAxis->range().upper - this->xAxis->range().lower);
  this->replot();
}

void myPlot::setRangeType(int type) {
  this->plottingRangeType = type;
  if (type == PLOT_RANGE_FREE) {
    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);
  } else {
    this->setInteraction(QCP::iRangeDrag, false);
    this->setInteraction(QCP::iRangeZoom, false);
  }
}

void myPlot::pauseClicked() {
  if (plottingStatus == PLOT_STATUS_RUN)
    plottingStatus = PLOT_STATUS_PAUSE;
  else if (plottingStatus == PLOT_STATUS_SINGLETRIGER)
    plottingStatus = PLOT_STATUS_RUN;
  else if (plottingStatus == PLOT_STATUS_PAUSE)
    plottingStatus = PLOT_STATUS_RUN;
  emit showPlotStatus(plottingStatus);
}

void myPlot::singleTrigerClicked() {
  plottingStatus = PLOT_STATUS_SINGLETRIGER;
  emit showPlotStatus(plottingStatus);
}

void myPlot::setVerticalDiv(double value) {
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  this->yAxis->setTicker(fixedTicker);
  fixedTicker->setTickStep(value);
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
}

void myPlot::setHorizontalDiv(double value) {
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  this->xAxis->setTicker(fixedTicker);
  fixedTicker->setTickStep(value);
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
}

void myPlot::setShowVerticalValues(bool enabled) { this->yAxis->setTicks(enabled); }

void myPlot::setShowHorizontalValues(bool enabled) { this->xAxis->setTicks(enabled); }

void myPlot::setCurXen(bool en) {
  cursorX1->setVisible(en);
  cursorX2->setVisible(en);
}

void myPlot::setCurYen(bool en) {
  cursorY1->setVisible(en);
  cursorY2->setVisible(en);
}

void myPlot::updateVisuals() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    this->graph(i)->setPen(QPen(settings->channelSettings.at(i)->color));
    this->graph(i)->setVisible((settings->channelSettings.at(i)->style != PLOT_STYLE_HIDDEN));

    if (settings->channelSettings.at(i)->style == PLOT_STYLE_LINEANDPIONT) {
      this->graph(i)->setScatterStyle(QCPScatterStyle::ssDisc);
      this->graph(i)->setLineStyle(QCPGraph::lsLine);
    }
    if (settings->channelSettings.at(i)->style == PLOT_STYLE_LINE) {
      this->graph(i)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(i)->setLineStyle(QCPGraph::lsLine);
    }
    if (settings->channelSettings.at(i)->style == PLOT_STYLE_POINT) {
      this->graph(i)->setScatterStyle(QCPScatterStyle::ssDisc);
      this->graph(i)->setLineStyle(QCPGraph::lsNone);
    }
  }
}

void myPlot::resetChannels() {
  this->clearGraphs();
  for (int i = 0; i < CHANNEL_COUNT; i++)
    this->addGraph();
}

void myPlot::rescale(int ch, double relativeScale) {
  for (QCPGraphDataContainer::iterator it = graph(ch)->data()->begin(); it != graph(ch)->data()->end(); it++) {
    (*it).value -= settings->channelSettings.at(ch)->offset;
    (*it).value *= relativeScale;
    (*it).value += settings->channelSettings.at(ch)->offset;
  }
}

void myPlot::reoffset(int ch, double relativeOffset) {
  zeroLines.at(ch)->start->setCoords(0, settings->channelSettings.at(ch)->offset + relativeOffset);
  zeroLines.at(ch)->end->setCoords(1, settings->channelSettings.at(ch)->offset + relativeOffset);
  zeroLines.at(ch)->setVisible(settings->channelSettings.at(ch)->offset + relativeOffset != 0);
  for (QCPGraphDataContainer::iterator it = graph(ch)->data()->begin(); it != graph(ch)->data()->end(); it++)
    (*it).value += relativeOffset;
}

void myPlot::newData(QVector<Channel *> *channels) {
  if (plottingStatus == PLOT_STATUS_RUN || plottingStatus == PLOT_STATUS_SINGLETRIGER) {
    for (int ch = 0; ch < CHANNEL_COUNT; ch++) {
      if (channels->at(ch)->isEmpty())
        continue;
      channels->at(ch)->applyScaleAndOffset(settings->channelSettings.at(ch)->scale, settings->channelSettings.at(ch)->offset);
      if ((!this->graph(ch)->data()->isEmpty()) && graphLastTime(ch) > channels->at(ch)->firstTime())
        this->graph(ch)->setData(channels->at(ch)->time, channels->at(ch)->value, true);
      else
        this->graph(ch)->addData(channels->at(ch)->time, channels->at(ch)->value, true);
      channels->at(ch)->clear();
    }
  }
  if (plottingStatus == PLOT_STATUS_SINGLETRIGER) {
    plottingStatus = PLOT_STATUS_PAUSE;
    emit showPlotStatus(plottingStatus);
  }
  minT = minTime();
  maxT = maxTime();
}
