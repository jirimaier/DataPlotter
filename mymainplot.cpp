//  Copyright (C) 2020-2021  Jiří Maier

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "mymainplot.h"

MyMainPlot::MyMainPlot(QWidget *parent) : MyPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  yAxis->setRange(0, 10, Qt::AlignCenter);
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
    for (int j = 0; j < LOGIC_BITS; j++) {
      addGraph(xAxis, logicGroupAxis.last());
      graph(graphCount() - 1)->setFillBase(j * 3);
    }
    logicGroupAxis.last()->setTicks(false);
    logicGroupAxis.last()->setBasePen(Qt::NoPen);
    logicGroupAxis.last()->setOffset(0);
    logicGroupAxis.last()->setPadding(0);
    logicGroupAxis.last()->setLabelPadding(0);
    logicGroupAxis.last()->setTickLabelPadding(0);
    logicGroupAxis.last()->setTickLength(0, 0);
  }

  initZeroLines();

  // Propojení musí být až po skončení inicializace!
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
    if (!this->graph(i)->data()->isEmpty() && channelSettings.at(i).visible) {
      firsts.append(graph(i)->data()->begin()->key);
      lasts.append((graph(i)->data()->end() - 1)->key);
    }
  for (int i = 0; i < LOGIC_GROUPS; i++)
    if (!this->graph(GlobalFunctions::getLogicChannelID(i, 0))->data()->isEmpty() && logicSettings.at(i).visible) {
      int chID = GlobalFunctions::getLogicChannelID(i, 0);
      firsts.append(graph(chID)->data()->begin()->key);
      lasts.append((graph(chID)->data()->end() - 1)->key);
    }
  if (!firsts.isEmpty()) {
    minT = *std::min_element(firsts.begin(), firsts.end());
    maxT = *std::max_element(lasts.begin(), lasts.end());
  } else {
    minT = 0;
    maxT = 100;
  }
}

void MyMainPlot::reOffsetAndRescaleCH(int chID) {
  double center = (yAxis->range().center() - channelSettings.at(chID).offset) / channelSettings.at(chID).scale;
  if (channelSettings.at(chID).inverted) center *= (-1);
  double range = yAxis->range().size() / channelSettings.at(chID).scale;
  analogAxis.at(chID)->setRange(center, range, Qt::AlignCenter);
}

void MyMainPlot::reOffsetAndRescaleLogic(int group) {
  double center = (yAxis->range().center() - logicSettings.at(group).offset) / logicSettings.at(group).scale;
  if (logicSettings.at(group).inverted) center *= (-1);
  double range = yAxis->range().size() / logicSettings.at(group).scale;
  logicGroupAxis.at(group)->setRange(center, range, Qt::AlignCenter);
}

