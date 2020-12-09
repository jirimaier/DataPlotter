#include "mymainplot.h"

MyMainPlot::MyMainPlot(QWidget *parent) : MyPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  yAxis->setRange(0, 10, Qt::AlignCenter);
  pauseBufferTime.resize(ANALOG_COUNT + MATH_COUNT);
  pauseBufferValue.resize(ANALOG_COUNT + MATH_COUNT);
  channelSettings.resize(ANALOG_COUNT + MATH_COUNT);
  logicSettings.resize(LOGIC_GROUPS);

  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
    zeroLines.append(new QCPItemLine(this));
    analogAxis.append(this->axisRect()->addAxis(QCPAxis::atRight, 0));
    analogAxis.last()->setRange(yAxis->range());
    addGraph(xAxis, analogAxis.last());
    analogAxis.last()->setTicks(false);
    analogAxis.last()->setBasePen(Qt::NoPen);
    analogAxis.last()->setOffset(0);
    analogAxis.last()->setPadding(0);
    analogAxis.last()->setLabelPadding(0);
    analogAxis.last()->setTickLabelPadding(0);
    analogAxis.last()->setTickLength(0, 0);
  }
  for (int i = 0; i < LOGIC_GROUPS; i++) {
    logicGroupAxis.append(this->axisRect()->addAxis(QCPAxis::atRight, 0));
    logicGroupAxis.last()->setRange(yAxis->range());
    for (int j = 0; j < LOGIC_BITS; j++)
      addGraph(xAxis, logicGroupAxis.last());
    logicGroupAxis.last()->setTicks(false);
    logicGroupAxis.last()->setBasePen(Qt::NoPen);
    logicGroupAxis.last()->setOffset(0);
    logicGroupAxis.last()->setPadding(0);
    logicGroupAxis.last()->setLabelPadding(0);
    logicGroupAxis.last()->setTickLabelPadding(0);
    logicGroupAxis.last()->setTickLength(0, 0);
  }
  initZeroLines();

  // Propojení musí být aý po skončení inicializace!
  connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(verticalAxisRangeChanged(void)));
  connect(&plotUpdateTimer, &QTimer::timeout, this, &MyMainPlot::update);
  plotUpdateTimer.start(50);
  replot();
}

MyMainPlot::~MyMainPlot() {}

void MyMainPlot::initZeroLines() {
  QPen zeroPen;
  zeroPen.setWidth(1);
  zeroPen.setStyle(Qt::DotLine);
  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
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

void MyMainPlot::updateMinMaxTimes() {
  QVector<double> firsts, lasts;
  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty() && channelSettings.at(i).style != GraphStyle::hidden) {
      firsts.append(graph(i)->data()->begin()->key);
      lasts.append((graph(i)->data()->end() - 1)->key);
    }
  for (int i = 0; i < LOGIC_GROUPS; i++)
    if (!this->graph(GlobalFunctions::getLogicChannelID(i, 0))->data()->isEmpty() && logicSettings.at(i).style != GraphStyle::hidden) {
      int chid = GlobalFunctions::getLogicChannelID(i, 0);
      firsts.append(graph(chid)->data()->begin()->key);
      lasts.append((graph(chid)->data()->end() - 1)->key);
    }
  if (!firsts.isEmpty()) {
    minT = *std::min_element(firsts.begin(), firsts.end());
    maxT = *std::max_element(lasts.begin(), lasts.end());
  } else {
    minT = 0;
    maxT = 100;
  }
}

void MyMainPlot::reOffsetAndRescaleCH(int chid) {
  double center = (yAxis->range().center() - channelSettings.at(chid).offset) / channelSettings.at(chid).scale;
  if (channelSettings.at(chid).inverted)
    center *= (-1);
  double range = yAxis->range().size() / channelSettings.at(chid).scale;
  analogAxis.at(chid)->setRange(center, range, Qt::AlignCenter);
}

void MyMainPlot::reOffsetAndRescaleLogic(int group) {
  double center = (yAxis->range().center() - logicSettings.at(group).offset) / logicSettings.at(group).scale;
  if (logicSettings.at(group).inverted)
    center *= (-1);
  double range = yAxis->range().size() / logicSettings.at(group).scale;
  logicGroupAxis.at(group)->setRange(center, range, Qt::AlignCenter);
}

void MyMainPlot::verticalAxisRangeChanged() {
  for (int chid = 0; chid < ANALOG_COUNT + MATH_COUNT; chid++) {
    reOffsetAndRescaleCH(chid);
  }
  for (int group = 0; group < LOGIC_GROUPS; group++) {
    reOffsetAndRescaleLogic(group);
  }
}

void MyMainPlot::togglePause() {
  if (plottingStatus == PlotStatus::run)
    pause();
  else
    resume();
}

