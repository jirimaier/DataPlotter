//  Copyright (C) 2020-2021  Jiří Maier

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "interpolator.h"

Interpolator::Interpolator(QObject* parent) : QObject(parent) {
  QFile firFile(":/text/interpolationFIR.csv");
  if (firFile.open(QFile::ReadOnly | QFile::Text)) {
    QByteArrayList fir = firFile.readAll().split(',');
    foreach (QByteArray value, fir)
      lowPassFIR.append(value.toDouble());
    firFile.close();
  } else {
    qDebug() << "Can not load FIR filter coeficients!";
  }
}

void Interpolator::interpolate(int chID, const QSharedPointer<QCPGraphDataContainer> data, QCPRange visibleRange, bool dataIsFromInterpolationBuffer) {
  int M = lowPassFIR.size() - 1;

  double fs = (data->size() - 1) / (data->at(data->size() - 1)->key - data->at(0)->key);
  double resultSamplingPeriod = 1.0 / INTERPOLATION_UPSAMPLING / fs;

  QVector<float> values;

  auto dataBegin = data->constBegin(); // Při volání této funkce se změní adresy v data!!!
  auto dataEnd = data->constEnd();

  int samplePaddings = M / 2 / INTERPOLATION_UPSAMPLING;
  double timePaddings = visibleRange.size() / 2;

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

  if (values.length() < lowPassFIR.size()) {
    // Moc málo vzorků, nemá to cenu
    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*data));
    emit interpolationResult(chID, data, result, dataIsFromInterpolationBuffer);
    return;
  }

  if (values.length() > 2000 * INTERPOLATION_UPSAMPLING) {
    // Hodně vzorků, nemá cenu interpolovat
    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*data));
    emit interpolationResult(chID, data, result, dataIsFromInterpolationBuffer);
    return;
  }

  values = filter(values, lowPassFIR);

  auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
  for (int i = 0; i < values.size(); i++)
    result->add(QCPGraphData((begin + (i + M / 2) / INTERPOLATION_UPSAMPLING)->key + ((i + M / 2) % INTERPOLATION_UPSAMPLING)*resultSamplingPeriod, values.at(i)*INTERPOLATION_UPSAMPLING));

  emit interpolationResult(chID, data, result, dataIsFromInterpolationBuffer);
  emit finished(chID);
}

/// Provede konvoluci s odezvou (koeficienty) FIR filtru prvních M/2 a posledních M/2 vzorků je vynecháno (přechodné jevy).
/// Výstup se tedy předbíhá o M/2 oproti vstupu (filtrovaný signál by se o M/2 zpožďoval, ale na začátku je vynecháno M vzorků)
QVector<float> Interpolator::filter(QVector<float> x, QVector<float> h) {
  QVector<float> y;

  int N = x.length();     // Délka signálu
  int M = h.length(); // Délka odezvy filtru

  y.resize(N - M + 1);

  for (int n = M - 1; n < N; n++) {
    for (int k = 0; k < M; k++) {
      y[n - M + 1] += x[n - k] * h[k];
    }
  }

  return y;
}