void MyMainPlot::verticalAxisRangeChanged() {
  for (int chID = 0; chID < ANALOG_COUNT + MATH_COUNT; chID++) {
    reOffsetAndRescaleCH(chID);
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

QPair<QVector<double>, QVector<double>> MyMainPlot::getDataVector(int chID, bool onlyInView) {
  QVector<double> keys, values;
  for (QCPGraphDataContainer::iterator it = graph(chID)->data()->begin(); it != graph(chID)->data()->end(); it++) {
    if (!onlyInView || (it->key >= this->xAxis->range().lower && it->key <= this->xAxis->range().upper)) {
      keys.append(it->key);
      values.append(it->value);
    }
  }
  return QPair<QVector<double>, QVector<double>>(keys, values);
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
    int chID = GlobalFunctions::getLogicChannelID(group, bit);
    if (style == GraphStyle::linePoint) {
      this->graph(chID)->setScatterStyle(POINT_STYLE);
      this->graph(chID)->setLineStyle(QCPGraph::lsLine);
      this->graph(chID)->setBrush(Qt::NoBrush);
    } else if (style == GraphStyle::line) {
      this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(chID)->setLineStyle(QCPGraph::lsLine);
      this->graph(chID)->setBrush(Qt::NoBrush);
    } else if (style == GraphStyle::point) {
      this->graph(chID)->setScatterStyle(POINT_STYLE);
      this->graph(chID)->setLineStyle(QCPGraph::lsNone);
      this->graph(chID)->setBrush(Qt::NoBrush);
    } else if (style == GraphStyle::filled) {
      this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(chID)->setLineStyle(QCPGraph::lsLine);
      this->graph(chID)->setBrush(logicSettings.at(group).color);
    } else if (style == GraphStyle::square) {
      this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(chID)->setLineStyle(QCPGraph::lsStepCenter);
      this->graph(chID)->setBrush(Qt::NoBrush);
    } else if (style == GraphStyle::squareFilled) {
      this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(chID)->setLineStyle(QCPGraph::lsStepCenter);
      this->graph(chID)->setBrush(logicSettings.at(group).color);
    }
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setLogicColor(int group, QColor color) {
  logicSettings[group].color = color;
  for (int bit = 0; bit < LOGIC_BITS; bit++) {
    this->graph(GlobalFunctions::getLogicChannelID(group, bit))->setPen(QPen(color));
    if (logicSettings.at(group).style == GraphStyle::filled || logicSettings.at(group).style == GraphStyle::squareFilled) this->graph(GlobalFunctions::getLogicChannelID(group, bit))->setBrush(color);
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setLogicVisibility(int group, bool visible) {
  logicSettings[group].visible = visible;
  for (int bit = 0; bit < LOGIC_BITS; bit++) this->graph(GlobalFunctions::getLogicChannelID(group, bit))->setVisible(visible);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

int MyMainPlot::getLogicBitsUsed(int group) {
  for (int i = 0; i < LOGIC_BITS; i++)
    if (!isChUsed(GlobalFunctions::getLogicChannelID(group, i))) return (i);
  return LOGIC_BITS;
}

QPair<unsigned int, unsigned int> MyMainPlot::getChVisibleSamplesRange(int chID) {
  unsigned int min = 0;
  unsigned int max = 0;
  for (QCPGraphDataContainer::iterator it = graph(chID)->data()->begin(); it != graph(chID)->data()->end(); it++) {
    if (it->key < xAxis->range().lower) min++;
    if (it->key > xAxis->range().upper) break;
    max++;
  }
  if (max > 0) max--;
  return (QPair<unsigned int, unsigned int>(min, max));
}

void MyMainPlot::setChStyle(int chID, int style) {
  channelSettings[chID].style = style;
  if (style == GraphStyle::linePoint) {
    this->graph(chID)->setScatterStyle(POINT_STYLE);
    this->graph(chID)->setLineStyle(QCPGraph::lsLine);
    this->graph(chID)->setBrush(Qt::NoBrush);
  } else if (style == GraphStyle::line) {
    this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
    this->graph(chID)->setLineStyle(QCPGraph::lsLine);
    this->graph(chID)->setBrush(Qt::NoBrush);
  } else if (style == GraphStyle::point) {
    this->graph(chID)->setScatterStyle(POINT_STYLE);
    this->graph(chID)->setLineStyle(QCPGraph::lsNone);
    this->graph(chID)->setBrush(Qt::NoBrush);
  } else if (style == GraphStyle::filled) {
    this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
    this->graph(chID)->setLineStyle(QCPGraph::lsLine);
    this->graph(chID)->setBrush(channelSettings.at(chID).color);
  } else if (style == GraphStyle::square) {
    this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
    this->graph(chID)->setLineStyle(QCPGraph::lsStepCenter);
    this->graph(chID)->setBrush(Qt::NoBrush);
  } else if (style == GraphStyle::squareFilled) {
    this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
    this->graph(chID)->setLineStyle(QCPGraph::lsStepCenter);
    this->graph(chID)->setBrush(channelSettings.at(chID).color);
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChColor(int chID, QColor color) {
  channelSettings[chID].color = color;
  zeroLines.at(chID)->setPen(QPen(color, 1, Qt::DashLine));
  this->graph(chID)->setPen(QPen(color));
  if (channelSettings.at(chID).style == GraphStyle::filled || channelSettings.at(chID).style == GraphStyle::squareFilled) this->graph(chID)->setBrush(color);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChOffset(int chID, double offset) {
  channelSettings[chID].offset = offset;
  reOffsetAndRescaleCH(chID);
  zeroLines.at(chID)->start->setCoords(0, offset);
  zeroLines.at(chID)->end->setCoords(1, offset);
  zeroLines.at(chID)->setVisible(offset != 0);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  emit requestCursorUpdate();
}

void MyMainPlot::setChScale(int chID, double scale) {
  channelSettings[chID].scale = scale;
  reOffsetAndRescaleCH(chID);
  emit requestCursorUpdate();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChInvert(int chID, bool inverted) {
  channelSettings[chID].inverted = inverted;
  analogAxis.at(chID)->setRangeReversed(inverted);
  reOffsetAndRescaleCH(chID);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChVisible(int chID, bool visible) {
  channelSettings[chID].visible = visible;
  zeroLines.at(chID)->setVisible(visible && channelSettings.at(chID).offset != 0);
  this->graph(chID)->setVisible(visible);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::resume() {
  plottingStatus = PlotStatus::run;
  emit showPlotStatus(plottingStatus);
  for (int i = 0; i < ALL_COUNT; i++) {
    if (!pauseBuffer.at(i).data()->isEmpty()) graph(i)->setData(pauseBuffer.at(i));
  }
  pauseBuffer.clear();
  newData = true;
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
      this->xAxis->setRange(minT + dataLenght * 0.001 * (horizontalPos - zoom / 2), minT + dataLenght * 0.001 * (horizontalPos + zoom / 2));
    } else if (plotRangeType == PlotRange::rolling) {
      double maxTnew = maxT;
      if (shiftStep > 0) maxTnew = ceil(maxT / (rollingRange * shiftStep / 100.0)) * (rollingRange * shiftStep / 100.0);
      this->xAxis->setRange(maxTnew - rollingRange, maxTnew);
    }
  }
  emit requestCursorUpdate();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setRangeType(PlotRange::enumerator type) {
  if (plotRangeType == PlotRange::freeMove && type != PlotRange::freeMove) {
    this->yAxis->setRange(presetVRange * presetVCenterRatio / 200.0, presetVRange, Qt::AlignCenter);
    this->yAxis->setRange(presetVRange * presetVCenterRatio / 200.0, yAxis->range().size(), Qt::AlignCenter);
    this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  }
  this->plotRangeType = type;
  setMouseControlls(type == PlotRange::freeMove);
  if (plotRangeType == PlotRange::rolling && shiftStep == 0)
    plotUpdateTimer.setInterval(16);
  else
    plotUpdateTimer.setInterval(30);
  redraw();
}

void MyMainPlot::pause() {
  for (int i = 0; i < ALL_COUNT; i++) pauseBuffer.append(QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*graph(i)->data())));
  plottingStatus = PlotStatus::pause;
  emit showPlotStatus(plottingStatus);
}

void MyMainPlot::clearLogicGroup(int number, int fromBit) {
  if (isChUsed(GlobalFunctions::getLogicChannelID(number, fromBit))) {
    for (int i = fromBit; i < LOGIC_BITS; i++) clearCh(GlobalFunctions::getLogicChannelID(number, i));
    newData = true;
  }
}

void MyMainPlot::clearCh(int chID) {
  this->graph(chID)->data().data()->clear();                               // Odstraní kanál
  if (plottingStatus == PlotStatus::pause) pauseBuffer.at(chID)->clear();  // Vymaže i z paměti pro pauzu (jinak by se po ukončení pauzy načetl zpět)
  newData = true;                                                          // Aby se překreslil graf
}

void MyMainPlot::resetChannels() {
  for (int i = 0; i < ALL_COUNT; i++) {
    clearCh(i);
    if (plottingStatus == PlotStatus::pause) pauseBuffer.at(i)->clear();
  }
  updateMinMaxTimes();
  redraw();
}

void MyMainPlot::newDataVector(int chID, QSharedPointer<QCPGraphDataContainer> data, bool ignorePause) {
  if (data->size() == 1) {
    newDataPoint(chID, data->at(0)->key, data->at(0)->value, data->at(0)->key > graph(chID)->data()->at(graph(chID)->data()->size() - 1)->key);
    return;
  }
  if (plottingStatus != PlotStatus::pause || ignorePause) {
    this->graph(chID)->setData(data);
    newData = true;
  }
}

void MyMainPlot::setRollingRange(double value) {
  rollingRange = value;
  redraw();
}

void MyMainPlot::setHorizontalPos(double value) {
  horizontalPos = value;
  redraw();
}

void MyMainPlot::setVerticalRange(double value) {
  presetVRange = value;
  if (plotRangeType != PlotRange::freeMove) {
    this->yAxis->setRange(presetVRange * presetVCenterRatio / 200.0, value, Qt::AlignCenter);
    this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  }
}

void MyMainPlot::setZoomRange(int value) {
  zoom = value;
  redraw();
}

void MyMainPlot::setVerticalCenter(int value) {
  presetVCenterRatio = value;
  if (plotRangeType != PlotRange::freeMove) {
    this->yAxis->setRange(presetVRange * presetVCenterRatio / 200.0, yAxis->range().size(), Qt::AlignCenter);
    this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  }
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

void MyMainPlot::newDataPoint(int chID, double time, double value, bool append) {
  if (plottingStatus != PlotStatus::pause) {
    if (!append) this->graph(chID)->data()->clear();
    this->graph(chID)->addData(time, value);
    newData = true;
  } else {
    if (!append) pauseBuffer.at(chID)->clear();
    pauseBuffer.at(chID)->add(QCPGraphData(time, value));
  }
}

QByteArray MyMainPlot::exportChannelCSV(char separator, char decimal, int chID, int precision, bool onlyInView) {
  QByteArray output = (QString("time%1%2\n").arg(separator).arg(GlobalFunctions::getChName(chID))).toUtf8();
  for (QCPGraphDataContainer::iterator it = graph(chID)->data()->begin(); it != graph(chID)->data()->end(); it++) {
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
        int chID = GlobalFunctions::getLogicChannelID(group, bit);
        output.append(QString((((int)graph(chID)->data()->at(i)->value) % 3) ? "0" : "1").replace('.', decimal).toUtf8());
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
    bool isVisible;
    if (i < ANALOG_COUNT + MATH_COUNT)
      isVisible = channelSettings.at(i).visible;
    else
      isVisible = logicSettings.at(i - ANALOG_COUNT - MATH_COUNT).visible;
    if (!graph(i)->data()->isEmpty() && (isVisible || includeHidden)) {
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
  for (QVector<QPair<QVector<double>, QVector<double>>>::iterator it = channels.begin(); it != channels.end(); it++) foreach (double time, it->first)
      if (!times.contains(time)) times.append(time);
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
