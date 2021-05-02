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

MyMainPlot::MyMainPlot(QWidget* parent) : MyPlot(parent) {
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

  // Interpolační kanály
  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
    addGraph(xAxis, analogAxis.at(i));
  }

  initZeroLines();
  initTriggerLine();

  dataToBeInterpolated.resize(ANALOG_COUNT + MATH_COUNT);

  // Propojení musí být až po skončení inicializace!
  connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(verticalAxisRangeChanged()));
  connect(&plotUpdateTimer, &QTimer::timeout, this, &MyMainPlot::update);
  plotUpdateTimer.start(30);
  replot();
}

MyMainPlot::~MyMainPlot() {}

void MyMainPlot::initZeroLines() {
  QPen zeroPen;
  zeroPen.setWidth(1);
  zeroPen.setStyle(Qt::DotLine);
  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
    zeroPen.setColor(channelSettings.at(i).color);
    QCPItemLine& zeroLine = *zeroLines.at(i);
    zeroLine.setPen(zeroPen);
    zeroLine.start->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine.start->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine.end->setTypeY(QCPItemPosition::ptPlotCoords);
    zeroLine.end->setTypeX(QCPItemPosition::ptViewportRatio);
    zeroLine.start->setAxes(xAxis, graph(i)->valueAxis());
    zeroLine.end->setAxes(xAxis, graph(i)->valueAxis());
    zeroLine.start->setCoords(0, 0);
    zeroLine.end->setCoords(1, 0);
    zeroLine.setVisible(channelSettings.at(i).offset != 0);
  }
}

void MyMainPlot::initTriggerLine() {
  triggerLine = new QCPItemLine(this);
  triggerLineCh = graph(0);
  QPen triggerLinePen;
  triggerLinePen.setWidth(1);
  triggerLinePen.setStyle(Qt::DashLine);
  triggerLinePen.setColor(Qt::black);
  triggerLine->setPen(triggerLinePen);
  triggerLine->start->setTypeY(QCPItemPosition::ptPlotCoords);
  triggerLine->start->setTypeX(QCPItemPosition::ptViewportRatio);
  triggerLine->end->setTypeY(QCPItemPosition::ptPlotCoords);
  triggerLine->end->setTypeX(QCPItemPosition::ptViewportRatio);
  triggerLine->start->setAxes(xAxis, graph(0)->valueAxis());
  triggerLine->end->setAxes(xAxis, graph(0)->valueAxis());
  triggerLine->start->setCoords(0, 0);
  triggerLine->end->setCoords(1, 0);

  triggerLabel = new QCPItemText(this);
  triggerLabel->setLayer(cursorLayer);
  triggerLabel->setTextAlignment(Qt::AlignRight);
  triggerLabel->setPositionAlignment(Qt::AlignBottom | Qt::AlignRight);
  triggerLabel->position->setParentAnchorY(triggerLine->start);
  triggerLabel->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
  triggerLabel->position->setCoords(1, -2);
  triggerLabel->setVisible(false);
  triggerLabel->setBrush(transparentWhite);
  triggerLabel->setPadding(QMargins(2, 2, 2, 2));
  triggerLabel->setText(tr("Trigger"));
}

void MyMainPlot::updateMinMaxTimes() {
  QVector<double> firsts, lasts;
  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++)
    if (!this->graph(i)->data()->isEmpty() && channelSettings.at(i).visible) {
      firsts.append(graph(i)->data()->begin()->key);
      lasts.append((graph(i)->data()->end() - 1)->key);
    }
  for (int i = 0; i < LOGIC_GROUPS; i++)
    if (!this->graph(getLogicChannelID(i, 0))->data()->isEmpty() && logicSettings.at(i).visible) {
      int chID = getLogicChannelID(i, 0);
      firsts.append(graph(chID)->data()->begin()->key);
      lasts.append((graph(chID)->data()->end() - 1)->key);
    }
  if (!firsts.isEmpty()) {
    minT = *std::min_element(firsts.begin(), firsts.end());
    maxT = *std::max_element(lasts.begin(), lasts.end());
  } else {
    minT = 0;
    maxT = 10;
  }
}

