#include "myfftplot.h"

MyFFTPlot::MyFFTPlot(QWidget *parent) : MyPlot(parent) {
  addGraph();
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  this->xAxis->setRange(0, 1000);
  this->yAxis->setRange(0, 100);
  setGridHintX(-2);
  setGridHintY(-2);
}

QByteArray MyFFTPlot::exportCSV(char separator, char decimal, int precision) {
  QByteArray output = (QString("frequency%1amplitude\n").arg(separator)).toUtf8();
  for (QCPGraphDataContainer::iterator it = graph(0)->data()->begin(); it != graph(0)->data()->end(); it++) {
    output.append(QString::number(it->key, 'f', precision).replace('.', decimal).toUtf8());
    output.append(separator);
    output.append(QString::number(it->value, 'f', precision).replace('.', decimal).toUtf8());
    output.append('\n');
  }
  return output;
}

void MyFFTPlot::newData(QSharedPointer<QCPGraphDataContainer> data) {
  graph(0)->setData(data);
  if (autoSize) autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::setStyle(int style) {
  if (style == GraphStyle::line) {
    graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    graph(0)->setLineStyle(QCPGraph::lsLine);
  } else if (style == GraphStyle::point) {
    graph(0)->setScatterStyle(POINT_STYLE);
    graph(0)->setLineStyle(QCPGraph::lsNone);
  } else if (style == GraphStyle::linePoint) {
    graph(0)->setScatterStyle(POINT_STYLE);
    graph(0)->setLineStyle(QCPGraph::lsLine);
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::setAutoSize(bool en) {
  autoSize = en;
  setMouseControlls(!en);
  autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::autoset() {
  bool foundrange, foundrange2;
  QCPRange yRange = graph(0)->data()->valueRange(foundrange);
  QCPRange xRange = graph(0)->data()->keyRange(foundrange2);
  yRange.lower = GlobalFunctions::ceilToNiceValue(yRange.lower);
  yRange.upper = GlobalFunctions::ceilToNiceValue(yRange.upper);
  xRange.lower = GlobalFunctions::ceilToNiceValue(xRange.lower);
  xRange.upper = GlobalFunctions::ceilToNiceValue(xRange.upper);
  xAxis->setRange(xRange);
  yAxis->setRange(yRange);
}
