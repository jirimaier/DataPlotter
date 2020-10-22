#include "myxyplot.h"

MyXYPlot::MyXYPlot(QWidget *parent) : QCustomPlot(parent) {
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  graphXY = new QCPCurve(this->xAxis, this->yAxis);
  this->xAxis->setRange(-100, 100);
  this->yAxis->setRange(-100, 100);
  initcursors();
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

  this->xAxis->setRange(left, right);
  this->yAxis->setRange(down, up);

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

void MyXYPlot::updateCursors(double *cursorPositions) {
  for (int i = 0; i < 4; i++) {
    if (i < 2) {
      cursors.at(i)->start->setCoords(cursorPositions[i], 0);
      cursors.at(i)->end->setCoords(cursorPositions[i], 1);
    } else {
      cursors.at(i)->start->setCoords(0, cursorPositions[i]);
      cursors.at(i)->end->setCoords(1, cursorPositions[i]);
    }
  }
}

void MyXYPlot::initcursors() {
  QPen cursorpen;
  cursorpen.setColor(Qt::black);
  for (int i = 0; i < 4; i++) {
    cursors.append(new QCPItemLine(this));
    if (i < 2) {
      cursors.at(i)->start->setTypeX(QCPItemPosition::ptPlotCoords);
      cursors.at(i)->start->setTypeY(QCPItemPosition::ptViewportRatio);
      cursors.at(i)->end->setTypeX(QCPItemPosition::ptPlotCoords);
      cursors.at(i)->end->setTypeY(QCPItemPosition::ptViewportRatio);
    } else {
      cursors.at(i)->start->setTypeX(QCPItemPosition::ptViewportRatio);
      cursors.at(i)->start->setTypeY(QCPItemPosition::ptPlotCoords);
      cursors.at(i)->end->setTypeX(QCPItemPosition::ptViewportRatio);
      cursors.at(i)->end->setTypeY(QCPItemPosition::ptPlotCoords);
    }
    cursors.at(i)->setPen(cursorpen);
    cursors.at(i)->setVisible(false);
  }
}

void MyXYPlot::setCursorsAccess(bool allowed) {
  for (int i = 0; i < 4; i++)
    cursors.at(i)->setVisible(allowed);
  iHaveCursors = allowed;
}
