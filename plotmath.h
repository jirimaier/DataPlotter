#ifndef PLOTMATH_H
#define PLOTMATH_H

#include "enums_defines_constants.h"
#include <QDebug>
#include <QObject>
#include <QThread>

class PlotMath : public QObject {
  Q_OBJECT
public:
  explicit PlotMath(QObject *parent = nullptr);
  ~PlotMath();
public slots:
  void doMath(int resultCh, int operation, QPair<QVector<double>, QVector<double>> data, QPair<QVector<double>, QVector<double>> data2);
signals:
  void sendResult(int ch, QVector<double> *time, QVector<double> *value, bool continous, bool sorted, bool ignorePause = true);
};

#endif // PLOTMATH_H
