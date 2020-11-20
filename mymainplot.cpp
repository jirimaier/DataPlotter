#include "mymainplot.h"

MyMainPlot::MyMainPlot(QWidget *parent) : MyPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  for (int i = 0; i < ALL_COUNT; i++) {
    pauseBufferTime.resize(pauseBufferTime.size() + 1);
    pauseBufferValue.resize(pauseBufferValue.size() + 1);
    channelSettings.resize(channelSettings.size() + 1);
    zeroLines.append(new QCPItemLine(this));
  }
  initZeroLines();
  resetChannels();
  setUpLogic();
}

MyMainPlot::~MyMainPlot() {}

void MyMainPlot::initZeroLines() {
  QPen zeroPen;
  zeroPen.setWidth(1);
  zeroPen.setStyle(Qt::DotLine);
  for (int i = 0; i < ALL_COUNT; i++) {
    zeroPen.setColor(channelSettings.at(i).color);
    QCPItemLine &zeroLine = *zeroLines.at(i);
    zeroLine.setPen(zeroPen);
    zeroLine.start->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine.start->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine.end->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine.end->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine.start->setCoords(0, channelSettings.at(i).offset);
    zeroLine.end->setCoords(1, channelSettings.at(i).offset);
    zeroLine.setVisible(channelSettings.at(i).offset != 0);
  }
}

double MyMainPlot::minTime() {
  QVector<double> times;
  for (int i = 0; i < ALL_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty() && channelSettings.at(i).style != GraphStyle::hidden)
      times.append(this->graph(i)->data()->begin()->key);
  if (times.isEmpty())
    return 0;
  else
    return *std::min_element(times.begin(), times.end());
}

double MyMainPlot::maxTime() {
  QVector<double> times;
  for (int i = 0; i < ALL_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty() && channelSettings.at(i).style != GraphStyle::hidden) {
      times.append((this->graph(i)->data()->end() - 1)->key);
    }
  if (times.isEmpty())
    return 100;
  else
    return *std::max_element(times.begin(), times.end());
}

void MyMainPlot::togglePause() {
  if (plottingStatus == PlotStatus::run)
    pause();
  else
    resume();
}

QPair<QVector<double>, QVector<double>> MyMainPlot::getDataVector(int chid, bool includeOffsets, bool onlyInView) {
  QVector<double> keys, values;
  for (QCPGraphDataContainer::iterator it = graph(chid)->data()->begin(); it != graph(chid)->data()->end(); it++) {
    if (!onlyInView || (it->key >= this->xAxis->range().lower && it->key <= this->xAxis->range().upper)) {
      keys.append(it->key);
      values.append(includeOffsets ? it->value : ((it->value - channelSettings.at(chid).offset) / channelSettings.at(chid).scale));
    }
  }
  return QPair<QVector<double>, QVector<double>>(keys, values);
}

double MyMainPlot::getChMin(int chid) {
  QVector<double> values = getDataVector(chid, false).second;
  return *std::min_element(values.begin(), values.end());
}

void MyMainPlot::changeLogicOffset(int group, double offset) {
  double step = 2 * getChScale(GlobalFunctions::getLogicChannelId(group, 1));
  for (int bit = 1; bit <= LOGIC_BITS; bit++) {
    changeChOffset(GlobalFunctions::getLogicChannelId(group, bit), offset);
    offset += step;
  }
}

void MyMainPlot::changeLogicScale(int group, double scale) {
  for (int bit = 1; bit <= LOGIC_BITS; bit++)
    changeChScale(GlobalFunctions::getLogicChannelId(group, bit), scale);
  changeLogicOffset(group, getChOffset(GlobalFunctions::getLogicChannelId(group, 1)));
}

void MyMainPlot::setLogicStyle(int group, int style) {
  for (int bit = 1; bit <= LOGIC_BITS; bit++)
    setChStyle(GlobalFunctions::getLogicChannelId(group, bit), style);
}

void MyMainPlot::setLogicColor(int group, QColor color) {
  for (int bit = 1; bit <= LOGIC_BITS; bit++)
    setChColor(GlobalFunctions::getLogicChannelId(group, bit), color);
}

