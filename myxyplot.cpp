#include "myxyplot.h"

MyXYPlot::MyXYPlot(QWidget *parent) : MyPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  graphXY = new QCPCurve(this->xAxis, this->yAxis);
  this->xAxis->setRange(-100, 100);
  this->yAxis->setRange(-100, 100);
}

void MyXYPlot::newData(QVector<double> *x, QVector<double> *y) {
  double left = *std::min_element(x->begin(), x->end());
  double right = *std::max_element(x->begin(), x->end());
  double down = *std::min_element(y->begin(), y->end());
  double up = *std::max_element(y->begin(), y->end());

  left = floor(left / 20.0) * 20;
  right = ceil(right / 20.0) * 20;
  up = ceil(up / 20.0) * 20;
  down = floor(down / 20.0) * 20;

  if (autoSize) {
    this->xAxis->setRange(left, right);
    this->yAxis->setRange(down, up);
  }

  if (iHaveCursors) {
    PlotFrame_t frame;
    frame.xMinTotal = left;
    frame.xMaxTotal = right;
    frame.yMinTotal = down;
    frame.yMaxTotal = up;
    frame.xMinView = this->xAxis->range().lower;
    frame.xMaxView = this->xAxis->range().upper;
    frame.yMinView = this->yAxis->range().lower;
    frame.yMaxView = this->yAxis->range().upper;
    emit setCursorBounds(frame);
  }

  graphXY->setData(*x, *y);
  delete x;
  delete y;
  this->replot();
}

void MyXYPlot::setAutoSize(bool en) {
  autoSize = en;
  setMouseControlls(!en);
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