QPair<QVector<double>, QVector<double>> MyMainPlot::getDataVector(int chid, bool onlyInView) {
  QVector<double> keys, values;
  for (QCPGraphDataContainer::iterator it = graph(chid)->data()->begin(); it != graph(chid)->data()->end(); it++) {
    if (!onlyInView || (it->key >= this->xAxis->range().lower && it->key <= this->xAxis->range().upper)) {
      keys.append(it->key);
      values.append(it->value);
    }
  }
  return QPair<QVector<double>, QVector<double>>(keys, values);
}

double MyMainPlot::getChMin(int chid) {
  QVector<double> values = getDataVector(chid, false).second;
  return *std::min_element(values.begin(), values.end());
}

void MyMainPlot::setLogicOffset(int group, double offset) {
  logicSettings[group].offset = offset;
  reOffsetAndRescaleLogic(group);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setLogicScale(int group, double scale) {
  logicSettings[group].scale = scale;
  reOffsetAndRescaleLogic(group);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setLogicStyle(int group, int style) {
  logicSettings[group].style = style;
  for (int bit = 0; bit < LOGIC_BITS; bit++) {
    int chid = GlobalFunctions::getLogicChannelID(group, bit);
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
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setLogicColor(int group, QColor color) {
  logicSettings[group].color = color;
  for (int bit = 0; bit < LOGIC_BITS; bit++)
    this->graph(GlobalFunctions::getLogicChannelID(group, bit))->setPen(QPen(color));
}

void MyMainPlot::setLogicInvert(int group, bool inverted) {
  logicSettings[group].inverted = inverted;
  logicGroupAxis.at(group)->setRangeReversed(inverted);
  reOffsetAndRescaleLogic(group);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

int MyMainPlot::getLogicBitsUsed(int group) {
  for (int i = 0; i < LOGIC_BITS; i++)
    if (!isChUsed(GlobalFunctions::getLogicChannelID(group, i)))
      return (i);
  return LOGIC_BITS;
}

QPair<long, long> MyMainPlot::getChVisibleSamplesRange(int chid) {
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
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChColor(int chid, QColor color) {
  channelSettings[chid].color = color;
  zeroLines.at(chid)->setPen(QPen(color, 1, Qt::DashLine));
  this->graph(chid)->setPen(QPen(color));
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChOffset(int chid, double offset) {
  channelSettings[chid].offset = offset;
  reOffsetAndRescaleCH(chid);
  zeroLines.at(chid)->start->setCoords(0, offset);
  zeroLines.at(chid)->end->setCoords(1, offset);
  zeroLines.at(chid)->setVisible(offset != 0);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  emit requestCursorUpdate();
}

void MyMainPlot::setChScale(int chid, double scale) {
  channelSettings[chid].scale = scale;
  reOffsetAndRescaleCH(chid);
  emit requestCursorUpdate();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChInvert(int ch, bool inverted) {
  channelSettings[ch].inverted = inverted;
  analogAxis.at(ch)->setRangeReversed(inverted);
  reOffsetAndRescaleCH(ch);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::resume() {
  plottingStatus = PlotStatus::run;
  emit showPlotStatus(plottingStatus);
  for (int i = 0; i < ALL_COUNT; i++) {
    if (!pauseBufferTime.at(i).isEmpty()) {
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
    updateMinMaxTimes();
    redraw();
  }
}

void MyMainPlot::redraw() {
  if (plotRangeType != PlotRange::freeMove) {
    if (plotRangeType == PlotRange::fixedRange) {
      double dataLenght = maxT - minT;
      this->xAxis->setRange(minT + dataLenght * 0.001 * (plotSettings.horizontalPos - plotSettings.zoom / 2), minT + dataLenght * 0.001 * (plotSettings.horizontalPos + plotSettings.zoom / 2));
    } else if (plotRangeType == PlotRange::rolling) {
      double maxTnew = maxT;
      if (shiftStep > 0)
        maxTnew = ceil(maxT / (plotSettings.rollingRange * shiftStep / 100.0)) * (plotSettings.rollingRange * shiftStep / 100.0);
      this->xAxis->setRange(maxTnew - plotSettings.rollingRange, maxTnew);
    }
  }
  emit requestCursorUpdate();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setRangeType(PlotRange::enumerator type) {
  if (plotRangeType == PlotRange::freeMove && type != PlotRange::freeMove) {
    setVerticalCenter(presetVRange * presetVCenterRatio / 200.0);
    setVerticalRange(presetVRange);
  }
  this->plotRangeType = type;
  setMouseControlls(type == PlotRange::freeMove);
  if (plotRangeType == PlotRange::rolling && shiftStep == 0)
    plotUpdateTimer.setInterval(16);
  else
    plotUpdateTimer.setInterval(50);
  redraw();
}

void MyMainPlot::pause() {
  plottingStatus = PlotStatus::pause;
  emit showPlotStatus(plottingStatus);
}

void MyMainPlot::clearLogicGroup(int number) {
  for (int i = 0; i < LOGIC_BITS; i++)
    clearCh(GlobalFunctions::getLogicChannelID(number, i));
  updateMinMaxTimes();
  redraw();
}

void MyMainPlot::resetChannels() {
  for (int i = 0; i < ALL_COUNT; i++) {
    clearCh(i);
  }
  updateMinMaxTimes();
  redraw();
}

void MyMainPlot::newDataVector(int ch, QSharedPointer<QVector<double>> time, QVector<double> *value, bool isMath) {
  int chid = GlobalFunctions::getAnalogChId(ch, ChannelType::analog);
  if (plottingStatus != PlotStatus::pause || isMath) {
    // for (QVector<double>::iterator it = value->begin(); it != value->end(); it++)
    //*it = *it * channelSettings.at(chid).scale + channelSettings.at(chid).offset;
    this->graph(chid)->setData(*time, *value, true);
    newData = true;
  }
  delete value;
}

void MyMainPlot::newLogicDataVector(int group, QSharedPointer<QVector<double>> time, QVector<uint32_t> *value, int bits) {
  group--;
  if (plottingStatus != PlotStatus::pause) {
    if (isChUsed(GlobalFunctions::getLogicChannelID(group, bits)))
      clearLogicGroup(group);

    QVector<QVector<double>> subChannels;
    subChannels.resize(bits);

    for (QVector<uint32_t>::iterator it = value->begin(); it != value->end(); it++)
      for (uint8_t bit = 0; bit < bits; bit++)
        subChannels[bit].push_back(((bool)((*it) & ((uint32_t)1 << (bit)))) + bit * 3);

    for (uint8_t bit = 0; bit < bits; bit++)
      this->graph(GlobalFunctions::getLogicChannelID(group, bit))->setData(*time, subChannels.at(bit), true);

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
  presetVRange = value;
  this->yAxis->setRange(presetVRange * presetVCenterRatio / 200.0, value, Qt::AlignCenter);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setZoomRange(int value) {
  plotSettings.zoom = value;
  redraw();
}

void MyMainPlot::setVerticalCenter(int value) {
  presetVCenterRatio = value;
  this->yAxis->setRange(presetVRange * presetVCenterRatio / 200.0, yAxis->range().size(), Qt::AlignCenter);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setShiftStep(int step) {
  shiftStep = step;
  if (plotRangeType == PlotRange::rolling) {
    if (step == 0)
      plotUpdateTimer.setInterval(16);
    else
      plotUpdateTimer.setInterval(50);
  }
}

void MyMainPlot::newDataPoint(int ch, double time, double value, bool append) {
  int chid = GlobalFunctions::getAnalogChId(ch, ChannelType::analog);
  if (plottingStatus != PlotStatus::pause) {
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

QByteArray MyMainPlot::exportChannelCSV(char separator, char decimal, int chid, int precision, bool onlyInView) {
  QByteArray output = (QString("time%1%2\n").arg(separator).arg(GlobalFunctions::getChName(chid))).toUtf8();
  for (QCPGraphDataContainer::iterator it = graph(chid)->data()->begin(); it != graph(chid)->data()->end(); it++) {
    if (!onlyInView || (it->key >= this->xAxis->range().lower && it->key <= this->xAxis->range().upper)) {
      output.append(QString::number(it->key, 'f', precision).replace('.', decimal).toUtf8());
      output.append(separator);
      output.append(QString::number(it->value, 'f', precision).replace('.', decimal).toUtf8());
      output.append('\n');
    }
  }
  return output;
}

QByteArray MyMainPlot::exportLogicCSV(char separator, char decimal, int group, int precision, bool onlyInView) {
  QByteArray output = tr("time").toUtf8();

  int bits = getLogicBitsUsed(group);
  for (int i = bits - 1; i >= 0; i++) {
    output.append(separator);
    output.append(tr("bit %1").arg(i).toUtf8());
  }
  output.append('\n');
  for (int i = 0; i < graph(GlobalFunctions::getLogicChannelID(group, 0))->dataCount(); i++) {
    double time = graph(GlobalFunctions::getLogicChannelID(group, 0))->data()->at(i)->key;
    if (!onlyInView || (time >= this->xAxis->range().lower && time <= this->xAxis->range().upper)) {
      output.append(QString::number(time, 'f', precision).toUtf8());
      for (int bit = bits - 1; bit >= 0; bit++) {
        output.append(separator);
        int chid = GlobalFunctions::getLogicChannelID(group, bit);
        output.append(QString((((int)graph(chid)->data()->at(i)->value) % 3) ? "0" : "1").replace('.', decimal).toUtf8());
      }
      output.append('\n');
    }
  }
  return output;
}

QByteArray MyMainPlot::exportAllCSV(char separator, char decimal, int precision, bool onlyInView, bool includeHidden) {
  QByteArray output = "";
  QVector<QPair<QVector<double>, QVector<double>>> channels;
  bool firstNonEmpty = true;
  for (int i = 0; i < ALL_COUNT; i++) {
    if (!graph(i)->data()->isEmpty() && (getChStyle(i + 1) != GraphStyle::hidden || includeHidden)) {
      if (firstNonEmpty) {
        firstNonEmpty = false;
        output.append(tr("time").toUtf8());
      }
      channels.append(getDataVector(i, onlyInView));
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
