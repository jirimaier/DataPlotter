#include "interpolator.h"

Interpolator::Interpolator(QObject* parent) : QObject(parent) {
  QFile firFile(":/text/interpolationFIR.csv");
  firFile.open(QFile::ReadOnly | QFile::Text);
  QByteArrayList fir = firFile.readAll().split(',');
  foreach (QByteArray value, fir)
    interpolationFIR.append(value.toDouble());
}

void Interpolator::interpolate(int chID, const QSharedPointer<QCPGraphDataContainer> data, QCPRange visibleRange, bool dataIsFromInterpolationBuffer) {
  double fs = (data->size() - 1) / (data->at(data->size() - 1)->key - data->at(0)->key);
  double resultSamplingPeriod = 1.0 / INTERPOLATION_UPSAMPLING / fs;

  QVector<float> values;

  auto dataBegin = data->begin(); // Při volání této funkce se změní adresy v data!!!
  auto dataEnd = data->end();

  int samplePaddings = (interpolationFIR.size() - 1) / 2 / INTERPOLATION_UPSAMPLING;
  int timePaddings = visibleRange.size() / 2;

  const QCPGraphData* begin = data->findBegin(visibleRange.lower - timePaddings) - samplePaddings;
  const QCPGraphData* end = data->findEnd(visibleRange.upper + timePaddings) + samplePaddings;

  if (begin < dataBegin)
    begin = dataBegin;
  if (end > dataEnd)
    end = dataEnd;

  for (auto it = begin; it != end; it++) {
    values.append(it->value);
    for (int i = 1; i < INTERPOLATION_UPSAMPLING; i++)
      values.append(0);
  }

  if (values.length() < interpolationFIR.size()) {
    // Moc málo vzorků, nemá to cenu
    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*data));
    emit interpolationResult(chID, data, result, dataIsFromInterpolationBuffer);
    return;
  }

  if (values.length() > 10000) {
    // Hodně vzorků, nemá cenu interpolovat
    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*data));
    emit interpolationResult(chID, data, result, dataIsFromInterpolationBuffer);
    return;
  }

  values = convolute(values, interpolationFIR);

  values.remove(0, (interpolationFIR.length() - 1) / 2);

  auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
  for (int i = 0; i < values.size(); i++) {
    result->add(QCPGraphData((begin + i / INTERPOLATION_UPSAMPLING)->key + (i % INTERPOLATION_UPSAMPLING)*resultSamplingPeriod, values.at(i)*INTERPOLATION_UPSAMPLING));
  }

  emit interpolationResult(chID, data, result, dataIsFromInterpolationBuffer);
  emit finished(chID);
}

QVector<float> Interpolator::convolute(QVector<float> x, QVector<float> h) {
  QVector<float> y;

  int N = x.length();
  int M = h.length();

  y.resize(N);

  for (int n = 0; n < N; n++) {
    for (int k = 0; k < M; k++) {
      if (n - k >= 0)
        y[n] += x[n - k] * h[k];
    }
  }

  return y;
}
