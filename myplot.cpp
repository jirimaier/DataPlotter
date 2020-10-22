#include "myplot.h"

MyPlot::MyPlot(QWidget *parent) : QCustomPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  for (int i = 0; i < CHANNEL_COUNT + MATH_COUNT; i++) {
    pauseBufferTime.append(new QVector<double>);
    pauseBufferValue.append(new QVector<double>);
    channelSettings.append(new ChannelSettings_t);
    scales.append(1);
    offsets.append(0);
    zeroLines.append(new QCPItemLine(this));
  }
  initCursors();
  initZeroLines();
  resetChannels();
}

void MyPlot::initCursors() {
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

void MyPlot::initZeroLines() {
  QPen zeroPen;
  zeroPen.setWidth(1);
  zeroPen.setStyle(Qt::DotLine);
  for (int i = 0; i < CHANNEL_COUNT + MATH_COUNT; i++) {
    zeroPen.setColor(channelSettings.at(i)->color);
    QCPItemLine &zeroLine = *zeroLines.at(i);
    zeroLine.setPen(zeroPen);
    zeroLine.start->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine.start->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine.end->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine.end->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine.start->setCoords(0, offsets.at(i));
    zeroLine.end->setCoords(1, offsets.at(i));
    zeroLine.setVisible(offsets.at(i) != 0);
  }
}

double MyPlot::minTime() {
  QVector<double> times;
  for (int i = 0; i < CHANNEL_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty() && channelSettings.at(i)->style != GraphStyle::hidden)
      times.append(this->graph(i)->data()->begin()->key);
  if (times.isEmpty())
    return 0;
  else
    return *std::min_element(times.begin(), times.end());
}

double MyPlot::maxTime() {
  QVector<double> times;
  for (int i = 0; i < CHANNEL_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty() && channelSettings.at(i)->style != GraphStyle::hidden) {
      times.append(graphLastTime(i));
    }
  if (times.isEmpty())
    return 100;
  else
    return *std::max_element(times.begin(), times.end());
}

double MyPlot::graphLastTime(quint8 i) {
  QCPGraphDataContainer::iterator it = this->graph(i)->data()->end();
  return (--it)->key;
}

QPair<QVector<double>, QVector<double>> MyPlot::getDataVector(int ch, bool includeOffsets, bool onlyInView) {
  QVector<double> keys, values;
  for (QCPGraphDataContainer::iterator it = graph(ch)->data()->begin(); it != graph(ch)->data()->end(); it++) {
    if (!onlyInView || (it->key >= this->xAxis->range().lower && it->key <= this->xAxis->range().upper)) {
      if (includeOffsets) {
        keys.append(it->key);
        values.append(it->value);
      } else {
        keys.append(it->key);
        values.append((it->value - offsets.at(ch)) / scales.at(ch));
      }
    }
  }
  return QPair<QVector<double>, QVector<double>>(keys, values);
}

double MyPlot::getChMinValue(int ch) {
  QVector<double> values = getDataVector(ch - 1, false).second;
  return *std::min_element(values.begin(), values.end());
}

double MyPlot::getChMaxValue(int ch) {
  QVector<double> values = getDataVector(ch - 1, false).second;
  return *std::max_element(values.begin(), values.end());
}

void MyPlot::setCursorsAccess(bool allowed) {
  for (int i = 0; i < 4; i++)
    cursors.at(i)->setVisible(allowed);
  iHaveCursors = allowed;
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

void MyPlot::setChStyle(int ch, int style) {
  channelSettings.at(ch - 1)->style = style;
  updateVisuals();
}

void MyPlot::setChColor(int ch, QColor color) {
  channelSettings.at(ch - 1)->color = color;
  updateVisuals();
}

void MyPlot::setChName(int ch, QString name) {
  channelSettings.at(ch - 1)->name = name;
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
  if (plottingRangeType != PlotRange::freeMove) {
    this->yAxis->setRange((plotSettings.verticalCenter * 0.01 - 1) * 0.5 * plotSettings.verticalRange, (plotSettings.verticalCenter * 0.01 + 1) * 0.5 * plotSettings.verticalRange);
    if (plottingRangeType == PlotRange::fixedRange) {
      double dataLenght = maxT - minT;
      this->xAxis->setRange(minT + dataLenght * 0.001 * (plotSettings.horizontalPos - plotSettings.zoom / 2), minT + dataLenght * 0.001 * (plotSettings.horizontalPos + plotSettings.zoom / 2));
    } else if (plottingRangeType == PlotRange::rolling) {
      this->xAxis->setRange(maxT - plotSettings.rollingRange, maxT);
    }
  }
  PlotFrame_t frame;

  frame.xMinView = this->xAxis->range().lower;
  frame.xMaxView = this->xAxis->range().upper;
  frame.yMinView = this->yAxis->range().lower;
  frame.yMaxView = this->yAxis->range().upper;
  if (iHaveCursors) {
    frame.xMinTotal = minT;
    frame.xMaxTotal = maxT;
    frame.yMinTotal = plotSettings.verticalCenter - plotSettings.verticalRange / 2;
    frame.yMaxTotal = plotSettings.verticalCenter + plotSettings.verticalRange / 2;
    emit setCursorBounds(frame);
  }
  emit updateDivs(frame.yMaxView - frame.yMinView, frame.xMaxView - frame.yMinView);
  this->replot();
}

void MyPlot::setRangeType(int type) {
  this->plottingRangeType = type;
  if (type == PlotRange::freeMove) {
    this->setInteraction(QCP::iRangeDrag, true);
    this->setInteraction(QCP::iRangeZoom, true);
  } else {
    this->setInteraction(QCP::iRangeDrag, false);
    this->setInteraction(QCP::iRangeZoom, false);
  }
}

void MyPlot::pauseClicked() {
  if (plottingStatus == PlotStatus::run)
    plottingStatus = PlotStatus::pause;
  else if (plottingStatus == PlotStatus::single) {
    plottingStatus = PlotStatus::run;
    resume();
  } else if (plottingStatus == PlotStatus::pause) {
    plottingStatus = PlotStatus::run;
    resume();
  }
  emit showPlotStatus(plottingStatus);
}

void MyPlot::singleTrigerClicked() {
  plottingStatus = PlotStatus::single;
  resume();
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

void MyPlot::updateVisuals() {
  for (int i = 0; i < CHANNEL_COUNT + MATH_COUNT; i++) {
    this->graph(i)->setPen(QPen(channelSettings.at(i)->color));
    this->graph(i)->setVisible((channelSettings.at(i)->style != GraphStyle::hidden));
    zeroLines.at(i)->setVisible((offsets.at(i) != 0) && (channelSettings.at(i)->style != GraphStyle::hidden) && isChUsed(i + 1));
    zeroLines.at(i)->setPen(QPen(channelSettings.at(i)->color, 1, Qt::DashLine));
    if (channelSettings.at(i)->style == GraphStyle::linePoint) {
      this->graph(i)->setScatterStyle(POINT_STYLE);
      this->graph(i)->setLineStyle(QCPGraph::lsLine);
    }
    if (channelSettings.at(i)->style == GraphStyle::line) {
      this->graph(i)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(i)->setLineStyle(QCPGraph::lsLine);
    }
    if (channelSettings.at(i)->style == GraphStyle::point) {
      this->graph(i)->setScatterStyle(POINT_STYLE);
      this->graph(i)->setLineStyle(QCPGraph::lsNone);
    }
  }
}

void MyPlot::resetChannels() {
  this->clearGraphs();
  for (int i = 0; i < CHANNEL_COUNT + MATH_COUNT; i++) {
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
  zeroLines.at(ch)->setVisible((offsets.at(ch) + relativeOffset != 0) && (channelSettings.at(ch)->style != GraphStyle::hidden));
  for (QCPGraphDataContainer::iterator it = graph(ch)->data()->begin(); it != graph(ch)->data()->end(); it++)
    (*it).value += relativeOffset;
}

void MyPlot::newData(int ch, QVector<double> *time, QVector<double> *value, bool continous, bool sorted, bool ignorePause) {
  if (plottingStatus != PlotStatus::pause || ignorePause) {
    for (QVector<double>::iterator it = value->begin(); it != value->end(); it++)
      *it = *it * scales.at(ch - 1) + offsets.at(ch - 1);
    if (continous)
      this->graph(ch - 1)->addData(*time, *value, sorted);
    else
      this->graph(ch - 1)->setData(*time, *value, sorted);

    if (plottingStatus == PlotStatus::single) {
      plottingStatus = PlotStatus::pause;
      emit showPlotStatus(plottingStatus);
    }
  } else {
    if (!continous) {
      pauseBufferTime.at(ch - 1)->clear();
      pauseBufferValue.at(ch - 1)->clear();
    }
    pauseBufferTime.at(ch - 1)->append(*time);
    pauseBufferValue.at(ch - 1)->append(*value);
    if (plottingStatus == PlotStatus::single) {
      plottingStatus = PlotStatus::pause;
      emit showPlotStatus(plottingStatus);
    }
  }
  lastWasContinous = continous;
  delete time;
  delete value;
}

QByteArray MyPlot::exportChannelCSV(char separator, char decimal, int channel, int precision, bool offseted, bool onlyInView) {
  QByteArray output = (QString("time%1ch%2\n").arg(separator).arg(channel + 1)).toUtf8();
  for (QCPGraphDataContainer::iterator it = graph(channel)->data()->begin(); it != graph(channel)->data()->end(); it++) {
    if (!onlyInView || (it->key >= this->xAxis->range().lower && it->key <= this->xAxis->range().upper)) {
      output.append(QString::number(it->key, 'f', precision).replace('.', decimal).toUtf8());
      output.append(separator);
      output.append(QString::number(offseted ? it->value : (it->value - offsets.at(channel)) / scales.at(channel), 'f', precision).replace('.', decimal).toUtf8());
      output.append('\n');
    }
  }
  return output;
}

QByteArray MyPlot::exportAllCSV(char separator, char decimal, int precision, bool offseted, bool onlyInView, bool includeHidden) {
  QByteArray output = "";
  QVector<QPair<QVector<double>, QVector<double>>> channels;
  bool firstNonEmpty = true;
  for (int i = 0; i < CHANNEL_COUNT + MATH_COUNT; i++) {
    if (!graph(i)->data()->isEmpty() && (getChStyle(i + 1) != GraphStyle::hidden || includeHidden)) {
      if (firstNonEmpty) {
        firstNonEmpty = false;
        output.append("time");
      }
      channels.append(getDataVector(i, offseted, onlyInView));
      output.append(separator);
      output.append(QString("ch%1").arg(i + 1).toUtf8());
    }
  }
  QList<double> times;
  for (QVector<QPair<QVector<double>, QVector<double>>>::iterator it = channels.begin(); it != channels.end(); it++)
    foreach (double time, it->first)
      if (!times.contains(time))
        times.append(time);
  std::sort(times.begin(), times.end());

  foreach (double time, times) {
    output.append('\n');
    output.append(QString::number(time, 'f', precision).replace('.', decimal).toUtf8());
    for (QVector<QPair<QVector<double>, QVector<double>>>::iterator it = channels.begin(); it != channels.end(); it++) {
      output.append(',');
      if (!it->first.isEmpty())
        if (it->first.first() == time) {
          output.append(QString::number(it->second.first(), 'f', precision).replace('.', decimal).toUtf8());
          it->first.pop_front();
          it->second.pop_front();
        }
    }
  }
  return output;
}