void MyMainPlot::setUpLogic() {
  for (int i = 1; i <= LOGIC_GROUPS; i++) {
    changeLogicOffset(i, 0);
    changeLogicScale(i, 1);
  }
}

double MyMainPlot::getChMax(int chid) {
  QVector<double> values = getDataVector(chid, false).second;
  return *std::max_element(values.begin(), values.end());
}

void MyMainPlot::clearCh(int chid) {
  this->graph(chid)->data().data()->clear();
  updateVisuals();
}

void MyMainPlot::setChStyle(int chid, int style) {
  channelSettings[chid].style = style;
  updateVisuals();
}

void MyMainPlot::setChColor(int chid, QColor color) {
  channelSettings[chid].color = color;
  updateVisuals();
}

void MyMainPlot::changeChOffset(int chid, double offset) {
  reoffset(chid, offset - channelSettings.at(chid).offset);
  channelSettings[chid].offset = offset;
}

void MyMainPlot::changeChScale(int chid, double scale) {
  rescale(chid, scale / channelSettings.at(chid).scale);
  channelSettings[chid].scale = scale;
}

void MyMainPlot::resume() {
  plottingStatus = PlotStatus::run;
  emit showPlotStatus(plottingStatus);
  for (int i = 0; i < ALL_COUNT; i++) {
    if (!pauseBufferTime.at(i).isEmpty()) {
      for (QVector<double>::iterator it = pauseBufferValue[i].begin(); it != pauseBufferValue[i].end(); it++)
        *it = *it * channelSettings.at(i).scale + channelSettings.at(i).offset;
      graph(i)->addData(pauseBufferTime.at(i), pauseBufferValue.at(i), true);
      pauseBufferTime[i].clear();
      pauseBufferValue[i].clear();
    }
  }
}

void MyMainPlot::update() {
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
  if (this->xAxis->range().upper > MAX_PLOT_ZOOMOUT)
    if (this->xAxis->range().upper - this->xAxis->range().lower > MAX_PLOT_ZOOMOUT)
      this->xAxis->setRange(this->xAxis->range().lower, this->xAxis->range().upper - MAX_PLOT_ZOOMOUT);
  this->replot();
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
  emit updateDivs(frame.yMaxView - frame.yMinView, frame.xMaxView - frame.xMinView);
}

void MyMainPlot::setRangeType(int type) {
  this->plottingRangeType = type;
  setMouseControlls(type == PlotRange::freeMove);
}

void MyMainPlot::pause() {
  plottingStatus = PlotStatus::pause;
  emit showPlotStatus(plottingStatus);
}

void MyMainPlot::updateVisuals() {
  for (int i = 0; i < ALL_COUNT; i++) {
    this->graph(i)->setPen(QPen(channelSettings.at(i).color));
    this->graph(i)->setVisible((channelSettings.at(i).style != GraphStyle::hidden));
    zeroLines.at(i)->setVisible(((channelSettings.at(i).offset != 0) || (i > ANALOG_COUNT + MATH_COUNT)) && (channelSettings.at(i).style != GraphStyle::hidden) /*&& isChUsed(i)*/);
    zeroLines.at(i)->setPen(QPen(channelSettings.at(i).color, 1, Qt::DashLine));
    if (channelSettings.at(i).style == GraphStyle::linePoint) {
      this->graph(i)->setScatterStyle(POINT_STYLE);
      this->graph(i)->setLineStyle(QCPGraph::lsLine);
    }
    if (channelSettings.at(i).style == GraphStyle::line) {
      this->graph(i)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(i)->setLineStyle(QCPGraph::lsLine);
    }
    if (channelSettings.at(i).style == GraphStyle::point) {
      this->graph(i)->setScatterStyle(POINT_STYLE);
      this->graph(i)->setLineStyle(QCPGraph::lsNone);
    }
  }
}

void MyMainPlot::resetChannels() {
  this->clearGraphs();
  for (int i = 0; i < ALL_COUNT; i++) {
    pauseBufferTime[i].clear();
    pauseBufferValue[i].clear();
    this->addGraph();
  }
  updateVisuals();
}

