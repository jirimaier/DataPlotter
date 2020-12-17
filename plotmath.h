#ifndef PLOTMATH_H
#define PLOTMATH_H

#include "enums_defines_constants.h"
#include <QDebug>
#include <QObject>
#include <QThread>

#include "qcustomplot.h"

class PlotMath : public QObject {
  Q_OBJECT
public:
  explicit PlotMath(QObject *parent = nullptr);
  ~PlotMath();

private:
  QVector<QSharedPointer<QCPGraphDataContainer>> firsts, seconds = QVector<QSharedPointer<QCPGraphDataContainer>>();
  QSharedPointer<QCPGraphDataContainer> XYx, XYy;
  MathOperations::enumetrator operations[MATH_COUNT];
public slots:
  void clearMathFirst(int math) { firsts[math - 1] = nullptr; }
  void clearMathSecond(int math) { seconds[math - 1] = nullptr; }
  void clearXYFirst() { XYx = nullptr; }
  void clearXYSecond() { XYy = nullptr; }
  void addMathData(int mathNumber, bool isFirst, QSharedPointer<QCPGraphDataContainer> in);
  void setMathMode(int math, MathOperations::enumetrator mode) {
    operations[math - 1] = mode;
    sendResult(GlobalFunctions::getAnalogChId(math, ChannelType::math), QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer()), true);
  }
  void addXYData(bool isFirst, QSharedPointer<QCPGraphDataContainer> in);
signals:
  void sendResult(int chNumber, QSharedPointer<QCPGraphDataContainer> result, bool ignorePause);
  void sendResultXY(QSharedPointer<QCPCurveDataContainer> result);
};

#endif // PLOTMATH_H
