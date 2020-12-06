#include "mymainplot.h"

MyMainPlot::MyMainPlot(QWidget *parent) : MyPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  for (int i = 0; i < ALL_COUNT; i++) {
    pauseBufferTime.resize(pauseBufferTime.size() + 1);
    pauseBufferValue.resize(pauseBufferValue.size() + 1);
    channelSettings.resize(channelSettings.size() + 1);
    zeroLines.append(new QCPItemLine(this));
    addGraph();
  }
  initZeroLines();
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
  newData = true;
}

void MyMainPlot::changeLogicScale(int group, double scale) {
  for (int bit = 1; bit <= LOGIC_BITS; bit++)
    changeChScale(GlobalFunctions::getLogicChannelId(group, bit), scale);
  changeLogicOffset(group, getChOffset(GlobalFunctions::getLogicChannelId(group, 1)));
  newData = true;
}

void MyMainPlot::setLogicStyle(int group, int style) {
  for (int bit = 1; bit <= LOGIC_BITS; bit++)
    setChStyle(GlobalFunctions::getLogicChannelId(group, bit), style);
}

void MyMainPlot::setLogicColor(int group, QColor color) {
  for (int bit = 1; bit <= LOGIC_BITS; bit++)
    setChColor(GlobalFunctions::getLogicChannelId(group, bit), color);
}

int MyMainPlot::getLogicBitsUsed(int group) {
  for (int i = 1; i <= LOGIC_BITS; i++)
    if (!isChUsed(GlobalFunctions::getLogicChannelId(group, i)))
      return (i - 1);
  return LOGIC_BITS;
}

QPair<long, long> MyMainPlot::getChVisibleSamples(int chid) {
  long min = 0;
  long max = -1;
  for (QCPGraphDataContainer::iterator it = graph(chid)->data()->begin(); it != graph(chid)->data()->end(); it++) {
    if (it->key < xAxis->range().lower)
      min++;
    if (it->key > xAxis->range().upper)
      break;
    max++;
  }
  return (QPair<long, long>(min, MAX(0, max)));
}

