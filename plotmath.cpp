#include "plotmath.h"

PlotMath::PlotMath(QObject *parent) : QObject(parent) {}

void PlotMath::doMath(int resultCh, int operation, QPair<QVector<double>, QVector<double>> data1, QPair<QVector<double>, QVector<double>> data2) {
  QVector<double> *result = new QVector<double>;
  QVector<double> *resultTime = new QVector<double>;
  int length = MIN(data1.first.length(), data2.first.length());
  for (int i = 0; i < length; i++) {
    if (operation == MATH_ADD) {
      result->append(data1.second.at(i) + data2.second.at(i));
      resultTime->append(data1.first.at(i));
    } else if (operation == MATH_SUB) {
      result->append(data1.second.at(i) - data2.second.at(i));
      resultTime->append(data1.first.at(i));
    } else if (operation == MATH_MUL) {
      result->append(data1.second.at(i) * data2.second.at(i));
      resultTime->append(data1.first.at(i));
    } else if (operation == MATH_DIV) {
      result->append(data1.second.at(i) / data2.second.at(i));
      resultTime->append(data1.first.at(i));
    } else if (operation == MATH_XY) {
      result->append(data2.second.at(i));
      resultTime->append(data1.second.at(i));
    }
  }
  emit sendResult(resultCh, resultTime, result, false, operation != MATH_XY);
}
