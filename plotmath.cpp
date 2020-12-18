#include "plotmath.h"

PlotMath::PlotMath(QObject *parent) : QObject(parent) {
  qDebug() << "PlotMath created from " << QThread::currentThreadId();
  firsts.resize(MATH_COUNT);
  seconds.resize(MATH_COUNT);
  for (int i = 0; i < MATH_COUNT; i++)
    operations[i] = MathOperations::add;
}

PlotMath::~PlotMath() { qDebug() << "PlotMath deleted from " << QThread::currentThreadId(); }

void PlotMath::addMathData(int mathNumber, bool isFirst, QSharedPointer<QCPGraphDataContainer> in) {
  QSharedPointer<QCPGraphDataContainer> &first = firsts[mathNumber];
  QSharedPointer<QCPGraphDataContainer> &second = seconds[mathNumber];
  if (isFirst)
    first = in;
  else
    second = in;

  if (!first.isNull() && !second.isNull()) {
    if (first->size() != second->size()) {
      qDebug() << "Math: not same length";
      return;
    }
    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
    for (int i = 0; i < first->size(); i++) {
      double resultSample = first->at(i)->value;
      if (operations[mathNumber] == MathOperations::add)
        resultSample += second->at(i)->value;
      else if (operations[mathNumber] == MathOperations::subtract)
        resultSample -= second->at(i)->value;
      else if (operations[mathNumber] == MathOperations::multiply)
        resultSample *= second->at(i)->value;
      else if (operations[mathNumber] == MathOperations::divide)
        resultSample /= second->at(i)->value;
      result->add(QCPGraphData(first->at(i)->key, resultSample));
    }
    emit sendResult(GlobalFunctions::getAnalogChId(mathNumber + 1, ChannelType::math), result, false);
    first.clear();
    second.clear();
  }
}

void PlotMath::addXYData(bool isFirst, QSharedPointer<QCPGraphDataContainer> in) {
  if (isFirst)
    XYx = in;
  else
    XYy = in;

  if (!XYx.isNull() && !XYy.isNull()) {
    if (XYx->size() != XYy->size()) {
      qDebug() << "Math: not same length";
    }
    auto result = QSharedPointer<QCPCurveDataContainer>(new QCPCurveDataContainer());
    for (int i = 0; i < XYx->size(); i++) {
      result->add(QCPCurveData(XYx->at(i)->key, XYx->at(i)->value, XYy->at(i)->value));
    }
    emit sendResultXY(result);
    XYx.clear();
    XYy.clear();
  }
}
