#include "channel.h"

#include <QColorDialog>

#include "qcustomplot.h"

Channel::Channel(int number, QCustomPlot *plot, QColor color) {
  this->dataChannelNumber = (number - 1);
  this->plot = plot;
  this->plot->addGraph();
  this->color = color;
  dataPen.setWidth(1);
  zeroPen.setWidth(1);
  dataPen.setColor(color);
  zeroPen.setColor(color);
  zeroPen.setStyle(Qt::DotLine);
  this->plot->graph(dataChannelNumber)->setPen(dataPen);
  zeroLine = new QCPItemLine(plot);
  zeroLine->setPen(zeroPen);
  zeroLine->start->setTypeY(QCPItemPosition::ptPlotCoords);
  zeroLine->start->setTypeX(QCPItemPosition::ptViewportRatio);
  zeroLine->end->setTypeY(QCPItemPosition::ptPlotCoords);
  zeroLine->end->setTypeX(QCPItemPosition::ptViewportRatio);
}

void Channel::clear() {
  if (!this->isEmpty())
    lastSetMaxTime = time.at(time.length() - 1);
  else
    lastSetMaxTime = -INFINITY;
  time.clear();
  value.clear();
}

void Channel::addValue(QByteArray in_value, QByteArray in_time) {
  double in_time_double = in_time.length() > 0 ? in_time.toDouble() : time.length();
  if (this->time.length() > 0)
    if (in_time_double <= this->time.at(this->time.length() - 1))
      clear();
  this->time.append(in_time_double);
  this->value.append(in_value.toDouble() + offset);
}

void Channel::addValue(double in_value, double in_time) {
  if (this->time.length() > 0)
    if (in_time <= this->time.at(this->time.length() - 1))
      clear();
  this->time.append(in_time);
  this->value.append(in_value + offset);
}

void Channel::draw() {
  plot->graph(this->dataChannelNumber)->setData(time, value, true);
  lastDrawnIndex = time.length() - 1;
}

double Channel::lastDrawnTime() {
  if (lastDrawnIndex >= time.length() || lastDrawnIndex < 0)
    return 0;
  else
    return time.at(lastDrawnIndex);
}

void Channel::setStyle(int type) {
  style = type;
  plot->graph(dataChannelNumber)->setVisible(!(style == PLOT_STYLE_HIDDEN));
  zeroLine->setVisible(!(style == PLOT_STYLE_HIDDEN));

  if (style == PLOT_STYLE_LINEANDPIONT) {
    plot->graph(dataChannelNumber)->setScatterStyle(QCPScatterStyle::ssDisc);
    plot->graph(dataChannelNumber)->setLineStyle(QCPGraph::lsLine);
  }
  if (style == PLOT_STYLE_LINE) {
    plot->graph(dataChannelNumber)->setScatterStyle(QCPScatterStyle::ssNone);
    plot->graph(dataChannelNumber)->setLineStyle(QCPGraph::lsLine);
  }
  if (style == PLOT_STYLE_POINT) {
    plot->graph(dataChannelNumber)->setScatterStyle(QCPScatterStyle::ssDisc);
    plot->graph(dataChannelNumber)->setLineStyle(QCPGraph::lsNone);
  }
}

double Channel::upperTimeRange() { return (lastDrawnTime() < lastSetMaxTime) ? lastSetMaxTime : lastDrawnTime(); }

void Channel::changeColor(QColor color) {
  this->color = color;
  this->plot->graph(this->dataChannelNumber)->setPen(QPen(color));
}

void Channel::changeOffset(double offset) {
  QCPGraphDataContainer *data = plot->graph(dataChannelNumber)->data().data();
  for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++)
    it->value += offset - this->offset;
  for (QVector<double>::iterator it = value.begin(); it != value.end(); it++)
    *it += offset - this->offset;
  this->offset = offset;
  zeroLine->start->setCoords(0, offset);
  zeroLine->end->setCoords(1, offset);
}
