//  Copyright (C) 2020  Jiří Maier

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

#include "myxyplot.h"

MyXYPlot::MyXYPlot(QWidget *parent) : MyPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  graphXY = new QCPCurve(this->xAxis, this->yAxis);
  this->xAxis->setRange(-100, 100);
  this->yAxis->setRange(-100, 100);
  setGridHintX(-2);
  setGridHintY(-2);
}

MyXYPlot::~MyXYPlot() {}

void MyXYPlot::newData(QSharedPointer<QCPCurveDataContainer> data, bool ignorePause) {
  if (data->size() == 1) {
    if (plottingStatus == PlotStatus::run || ignorePause) {
      if (data->at(0)->t <= graphXY->data()->at(graphXY->data()->size() - 1)->t)
        graphXY->data().data()->clear();
      graphXY->addData(data->at(0)->t, data->at(0)->key, data->at(0)->value);
      if (useRange)
        graphXY->data()->removeBefore(data->at(0)->t - timeRange);
    } else {
      if (data->at(0)->t <= pauseBuffer->at(pauseBuffer->size() - 1)->t)
        pauseBuffer->clear();
      pauseBuffer->add(QCPCurveData(data->at(0)->t, data->at(0)->key, data->at(0)->value));
      if (useRange)
        pauseBuffer->removeBefore(data->at(0)->t - timeRange);
    }
  } else {
    if (plottingStatus == PlotStatus::run || ignorePause)
      graphXY->setData(data);
    else
      pauseBuffer = data;
  }
  if (plottingStatus == PlotStatus::run || ignorePause) {
    if (autoSize)
      autoset();
    this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  }
}

void MyXYPlot::setAutoSize(bool en) {
  autoSize = en;
  setMouseControlls(!en);
  autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyXYPlot::clear() {
  graphXY->data().data()->clear();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyXYPlot::setStyle(int style) {
  if (style == GraphStyle::line) {
    graphXY->setScatterStyle(QCPScatterStyle::ssNone);
    graphXY->setLineStyle(QCPCurve::lsLine);
  } else if (style == GraphStyle::point) {
    graphXY->setScatterStyle(POINT_STYLE);
    graphXY->setLineStyle(QCPCurve::lsNone);
  } else if (style == GraphStyle::linePoint) {
    graphXY->setScatterStyle(POINT_STYLE);
    graphXY->setLineStyle(QCPCurve::lsLine);
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

QByteArray MyXYPlot::exportCSV(char separator, char decimal, int precision) {
  QByteArray output = (QString("X%1Y\n").arg(separator)).toUtf8();
  for (QCPCurveDataContainer::iterator it = graphXY->data()->begin(); it != graphXY->data()->end(); it++) {
    output.append(QString::number(it->key, 'f', precision).replace('.', decimal).toUtf8());
    output.append(separator);
    output.append(QString::number(it->value, 'f', precision).replace('.', decimal).toUtf8());
    output.append('\n');
  }
  return output;
}

void MyXYPlot::resume() {
  plottingStatus = PlotStatus::run;
  graphXY->setData(pauseBuffer);
  if (autoSize)
    autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  pauseBuffer.clear();
}

void MyXYPlot::pause() {
  plottingStatus = PlotStatus::pause;
  pauseBuffer = QSharedPointer<QCPCurveDataContainer>(new QCPCurveDataContainer(*graphXY->data())); // Zkopíruje existující data do bufferu
}

void MyXYPlot::autoset() {
  bool foundrange, foundrange2;
  QCPRange yRange = graphXY->data()->valueRange(foundrange);
  QCPRange xRange = graphXY->data()->keyRange(foundrange2);
  yRange.lower = GlobalFunctions::ceilToNiceValue(yRange.lower);
  yRange.upper = GlobalFunctions::ceilToNiceValue(yRange.upper);
  xRange.lower = GlobalFunctions::ceilToNiceValue(xRange.lower);
  xRange.upper = GlobalFunctions::ceilToNiceValue(xRange.upper);
  xAxis->setRange(xRange);
  yAxis->setRange(yRange);
}

void MyXYPlot::togglePause() {
  if (plottingStatus == PlotStatus::run)
    pause();
  else
    resume();
}

void MyXYPlot::setTimeRange(double interval) {
  timeRange = interval;
  if (useRange) {
    if (plottingStatus == PlotStatus::run)
      graphXY->data()->removeBefore(graphXY->data()->at(graphXY->data()->size() - 1)->t - timeRange);
    else
      pauseBuffer->removeBefore(pauseBuffer->at(pauseBuffer->size())->t - timeRange);
  }
}

void MyXYPlot::setUseTimeRange(bool en) {
  useRange = en;
  setTimeRange(timeRange); // Aby se vymazali případné přebytečné
}

void MyXYPlot::setCenter(double value) {
  center = value;
  autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyXYPlot::setRange(double value) {
  range = value;
  autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}
