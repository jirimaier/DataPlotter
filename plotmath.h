#ifndef PLOTMATH_H
#define PLOTMATH_H

#include "settings.h"
#include <QObject>

class PlotMath : public QObject {
  Q_OBJECT
public:
  explicit PlotMath(QObject *parent = nullptr);
public slots:
  void doMath(int resultCh, int operation, QPair<QVector<double>, QVector<double>> data, QPair<QVector<double>, QVector<double>> data2);
signals:
  void sendResult(int ch, QVector<double> *time, QVector<double> *value, bool continous, bool sorted);
};

#endif // PLOTMATH_H
