#ifndef MYFFTPLOT_H
#define MYFFTPLOT_H

#include <QObject>

#include "myplot.h"

class MyFFTPlot : public MyPlot {
  Q_OBJECT
 public:
  explicit MyFFTPlot(QWidget* parent = nullptr);
  QByteArray exportCSV(char separator, char decimal, int precision);
  QPair<unsigned int, unsigned int> getVisibleSamplesRange();


 private:
  bool autoSize = true;
  void autoset();

 public slots:
  void newData(QSharedPointer<QCPGraphDataContainer> data);
  void clear();
  void setStyle(int style);
  void setAutoSize(bool en);

 private slots:
  void moveTracer(QMouseEvent* event);

 signals:
  void lengthChanged(int formerLength, int newLength);
};

#endif // MYFFTPLOT_H
