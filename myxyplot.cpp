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

void MyXYPlot::newData(QSharedPointer<QCPCurveDataContainer> data) {
  graphXY->setData(data);
  if (autoSize)
    autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);

  // Přepsat text u traceru
  if (tracer->visible()) {
    QString tracerTextStr;
    tracerTextStr.append("X: " + GlobalFunctions::floatToNiceString(tracer->position->key(), 4, true, true) + getXUnit() + "\n");
    tracerTextStr.append("Y: " + GlobalFunctions::floatToNiceString(tracer->position->value(), 4, true, true) + getYUnit() + "\n");
    tracerTextStr.append("t: " + GlobalFunctions::floatToNiceString(graphXY->data().data()->at(tracer->sampleNumber())->t, 4, true, true) + tUnit);
    tracerText->setText(tracerTextStr);
    tracerLayer->replot();
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

void MyXYPlot::showTracer(QMouseEvent *event) {
  if ((unsigned int)graphXY->selectTest(event->pos(), false) < 20) {
    tracer->setVisible(true);
    tracerText->setVisible(true);
    tracer->setCurve(graphXY);
    tracer->setPoint(event->pos());
    QString tracerTextStr;
    tracerTextStr.append("X: " + GlobalFunctions::floatToNiceString(tracer->position->key(), 4, false, false) + getXUnit() + "\n");
    tracerTextStr.append("Y: " + GlobalFunctions::floatToNiceString(tracer->position->value(), 4, false, false) + getYUnit() + "\n");
    tracerTextStr.append("t: " + GlobalFunctions::floatToNiceString(graphXY->data().data()->at(tracer->sampleNumber())->t, 4, false, false) + tUnit);
    tracerText->setText(tracerTextStr);
    checkIfTracerTextFits();
    tracerLayer->replot();

    if (mouseIsPressed) {
      this->setInteraction(QCP::iRangeDrag, false);
      emit moveCursor(XYID, event->buttons() == Qt::RightButton ? 2 : 1, tracer->sampleNumber());
    }
  } else
    hideTracer();
}