void MyMainPlot::rescale(int chid, double relativeScale) {
  for (QCPGraphDataContainer::iterator it = graph(chid)->data()->begin(); it != graph(chid)->data()->end(); it++) {
    (*it).value -= channelSettings.at(chid).offset;
    (*it).value *= relativeScale;
    (*it).value += channelSettings.at(chid).offset;
  }
}

void MyMainPlot::reoffset(int chid, double relativeOffset) {
  zeroLines.at(chid)->start->setCoords(0, channelSettings.at(chid).offset + relativeOffset);
  zeroLines.at(chid)->end->setCoords(1, channelSettings.at(chid).offset + relativeOffset);
  zeroLines.at(chid)->setVisible((channelSettings.at(chid).offset + relativeOffset != 0) && (channelSettings.at(chid).style != GraphStyle::hidden));
  for (QCPGraphDataContainer::iterator it = graph(chid)->data()->begin(); it != graph(chid)->data()->end(); it++)
    (*it).value += relativeOffset;
}

void MyMainPlot::newDataVector(int ch, QVector<double> *time, QVector<double> *value, bool isMath) {
  int chid = GlobalFunctions::getAnalogChId(ch, ChannelType::analog);
  if (plottingStatus != PlotStatus::pause || isMath) {
    for (QVector<double>::iterator it = value->begin(); it != value->end(); it++)
      *it = *it * channelSettings.at(chid).scale + channelSettings.at(chid).offset;
    this->graph(chid)->setData(*time, *value, true);
  }
  delete time;
  delete value;
}

void MyMainPlot::newDataPoint(int ch, double time, double value, bool append) {
  int chid = GlobalFunctions::getAnalogChId(ch, ChannelType::analog);
  if (plottingStatus != PlotStatus::pause) {
    value = value * channelSettings.at(chid).scale + channelSettings.at(chid).offset;
    if (append)
      this->graph(chid)->addData(time, value);
    else {
      QVector<double> singlepointTime, singlepointValue;
      singlepointTime.append(time);
      singlepointValue.append(value);
      this->graph(chid)->setData(singlepointTime, singlepointValue);
    }
  } else {
    if (!append) {
      pauseBufferTime[chid].clear();
      pauseBufferValue[chid].clear();
    }
    pauseBufferTime[chid].append(time);
    pauseBufferValue[chid].append(value);
  }
}

QByteArray MyMainPlot::exportChannelCSV(char separator, char decimal, int chid, int precision, bool offseted, bool onlyInView) {
  QByteArray output = (QString("time%1%2\n").arg(separator).arg(GlobalFunctions::getChName(chid))).toUtf8();
  for (QCPGraphDataContainer::iterator it = graph(chid)->data()->begin(); it != graph(chid)->data()->end(); it++) {
    if (!onlyInView || (it->key >= this->xAxis->range().lower && it->key <= this->xAxis->range().upper)) {
      output.append(QString::number(it->key, 'f', precision).replace('.', decimal).toUtf8());
      output.append(separator);
      output.append(QString::number(offseted ? it->value : (it->value - channelSettings.at(chid).offset) / channelSettings.at(chid).scale, 'f', precision).replace('.', decimal).toUtf8());
      output.append('\n');
    }
  }
  return output;
}

QByteArray MyMainPlot::exportAllCSV(char separator, char decimal, int precision, bool offseted, bool onlyInView, bool includeHidden) {
  QByteArray output = "";
  QVector<QPair<QVector<double>, QVector<double>>> channels;
  bool firstNonEmpty = true;
  for (int i = 0; i < ALL_COUNT; i++) {
    if (!graph(i)->data()->isEmpty() && (getChStyle(i + 1) != GraphStyle::hidden || includeHidden)) {
      if (firstNonEmpty) {
        firstNonEmpty = false;
        output.append(tr("time").toUtf8());
      }
      channels.append(getDataVector(i, offseted, onlyInView));
      output.append(separator);
      output.append(GlobalFunctions::getChName(i).toUtf8());
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
