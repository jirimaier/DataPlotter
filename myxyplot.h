#ifndef MYXYPLOT_H
#define MYXYPLOT_H

#include "myplot.h"

class MyXYPlot : public MyPlot {
  Q_OBJECT
public:
  MyXYPlot(QWidget *parent = nullptr);
  void newData(QVector<double> *x, QVector<double> *y);
  QByteArray exportCSV(char separator, char decimal, int precision);

private:
  bool autoSize = true;
  QCPCurve *graphXY;

public slots:
  void setAutoSize(bool en);
};

#endif // MYXYPLOT_H
