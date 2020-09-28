#include "myplot.h"

MyPlot::MyPlot(QWidget *parent) : QCustomPlot(parent) {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    pauseBufferTime.append(new QVector<double>);
    pauseBufferValue.append(new QVector<double>);
    channelSettings.append(new ChannelSettings_t);
    scales.append(1);
    offsets.append(0);
  }
  for (int i = 0; i < 8; i++)
    channelSettings.at(i)->color = defaultColors[i];
  initCursors();
  initZeroLines();
  resetChannels();
}

void MyPlot::initCursors() {
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

void MyPlot::initZeroLines() {
  QPen zeroPen;
  zeroPen.setWidth(1);
  zeroPen.setStyle(Qt::DotLine);
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    zeroPen.setColor(channelSettings.at(i)->color);
    QCPItemLine *zeroLine = new QCPItemLine(this);
    zeroLine->setPen(zeroPen);
    zeroLine->start->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine->start->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine->end->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine->end->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine->start->setCoords(0, offsets.at(i));
    zeroLine->end->setCoords(1, offsets.at(i));
    zeroLine->setVisible(offsets.at(i) != 0);
    zeroLines.append(zeroLine);
  }
}

double MyPlot::minTime() {
  QVector<double> times;
  for (int i = 0; i < CHANNEL_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty())
      times.append(this->graph(i)->data()->begin()->key);
  if (times.isEmpty())
    return 0;
  else
    return *std::min_element(times.begin(), times.end());
}

double MyPlot::maxTime() {
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

double MyPlot::graphLastTime(quint8 i) {
  QCPGraphDataContainer::iterator it = this->graph(i)->data()->end();
  return (--it)->key;
}

void MyPlot::updateCursors(double x1, double x2, double y1, double y2) {
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

void MyPlot::setChStyle(int ch, int style) {
  channelSettings.at(ch - 1)->style = style;
  updateVisuals();
}

void MyPlot::setChColor(int ch, QColor color) {
  channelSettings.at(ch - 1)->color = color;
  updateVisuals();
}

void MyPlot::changeChOffset(int ch, double offset) {
  reoffset(ch - 1, offset - offsets.at(ch - 1));
  offsets.replace(ch - 1, offset);
}

void MyPlot::changeChScale(int ch, double scale) {
  rescale(ch - 1, scale / scales.at(ch - 1));
  scales.replace(ch - 1, scale);
}

void MyPlot::resume() {
  plottingStatus = PLOT_STATUS_RUN;
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (!pauseBufferTime.at(i)->isEmpty()) {
      for (QVector<double>::iterator it = pauseBufferValue.at(i)->begin(); it != pauseBufferValue.at(i)->end(); it++)
        *it = *it * scales.at(i) + offsets.at(i);
      if (lastWasContinous)
        graph(i)->addData(*pauseBufferTime.at(i), *pauseBufferValue.at(i), true);
      else
        graph(i)->setData(*pauseBufferTime.at(i), *pauseBufferValue.at(i), true);
      pauseBufferTime.at(i)->clear();
      pauseBufferValue.at(i)->clear();
    }
  }
}

void MyPlot::update() {
  minT = minTime();
  maxT = maxTime();
  if (plottingRangeType != PLOT_RANGE_FREE) {
    this->yAxis->setRange((plotSettings.verticalCenter * 0.01 - 1) * 0.5 * plotSettings.verticalRange, (plotSettings.verticalCenter * 0.01 + 1) * 0.5 * plotSettings.verticalRange);
    if (plottingRangeType == PLOT_RANGE_FIXED) {
      double dataLenght = maxT - minT;
      this->xAxis->setRange(minT + dataLenght * 0.001 * (plotSettings.horizontalPos - plotSettings.zoom / 2), minT + dataLenght * 0.001 * (plotSettings.horizontalPos + plotSettings.zoom / 2));
    } else if (plottingRangeType == PLOT_RANGE_ROLLING) {
      this->xAxis->setRange(maxT - plotSettings.rollingRange, maxT);
    }
  }
  emit setCursorBounds(this->xAxis->range().lower, this->xAxis->range().upper, this->yAxis->range().lower, this->yAxis->range().upper, minT, maxT, plotSettings.verticalCenter - plotSettings.verticalRange / 2, plotSettings.verticalCenter + plotSettings.verticalRange / 2);
  emit updateDivs(this->yAxis->range().upper - this->yAxis->range().lower, this->xAxis->range().upper - this->xAxis->range().lower);
  this->replot();
}

void MyPlot::setRangeType(int type) {
  this->plottingRangeType = type;
  if (type == PLOT_RANGE_FREE) {
    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);
  } else {
    this->setInteraction(QCP::iRangeDrag, false);
    this->setInteraction(QCP::iRangeZoom, false);
  }
}