void MyMainPlot::reOffsetAndRescaleCH(int chID) {
  double center = (yAxis->range().center() - channelSettings.at(chID).offset) / channelSettings.at(chID).scale;
  if (channelSettings.at(chID).inverted)
    center *= (-1);
  double range = yAxis->range().size() / channelSettings.at(chID).scale;
  analogAxis.at(chID)->setRange(center, range, Qt::AlignCenter);
}

void MyMainPlot::reOffsetAndRescaleLogic(int group) {
  double center = (yAxis->range().center() - logicSettings.at(group).offset) / logicSettings.at(group).scale;
  if (logicSettings.at(group).inverted)
    center *= (-1);
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
      if (IS_LOGIC_CH(chID))
        values.append((int)round(it->value) % 3);
      else
        values.append(it->value);
    }
  }
  return QPair<QVector<double>, QVector<double>>(keys, values);
}

void MyMainPlot::updateTracerText(int index) {
  QString tracerTextStr;
  tracerTextStr.append(" " + getChName(index) + "\n");
  if (IS_LOGIC_CH(index)) {
    tracerTextStr.append(QString("=%1, ").arg((uint32_t)(1 << ChID_TO_LOGIC_GROUP_BIT(index))));
    if ((int)tracer->position->value() % 3)
      tracerTextStr.append("HIGH");
    else
      tracerTextStr.append("LOW");
    tracerTextStr.append("\n");
  } else
    tracerTextStr.append(floatToNiceString(tracer->position->value(), 4, true, false) + getYUnit() + "\n");
  tracerTextStr.append(floatToNiceString(tracer->position->key(), 4, true, false) + getXUnit());
  tracerText->setText(tracerTextStr);
  checkIfTracerTextFits();
  tracerLayer->replot();
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
    int chID = getLogicChannelID(group, bit);
    if (style == GraphStyle::logicpoints) {
      this->graph(chID)->setScatterStyle(POINT_STYLE);
      this->graph(chID)->setLineStyle(QCPGraph::lsNone);
      this->graph(chID)->setBrush(Qt::NoBrush);
    } else if (style == GraphStyle::logicFilled) {
      this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(chID)->setLineStyle(QCPGraph::lsLine);
      this->graph(chID)->setBrush(logicSettings.at(group).color);
    } else if (style == GraphStyle::logicSquare) {
      this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(chID)->setLineStyle(QCPGraph::lsStepCenter);
      this->graph(chID)->setBrush(Qt::NoBrush);
    } else if (style == GraphStyle::logicSquareFilled) {
      this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(chID)->setLineStyle(QCPGraph::lsStepCenter);
      this->graph(chID)->setBrush(logicSettings.at(group).color);
    } else { // Logic line no-fill
      this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
      this->graph(chID)->setLineStyle(QCPGraph::lsLine);
      this->graph(chID)->setBrush(Qt::NoBrush);
    }
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setLogicColor(int group, QColor color) {
  logicSettings[group].color = color;
  for (int bit = 0; bit < LOGIC_BITS; bit++) {
    this->graph(getLogicChannelID(group, bit))->setPen(QPen(color));
    if (logicSettings.at(group).style == GraphStyle::logicFilled || logicSettings.at(group).style == GraphStyle::logicSquareFilled) {
      this->graph(getLogicChannelID(group, bit))->setBrush(color);
    }
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setLogicVisibility(int group, bool visible) {
  logicSettings[group].visible = visible;
  for (int bit = 0; bit < LOGIC_BITS; bit++)
    this->graph(getLogicChannelID(group, bit))->setVisible(visible);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setTriggerLineVisible(bool visible) {
  if (!visible) {
    triggerLine->setVisible(false);
    triggerLabel->setVisible(false);
  } else {
    triggerLine->setVisible(triggerLineCh->visible());
    triggerLabel->setVisible(triggerLineCh->visible());
  }
  triggerLineEnabled = triggerLine->visible();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setTriggerLineValue(double value) {
  triggerLine->start->setCoords(0, value);
  triggerLine->end->setCoords(1, value);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setTriggerLineChannel(int chid) {
  triggerLineCh = graph(chid);
  triggerLine->start->setAxes(xAxis, triggerLineCh->valueAxis());
  triggerLine->end->setAxes(xAxis, triggerLineCh->valueAxis());
  setTriggerLineVisible(triggerLineEnabled);
  //this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot); Zavoláno z setTriggerLineVisible
}

int MyMainPlot::getLogicBitsUsed(int group) {
  for (int i = 0; i < LOGIC_BITS; i++)
    if (!isChUsed(getLogicChannelID(group, i)))
      return (i);
  return LOGIC_BITS;
}

QPair<unsigned int, unsigned int> MyMainPlot::getChVisibleSamplesRange(int chID) {
  if (graph(chID)->data()->isEmpty())
    return (QPair<unsigned int, unsigned int>(0, 0));
  unsigned int min = graph(chID)->findBegin(xAxis->range().lower, false);
  unsigned int max = graph(chID)->findEnd(xAxis->range().upper, false) - 1; // end je za posledním, snížit o 1
  return (QPair<unsigned int, unsigned int>(min, max));
}

void MyMainPlot::setChStyle(int chID, int style) {
  channelSettings[chID].style = style;
  if (style == GraphStyle::linePoint) {
    this->graph(chID)->setScatterStyle(POINT_STYLE);
    this->graph(chID)->setLineStyle(QCPGraph::lsLine);
  } else if (style == GraphStyle::point) {
    this->graph(chID)->setScatterStyle(POINT_STYLE);
    this->graph(chID)->setLineStyle(QCPGraph::lsNone);
  } else { // Line
    this->graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
    if (channelSettings.at(chID).interpolate)
      this->graph(chID)->setLineStyle(QCPGraph::lsNone);
    else
      this->graph(chID)->setLineStyle(QCPGraph::lsLine);
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChColor(int chID, QColor color) {
  channelSettings[chID].color = color;
  zeroLines.at(chID)->setPen(QPen(color, 1, Qt::DashLine));
  this->graph(chID)->setPen(QPen(color));
  this->graph(INTERPOLATION_CHID(chID))->setPen(QPen(color));
  if (channelSettings.at(chID).style == GraphStyle::logicFilled || channelSettings.at(chID).style == GraphStyle::logicSquareFilled)
    this->graph(chID)->setBrush(color);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setChOffset(int chID, double offset) {
  channelSettings[chID].offset = offset;
  reOffsetAndRescaleCH(chID);
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

void MyMainPlot::setChInterpolate(int chID, bool enabled) {
  channelSettings[chID].interpolate = enabled;
  graph(INTERPOLATION_CHID(chID))->setVisible(enabled && channelSettings.at(chID).visible);
  setChStyle(chID, channelSettings.at(chID).style); //Updatuje styl čáry
}

void MyMainPlot::setChVisible(int chID, bool visible) {
  channelSettings[chID].visible = visible;
  graph(INTERPOLATION_CHID(chID))->setVisible(visible && channelSettings.at(chID).interpolate);
  zeroLines.at(chID)->setVisible(visible && channelSettings.at(chID).offset != 0);
  this->graph(chID)->setVisible(visible);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  if (triggerLineCh == graph(chID))
    setTriggerLineVisible(triggerLineEnabled);
}

void MyMainPlot::resume() {
  plottingStatus = PlotStatus::run;
  emit showPlotStatus(plottingStatus);
  for (int i = 0; i < ALL_COUNT; i++) {
    if (!pauseBuffer.at(i).data()->isEmpty())
      graph(i)->setData(pauseBuffer.at(i));
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
    double dataLenght = maxT - minT;
    if (plotRangeType == PlotRange::fixedRange) {
      this->xAxis->setRange(minT + dataLenght * 0.0005 * (2 * horizontalPos - zoom), minT + dataLenght * 0.0005 * (2 * horizontalPos + zoom));
    } else if (plotRangeType == PlotRange::rolling) {
      if (dataLenght < rollingRange) {
        this->xAxis->setRange(minT, minT + rollingRange);
      } else {
        double maxTnew = maxT;
        if (shiftStep > 0)
          maxTnew = ceil(maxT / (rollingRange * shiftStep / 100.0)) * (rollingRange * shiftStep / 100.0);
        this->xAxis->setRange(maxTnew - rollingRange, maxTnew);
      }
    }
  }
  emit requestCursorUpdate();

  // Přepsat text u traceru
  if (tracer->visible())
    updateTracerText(currentTracerIndex);

  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyMainPlot::setRangeType(PlotRange::enumPlotRange type) {
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
  for (int i = 0; i < ALL_COUNT; i++)
    pauseBuffer.append(QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*graph(i)->data())));
  plottingStatus = PlotStatus::pause;
  emit showPlotStatus(plottingStatus);
}

void MyMainPlot::clearLogicGroup(int number, int fromBit) {
  if (isChUsed(getLogicChannelID(number, fromBit))) {
    for (int i = fromBit; i < LOGIC_BITS; i++)
      clearCh(getLogicChannelID(number, i));
    newData = true;
  }
}

void MyMainPlot::clearCh(int chID) {
  this->graph(chID)->data().data()->clear(); // Odstraní kanál
  if (chID < ANALOG_COUNT + MATH_COUNT)
    this->graph(INTERPOLATION_CHID(chID))->data().data()->clear(); // Odstraní graf interpolace
  if (plottingStatus == PlotStatus::pause)
    pauseBuffer.at(chID)->clear(); // Vymaže i z paměti pro pauzu (jinak by se po ukončení pauzy načetl zpět)
  newData = true;                  // Aby se překreslil graf
}

void MyMainPlot::resetChannels() {
  for (int i = 0; i < ALL_COUNT; i++) {
    clearCh(i);
    if (plottingStatus == PlotStatus::pause)
      pauseBuffer.at(i)->clear();
  }
  setTriggerLineChannel(0);
  setTriggerLineValue(0);
  updateMinMaxTimes();
  redraw();
}

void MyMainPlot::newDataVector(int chID, QSharedPointer<QCPGraphDataContainer> data, bool ignorePause) {
  if (data->size() == 1) {
    newDataPoint(chID, data->at(0)->key, data->at(0)->value, data->at(0)->key > graph(chID)->data()->at(graph(chID)->data()->size() - 1)->key);
    return;
  }
  if (plottingStatus != PlotStatus::pause || ignorePause) {
    if (!IS_LOGIC_CH(chID)) {
      // Pokud má být interpolován, nedá data do grafu, ale připravý do bufferu odkud si je odebere interpolátor
      if (channelSettings.at(chID).interpolate) {
        dataToBeInterpolated[chID] = data;
        return;
      }
    }
    this->graph(chID)->setData(data);
    newData = true;
  }
}

void MyMainPlot::newInterpolatedVector(int chID, QSharedPointer<QCPGraphDataContainer> dataOriginal, QSharedPointer<QCPGraphDataContainer> dataInterpolated, bool dataIsFromInterpolationBuffer) {
  if (dataIsFromInterpolationBuffer)
    this->graph(chID)->setData(dataOriginal);
  this->graph(INTERPOLATION_CHID(chID))->setData(dataInterpolated);
  newData = true;
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

void MyMainPlot::setZoom(int value) {
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
      plotUpdateTimer.setInterval(30);
  }
}

void MyMainPlot::newDataPoint(int chID, double time, double value, bool append) {
  if (plottingStatus != PlotStatus::pause) {
    if (!append) {
      this->graph(chID)->data()->clear();
      this->graph(INTERPOLATION_CHID(chID))->data()->clear();
    }
    this->graph(chID)->addData(time, value);
    newData = true;
  } else {
    if (!append)
      pauseBuffer.at(chID)->clear();
    pauseBuffer.at(chID)->add(QCPGraphData(time, value));
  }
}

QByteArray MyMainPlot::exportChannelCSV(char separator, char decimal, int chID, int precision, bool onlyInView) {
  if (graph(chID)->data()->isEmpty())
    return "";
  QByteArray output = (QString("time%1%2\n").arg(separator).arg(getChName(chID))).toUtf8();
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
  int bits = getLogicBitsUsed(group);
  if (bits == 0)
    return "";

  QByteArray output = tr("time").toUtf8();

  for (int i = 0; i < bits; i++) {
    output.append(separator);
    output.append(QString("bit %1").arg(i).toUtf8());
  }
  output.append('\n');
  for (int i = 0; i < graph(getLogicChannelID(group, 0))->dataCount(); i++) {
    double time = graph(getLogicChannelID(group, 0))->data()->at(i)->key;
    if (!onlyInView || (time >= this->xAxis->range().lower && time <= this->xAxis->range().upper)) {
      output.append(QString::number(time, 'f', precision).toUtf8());
      for (int bit = 0; bit < bits; bit++) {
        output.append(separator);
        int chID = getLogicChannelID(group, bit);
        output.append(QString((((int)round(graph(chID)->data()->at(i)->value)) % 3) ? "1" : "0").replace('.', decimal).toUtf8());
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
    if (!graph(i)->data()->isEmpty() && (graph(i)->visible() || includeHidden)) {
      if (firstNonEmpty) {
        firstNonEmpty = false;
        output.append(tr("time").toUtf8());
      }
      channels.append(getDataVector(i, onlyInView));
      output.append(separator);
      output.append(getChName(i).toUtf8());
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
      output.append(separator);
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

void MyMainPlot::mouseMoved(QMouseEvent* event) {
  if (mouseDrag == MouseDrag::nothing) {
    //Nic není taženo, zobrazí tracer

    // Najde nejbližší kanál k myši, pokud žádný není blíž než 20 pixelů, vůbec se nezobrazí
    int nearestIndex = -1;
    unsigned int nearestDistance = TRACER_MOUSE_DISTANCE;
    for (int i = 0; i < ALL_COUNT; i++) {
      if (graph(i)->visible()) {
        unsigned int distance = (unsigned int)graph(i)->selectTest(event->pos(), false);
        if (distance < nearestDistance) {
          nearestIndex = i;
          nearestDistance = distance;
        }
      }
    }

    // Aby fungovalo i na interpolovaný graf
    if (nearestIndex == -1) {
      nearestDistance = TRACER_MOUSE_DISTANCE;
      for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
        if (graph(INTERPOLATION_CHID(i))->visible()) {
          unsigned int distance = (unsigned int)graph(INTERPOLATION_CHID(i))->selectTest(event->pos(), false);
          if (distance < nearestDistance) {
            nearestIndex = i;
            nearestDistance = distance;
          }
        }
      }
    }

    if (nearestIndex != -1) { // Myš je na grafu
      tracer->setVisible(true);
      tracerText->setVisible(true);
      tracer->setGraph(graph(nearestIndex));
      tracer->setYAxis(graph(nearestIndex)->valueAxis());
      tracer->setPoint(event->pos());
      updateTracerText(nearestIndex);
      currentTracerIndex = nearestIndex;
      this->QWidget::setCursor(defaultMouseCursor); // Cursor myši, ne ten grafový
    } else {
      if (tracer->visible())
        hideTracer();
      setMouseCursorStyle(event);
    }
  } else {
    if (tracer->visible())
      hideTracer();

    if (mouseDrag == MouseDrag::zeroline) { // Je tažen offset
      setChOffset(mouseDragChIndex, yAxis->pixelToCoord(event->pos().y()));
      emit offsetChangedByMouse(mouseDragChIndex);
    } else if (mouseDrag == MouseDrag::cursorY1)
      emit moveValueCursor(Cursors::Cursor1, cur1YAxis->pixelToCoord(event->pos().y()));
    else if (mouseDrag == MouseDrag::cursorY2)
      emit moveValueCursor(Cursors::Cursor2, cur2YAxis->pixelToCoord(event->pos().y()));
    else if (mouseDrag == MouseDrag::cursorX1)
      emit moveTimeCursor(Cursors::Cursor1, cur1Graph == -1 ? 0 : keyToNearestSample(graph(cur1Graph), xAxis->pixelToCoord(event->pos().x())), xAxis->pixelToCoord(event->pos().x()));
    else if (mouseDrag == MouseDrag::cursorX2)
      emit moveTimeCursor(Cursors::Cursor2, cur2Graph == -1 ? 0 : keyToNearestSample(graph(cur2Graph), xAxis->pixelToCoord(event->pos().x())), xAxis->pixelToCoord(event->pos().x()));
  }
}

void MyMainPlot::mousePressed(QMouseEvent* event) {
  // Kanál
  int nearestIndex = -1;
  unsigned int nearestDistance = TRACER_MOUSE_DISTANCE;
  for (int i = 0; i < ALL_COUNT; i++) {
    if (graph(i)->visible()) {
      unsigned int distance = (unsigned int)graph(i)->selectTest(event->pos(), false);
      if (distance < nearestDistance) {
        nearestIndex = i;
        nearestDistance = distance;
      }
    }
  }

  // Aby fungovalo kliknutí na interpolovaný graf
  if (nearestIndex == -1) {
    nearestDistance = PLOT_ELEMENTS_MOUSE_DISTANCE;
    for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
      if (graph(INTERPOLATION_CHID(i))->visible()) {
        unsigned int distance = (unsigned int)graph(INTERPOLATION_CHID(i))->selectTest(event->pos(), false);
        if (distance < nearestDistance) {
          nearestIndex = i;
          nearestDistance = distance;
        }
      }
    }
  }

  if (nearestIndex != -1) {
    tracer->setGraph(graph(nearestIndex));
    tracer->setYAxis(graph(nearestIndex)->valueAxis());
    tracer->setPoint(event->pos());
    tracer->updatePosition();
    if (IS_LOGIC_CH(nearestIndex))
      nearestIndex = LOGIC_GROUP_TO_CH_LIST_INDEX(ChID_TO_LOGIC_GROUP(nearestIndex));
    if (event->button() == Qt::RightButton) {
      mouseDrag = MouseDrag::cursorX2;
      this->setInteraction(QCP::iRangeDrag, false);
      emit setCursorPos(nearestIndex, Cursors::Cursor2, tracer->sampleNumber());
    } else {
      mouseDrag = MouseDrag::cursorX1;
      this->setInteraction(QCP::iRangeDrag, false);
      emit setCursorPos(nearestIndex, Cursors::Cursor1, tracer->sampleNumber());
    }
    return;
  }

  // Kursory svislé
  unsigned int cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsKey.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsKey.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsKey.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsKey.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist <= cur2dist) {
    if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
      mouseDrag = MouseDrag::cursorX1;
      this->setInteraction(QCP::iRangeDrag, false);
      return;
    }
  } else if (cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    mouseDrag = MouseDrag::cursorX2;
    this->setInteraction(QCP::iRangeDrag, false);
    return;
  }

  // Kursory vodorovné
  cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsVal.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsVal.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsVal.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsVal.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist <= cur2dist) {
    if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
      mouseDrag = MouseDrag::cursorY1;
      this->setInteraction(QCP::iRangeDrag, false);
      return;
    }
  } else if (cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    mouseDrag = MouseDrag::cursorY2;
    this->setInteraction(QCP::iRangeDrag, false);
    return;
  }

// Offset
  nearestIndex = -1;
  nearestDistance = PLOT_ELEMENTS_MOUSE_DISTANCE;
  for (int i = 0; i < zeroLines.count(); i++) {
    if (zeroLines.at(i)->visible() || isChUsed(i)) {
      unsigned int distance = (unsigned int)zeroLines.at(i)->selectTest(event->pos(), false);
      if (distance < nearestDistance) {
        nearestIndex = i;
        nearestDistance = distance;
      }
    }
  }
  if (nearestIndex != -1) {
    mouseDragChIndex = nearestIndex;
    mouseDrag = MouseDrag::zeroline;
    this->setInteraction(QCP::iRangeDrag, false);
  }
}

void MyMainPlot::setMouseCursorStyle(QMouseEvent* event) {
  // Kursory svislé
  unsigned int cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsKey.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsKey.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsKey.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsKey.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE || cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    this->QWidget::setCursor(Qt::SizeHorCursor); // Cursor myši, ne ten grafový
    return;
  }

  // Kursory vodorovné
  cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsVal.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsVal.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsVal.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsVal.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE || cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    this->QWidget::setCursor(Qt::SizeVerCursor); // Cursor myši, ne ten grafový
    return;
  }

  // Offset
  for (int i = 0; i < zeroLines.count(); i++) {
    if (zeroLines.at(i)->visible() || isChUsed(i)) {
      unsigned int distance = (unsigned int)zeroLines.at(i)->selectTest(event->pos(), false);
      if (distance < PLOT_ELEMENTS_MOUSE_DISTANCE) {
        this->QWidget::setCursor(Qt::SizeVerCursor); // Cursor myši, ne ten grafový
        return;
      }
    }
  }

  // Nic
  this->QWidget::setCursor(defaultMouseCursor); // Cursor myši, ne ten grafový
}