QPair<double, double> MyMainPlot::setTimeCursor(int cursor, int chid, unsigned int sample) {
  double time = graph(chid)->data()->at(sample)->key;
  double value = graph(chid)->data()->at(sample)->value;
  updateCursor(cursor, time);
  updateCursor(cursor + 2, value);
  return (QPair<double, double>(time, (value - channelSettings.at(chid).offset) / channelSettings.at(chid).scale));
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

void MyMainPlot::setChStyle(int chid, int style) {
  channelSettings[chid].style = style;
  this->graph(chid)->setVisible((style != GraphStyle::hidden));
  if (style == GraphStyle::linePoint) {
    this->graph(chid)->setScatterStyle(POINT_STYLE);
    this->graph(chid)->setLineStyle(QCPGraph::lsLine);
  }
  if (style == GraphStyle::line) {
    this->graph(chid)->setScatterStyle(QCPScatterStyle::ssNone);
    this->graph(chid)->setLineStyle(QCPGraph::lsLine);
  }
  if (style == GraphStyle::point) {
    this->graph(chid)->setScatterStyle(POINT_STYLE);
    this->graph(chid)->setLineStyle(QCPGraph::lsNone);
  }
  repaintNeeded = true;
}

void MyMainPlot::setChColor(int chid, QColor color) {
  channelSettings[chid].color = color;
  zeroLines.at(chid)->setPen(QPen(color, 1, Qt::DashLine));
  this->graph(chid)->setPen(QPen(color));
  repaintNeeded = true;
}

void MyMainPlot::changeChOffset(int chid, double offset) {
  double diff = offset - channelSettings.at(chid).offset;
  if (diff != 0) {
    reoffset(chid, diff);
    channelSettings[chid].offset = offset;
    newData = true;
  }
}

void MyMainPlot::changeChScale(int chid, double scale) {
  double diff = scale / channelSettings.at(chid).scale;
  if (diff != 1) {
    rescale(chid, scale / channelSettings.at(chid).scale);
    channelSettings[chid].scale = scale;
    newData = true;
  }
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
  redraw();
}

void MyMainPlot::update() {
  if (newData) {
    newData = false;
    minT = minTime();
    maxT = maxTime();
    redraw();
  } else if (repaintNeeded) {
    repaintNeeded = false;
    this->replot();
  }
}

void MyMainPlot::redraw() {
  if (plottingRangeType != PlotRange::freeMove) {
    this->yAxis->setRange((plotSettings.verticalCenter * 0.01 - 1) * 0.5 * plotSettings.verticalRange, (plotSettings.verticalCenter * 0.01 + 1) * 0.5 * plotSettings.verticalRange);
    if (plottingRangeType == PlotRange::fixedRange) {
      double dataLenght = maxT - minT;
      this->xAxis->setRange(minT + dataLenght * 0.001 * (plotSettings.horizontalPos - plotSettings.zoom / 2), minT + dataLenght * 0.001 * (plotSettings.horizontalPos + plotSettings.zoom / 2));
    } else if (plottingRangeType == PlotRange::rolling) {
      double maxTnew = maxT;
      if (shiftStep > 0)
        maxTnew = ceil(maxT / (plotSettings.rollingRange * shiftStep / 10.0)) * (plotSettings.rollingRange * shiftStep / 10.0);
      this->xAxis->setRange(maxTnew - plotSettings.rollingRange, maxTnew);
    }
  }

  for (int i = 0; i < ALL_COUNT; i++) {
    zeroLines.at(i)->setVisible(((channelSettings.at(i).offset != 0) || (i > ANALOG_COUNT + MATH_COUNT)) && (channelSettings.at(i).style != GraphStyle::hidden) && isChUsed(i));
  }
  emit requestCursorUpdate();
  this->replot();
}

void MyMainPlot::setRangeType(int type) {
  this->plottingRangeType = type;
  setMouseControlls(type == PlotRange::freeMove);
  redraw();
}

void MyMainPlot::pause() {
  plottingStatus = PlotStatus::pause;
  emit showPlotStatus(plottingStatus);
}

void MyMainPlot::clearLogicGroup(int number) {
  for (int i = 1; i <= LOGIC_BITS; i++)
    clearCh(GlobalFunctions::getLogicChannelId(number, i));
  redraw();
}

void MyMainPlot::resetChannels() {
  for (int i = 0; i < ALL_COUNT; i++) {
    graph(i)->data().clear();
  }
  redraw();
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

void MyMainPlot::newDataVector(int ch, QSharedPointer<QVector<double>> time, QVector<double> *value, bool isMath) {
  int chid = GlobalFunctions::getAnalogChId(ch, ChannelType::analog);
  if (plottingStatus != PlotStatus::pause || isMath) {
    for (QVector<double>::iterator it = value->begin(); it != value->end(); it++)
      *it = *it * channelSettings.at(chid).scale + channelSettings.at(chid).offset;
    this->graph(chid)->setData(*time, *value, true);
    newData = true;
  }
  delete value;
}

void MyMainPlot::newLogicDataVector(int ch, QSharedPointer<QVector<double>> time, QVector<uint32_t> *value, int bits) {
  if (plottingStatus != PlotStatus::pause) {
    if (isChUsed(GlobalFunctions::getLogicChannelId(ch, bits + 1)))
      clearLogicGroup(ch);
    QVector<QVector<double>> channels;
    channels.resize(bits);
    for (QVector<uint32_t>::iterator it = value->begin(); it != value->end(); it++)
      for (uint8_t bit = 1; bit <= bits; bit++)
        channels[bit - 1].push_back((bool)((*it) & ((uint32_t)1 << (bit - 1))));

    for (uint8_t bit = 1; bit <= bits; bit++) {
      int chid = GlobalFunctions::getLogicChannelId(ch, bit);
      for (QVector<double>::iterator it = channels[bit - 1].begin(); it != channels[bit - 1].end(); it++)
        *it = *it * channelSettings.at(chid).scale + channelSettings.at(chid).offset;
      this->graph(chid)->setData(*time, channels.at(bit - 1), true);
    }
    newData = true;
  }
  delete value;
}

void MyMainPlot::setRollingRange(double value) {
  plotSettings.rollingRange = value;
  redraw();
}

void MyMainPlot::setHorizontalPos(double value) {
  plotSettings.horizontalPos = value;
  redraw();
}

void MyMainPlot::setVerticalRange(double value) {
  plotSettings.verticalRange = value;
  redraw();
}

void MyMainPlot::setZoomRange(int value) {
  plotSettings.zoom = value;
  redraw();
}

void MyMainPlot::setVerticalCenter(int value) {
  plotSettings.verticalCenter = value;
  redraw();
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
    newData = true;
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

QByteArray MyMainPlot::exportLogicCSV(char separator, char decimal, int group, int precision, bool offseted, bool onlyInView) {
  QByteArray output = tr("time").toUtf8();
  int bits = getLogicBitsUsed(group);
  for (int i = 1; i <= bits; i++) {
    output.append(separator);
    output.append(tr("bit %1").arg(i).toUtf8());
  }
  output.append('\n');
  for (int i = 0; i < graph(GlobalFunctions::getLogicChannelId(group, 1))->dataCount(); i++) {
    double time = graph(GlobalFunctions::getLogicChannelId(group, 1))->data()->at(i)->key;
    if (!onlyInView || (time >= this->xAxis->range().lower && time <= this->xAxis->range().upper)) {
      output.append(QString::number(time, 'f', precision).toUtf8());
      for (int bit = 1; bit <= bits; bit++) {
        output.append(separator);
        int chid = GlobalFunctions::getLogicChannelId(group, bit);
        output.append(offseted ? QString::number(graph(chid)->data()->at(i)->value, 'f', precision).replace('.', decimal).toUtf8() : QString::number(qRound((graph(chid)->data()->at(i)->value - channelSettings.at(chid).offset) / channelSettings.at(chid).scale)).toUtf8());
      }
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
