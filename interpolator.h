#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <QObject>

#include "enumsDefinesConstants.h"
#include "qcustomplot.h"

class Interpolator : public QObject {
  Q_OBJECT
 public:
  QVector<float> FIRfilter;
  explicit Interpolator(QObject* parent = nullptr);

 private:
  QVector<float> convolute(QVector<float> x, QVector<float> h);

 public slots:
  void interpolate(int chID, const QSharedPointer<QCPGraphDataContainer> data, QCPRange visibleRange, bool dataIsFromInterpolationBuffer);

 signals:
  void interpolationResult(int chID, QSharedPointer<QCPGraphDataContainer> dataOriginal, QSharedPointer<QCPGraphDataContainer> dataInterpolated, bool dataIsFromInterpolationBuffer);
  void finished(int chID);
};

#endif // INTERPOLATOR_H