void MyPlot::pauseClicked() {
  if (plottingStatus == PLOT_STATUS_RUN)
    plottingStatus = PLOT_STATUS_PAUSE;
  else if (plottingStatus == PLOT_STATUS_SINGLETRIGER)
    resume();
  else if (plottingStatus == PLOT_STATUS_PAUSE)
    resume();
  emit showPlotStatus(plottingStatus);
}

void MyPlot::singleTrigerClicked() {
  plottingStatus = PLOT_STATUS_SINGLETRIGER;
  emit showPlotStatus(plottingStatus);
}

void MyPlot::setVerticalDiv(double value) {
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  this->yAxis->setTicker(fixedTicker);
  fixedTicker->setTickStep(value);
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  vdiv = value;
}

void MyPlot::setHorizontalDiv(double value) {
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  this->xAxis->setTicker(fixedTicker);
  fixedTicker->setTickStep(value);
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  hdiv = value;
}

void MyPlot::setShowVerticalValues(bool enabled) {
  this->yAxis->setTicks(enabled);
  this->yAxis->setBasePen(enabled ? Qt::SolidLine : Qt::NoPen);
}

void MyPlot::setShowHorizontalValues(bool enabled) {
  this->xAxis->setTicks(enabled);
  this->xAxis->setBasePen(enabled ? Qt::SolidLine : Qt::NoPen);
}

void MyPlot::setCurXen(bool en) {
  cursorX1->setVisible(en);
  cursorX2->setVisible(en);
}

void MyPlot::setCurYen(bool en) {
  cursorY1->setVisible(en);
  cursorY2->setVisible(en);
}

void MyPlot::updateVisuals() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    this->graph(i)->setPen(QPen(channelSettings.at(i)->color));
    this->graph(i)->setVisible((channelSettings.at(i)->style != PLOT_STYLE_HIDDEN));
    zeroLines.at(i)->setVisible((offsets.at(i) != 0) && (channelSettings.at(i)->style != PLOT_STYLE_HIDDEN));
    if (channelSettings.at(i)->style == PLOT_STYLE_LINEANDPIONT) {
      this->graph(i)->setScatterStyle(QCPScatterStyle::ssDisc);
      this->graph(i)->setLineStyle(QCPGraph::lsLine);
    }
    if (channelSettings.at(i)->style == PLOT_STYLE_LINE) {
      this->graph(i)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(i)->setLineStyle(QCPGraph::lsLine);
    }
    if (channelSettings.at(i)->style == PLOT_STYLE_POINT) {
      this->graph(i)->setScatterStyle(QCPScatterStyle::ssDisc);
      this->graph(i)->setLineStyle(QCPGraph::lsNone);
    }
  }
}

void MyPlot::resetChannels() {
  this->clearGraphs();
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    pauseBufferTime.at(i)->clear();
    pauseBufferValue.at(i)->clear();
    this->addGraph();
  }
  updateVisuals();
}

void MyPlot::rescale(int ch, double relativeScale) {
  for (QCPGraphDataContainer::iterator it = graph(ch)->data()->begin(); it != graph(ch)->data()->end(); it++) {
    (*it).value -= offsets.at(ch);
    (*it).value *= relativeScale;
    (*it).value += offsets.at(ch);
  }
}

void MyPlot::reoffset(int ch, double relativeOffset) {
  zeroLines.at(ch)->start->setCoords(0, offsets.at(ch) + relativeOffset);
  zeroLines.at(ch)->end->setCoords(1, offsets.at(ch) + relativeOffset);
  zeroLines.at(ch)->setVisible((offsets.at(ch) + relativeOffset != 0) && (channelSettings.at(ch)->style != PLOT_STYLE_HIDDEN));
  for (QCPGraphDataContainer::iterator it = graph(ch)->data()->begin(); it != graph(ch)->data()->end(); it++)
    (*it).value += relativeOffset;
}

void MyPlot::newData(int ch, QVector<double> *time, QVector<double> *value, bool continous) {
  if (plottingStatus != PLOT_STATUS_PAUSE) {
    for (QVector<double>::iterator it = value->begin(); it != value->end(); it++)
      *it = *it * scales.at(ch - 1) + offsets.at(ch - 1);
    if (continous)
      this->graph(ch - 1)->addData(*time, *value, true);
    else
      this->graph(ch - 1)->setData(*time, *value, true);

    if (plottingStatus == PLOT_STATUS_SINGLETRIGER) {
      plottingStatus = PLOT_STATUS_PAUSE;
      emit showPlotStatus(plottingStatus);
    }
  } else {
    if (!continous) {
      pauseBufferTime.at(ch - 1)->clear();
      pauseBufferValue.at(ch - 1)->clear();
    }
    pauseBufferTime.at(ch - 1)->append(*time);
    pauseBufferValue.at(ch - 1)->append(*value);
    if (plottingStatus == PLOT_STATUS_SINGLETRIGER) {
      plottingStatus = PLOT_STATUS_PAUSE;
      emit showPlotStatus(plottingStatus);
    }
  }
  lastWasContinous = continous;
  delete time;
  delete value;
}
