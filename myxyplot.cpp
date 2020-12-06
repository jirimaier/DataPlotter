#include "myxyplot.h"

MyXYPlot::MyXYPlot(QWidget *parent) : MyPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  graphXY = new QCPCurve(this->xAxis, this->yAxis);
  this->xAxis->setRange(-100, 100);
  this->yAxis->setRange(-100, 100);
}

MyXYPlot::~MyXYPlot() {}

void MyXYPlot::newData(QVector<double> *x, QVector<double> *y) {
  double left = *std::min_element(x->begin(), x->end());
  double right = *std::max_element(x->begin(), x->end());
  double down = *std::min_element(y->begin(), y->end());
  double up = *std::max_element(y->begin(), y->end());

  graphXY->setData(*x, *y);
  delete x;
  delete y;

  left *= 1.1;
  right *= 1.1;
  up *= 1.1;
  down *= 1.1;

  left = Global::logaritmicSettings[GlobalFunctions::roundToStandardValue(abs(left))] * (left < 0 ? (-1) : 1);
  right = Global::logaritmicSettings[GlobalFunctions::roundToStandardValue(abs(right))] * (right < 0 ? (-1) : 1);
  up = Global::logaritmicSettings[GlobalFunctions::roundToStandardValue(abs(up))] * (up < 0 ? (-1) : 1);
  down = Global::logaritmicSettings[GlobalFunctions::roundToStandardValue(abs(down))] * (down < 0 ? (-1) : 1);

  this->setVerticalDiv((this->yAxis->range().upper - this->yAxis->range().lower) / 5);
  this->setHorizontalDiv((this->xAxis->range().upper - this->xAxis->range().lower) / 5);

  if (autoSize) {
    this->xAxis->setRange(left, right);
    this->yAxis->setRange(down, up);
  }

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
