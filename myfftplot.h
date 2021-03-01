#ifndef MYFFTPLOT_H
#define MYFFTPLOT_H

#include <QObject>

#include "myplot.h"

class MyFFTPlot : public MyPlot {
  Q_OBJECT
 public:
  explicit MyFFTPlot(QWidget* parent = nullptr);
  QByteArray exportCSV(char separator, char decimal, int precision);
  QPair<unsigned int, unsigned int> getVisibleSamplesRange(int ch);

  /// Přizpůsobý barvu a popis, vrátí true, pokud se barva změnila
  bool setChSorce(int ch, int sourceChannel, QColor color);

 private:
  bool autoSize = true;
  void autoset();
  void setMouseCursorStyle(QMouseEvent* event);
  void updateTracerText(int index);
  int currentTracerIndex = -1;
  int chSourceChannel[2];
  QColor chSourceColor[2];
  QPair<QVector<double>, QVector<double> > getDataVector(int chID);

 public slots:
  void newData(int ch, QSharedPointer<QCPGraphDataContainer> data);
  void clear(int ch);
  void clear();
  void setStyle(int ch, int style);
  void setAutoSize(bool en);

 private slots:
  void mouseMoved(QMouseEvent* event);
  void mousePressed(QMouseEvent* event);
};

#endif // MYFFTPLOT_H
