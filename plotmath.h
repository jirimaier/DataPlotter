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
  void doMath(int resultChNumber, int operation, QPair<QVector<double>, QVector<double>> data, QPair<QVector<double>, QVector<double>> data2);
  void doXY(QPair<QVector<double>, QVector<double>> data1, QPair<QVector<double>, QVector<double>> data2);
signals:
  void sendResult(int chNumber, QVector<double> *time, QVector<double> *value, bool isMath = true);
  void sendResultXY(QVector<double> *time, QVector<double> *value);
};

#endif // PLOTMATH_H
