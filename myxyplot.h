#ifndef MYXYPLOT_H
#define MYXYPLOT_H

#include "myplot.h"

class MyXYPlot : public MyPlot {
  Q_OBJECT
public:
  explicit MyXYPlot(QWidget *parent = nullptr);
  ~MyXYPlot();
  void newData(QSharedPointer<QCPCurveDataContainer> data);
  QByteArray exportCSV(char separator, char decimal, int precision);

private:
  bool autoSize = true;
  QCPCurve *graphXY;
  void autoset();

public slots:
  void setAutoSize(bool en);
  void clear();
  void setStyle(int style);
};

#endif // MYXYPLOT_H
