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

#include "signalprocessing.h"

#include <QElapsedTimer>

SignalProcessing::SignalProcessing(QObject *parent) : QObject(parent) {}

void SignalProcessing::calculateHamming(int length) {
  if (hamming.size() != length) {
    hamming.resize(length);
    for (int i = 0; i < length; i++) hamming[i] = 0.54 - 0.46 * cos(2 * M_PI * i / length);
  }
  QString txt = "";
  foreach (double d, hamming) { txt.append(QString::number(d) + "\n"); }
  QFile file("C:/Users/Maier/Desktop/hamm.csv");
  if (file.open(QFile::WriteOnly | QFile::Truncate)) {
    file.write(txt.toLocal8Bit());
  }
}

QVector<std::complex<float>> SignalProcessing::fft(QVector<std::complex<float>> signal) {
  int n = signal.size();  // NFFT
  if (n == 1) return signal;
  std::complex<float> W = std::exp(std::complex<float>(0, 2.0 * M_PI / (float)n));  // exp(2*Pi*i/n)

  QVector<std::complex<float>> Pe, Po;  // Sudé a liché koeficienty
  for (int i = 1; i <= n; i++) {
    if (i % 2)  // Liché
      Po.append(signal.at(i - 1));
    else  // Sudé
      Pe.append(signal.at(i - 1));
  }

  QVector<std::complex<float>> ye = fft(Pe), yo = fft(Po);  // Rekurse

  QVector<std::complex<float>> result;
  result.resize(n);
  for (int i = 0; i < n / 2; i++) {
    std::complex<float> Wi = std::pow(W, i);
    result[i] = ye[i] + Wi * yo[i];
    result[i + n / 2] = ye[i] - Wi * yo[i];
  }
  return result;
}

int SignalProcessing::nextPow2(int number) {
  for (int i = 1;; i++)
    if (pow(2, i) >= number) return (pow(2, i));
}

void SignalProcessing::plotFFT(QSharedPointer<QCPGraphDataContainer> data, bool dB, FFTWindow::enumerator window) {
  bool rangefound = false;
  double fs = 1 / ((data->keyRange(rangefound).upper - data->keyRange(rangefound).lower) / data->size());

  QVector<std::complex<float>> values;
  if (window == FFTWindow::rectangular)
    for (int i = 0; i < data->size(); i++) values.append(data->at(i)->value);
  else if (window == FFTWindow::hamming) {
    calculateHamming(data->size());
    for (int i = 0; i < data->size(); i++) values.append(data->at(i)->value * hamming.at(i));
  }
  int nfft = nextPow2(values.size());
  values.resize(nfft);

  QVector<std::complex<float>> resultValues = fft(values);

  auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
  double freqStep = fs / nfft;
  for (int i = 0; i < nfft / 2; i++) {
    if (dB)
      result->add(QCPGraphData(i * freqStep, 20 * log10(std::abs(resultValues.at(i)))));
    else
      result->add(QCPGraphData(i * freqStep, std::abs(resultValues.at(i))));
  }
  emit fftResult(result);
}

void SignalProcessing::process(int pos, QSharedPointer<QCPGraphDataContainer> data) {
  bool rangefound = false;
  double fs = 1 / ((data->keyRange(rangefound).upper - data->keyRange(rangefound).lower) / data->size());

  double max = data->valueRange(rangefound).upper;
  double min = data->valueRange(rangefound).lower;
  double amp = (max - min) / 2;

  double dc = 0;
  for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++) dc += it->value;
  dc /= data->size();

  double vrms = 0;
  for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++) vrms += (it->value * it->value);
  vrms /= data->size();
  vrms = sqrt(vrms);

  QVector<std::complex<float>> acValues;
  for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++) acValues.append(it->value - dc);

  /*int N = acValues.size();

  QVector<std::complex<float>> xCorr;
  xCorr.resize(2 * N - 1);
  for (int m = N - 1; m >= 0; m--) {
    float val = 0;
    for (int n = 0; n < N - m; n++) val += acValues.at(n + m) * acValues.at(n);
    xCorr[N + m - 1] = val / (N - m);
    xCorr[N - m - 1] = val / (N - m);
  }
  int nfft = nextPow2(xCorr.size());

  qDebug() << "x-corr took" << timer.elapsed() << "milliseconds - " << timer.nsecsElapsed() << "nanoseconds";
  timer.restart();

  xCorr.resize(nfft);
  QVector<std::complex<float>> xCorrFFT = fft(xCorr);

  qDebug() << "fft took" << timer.elapsed() << "milliseconds - " << timer.nsecsElapsed() << "nanoseconds";

  int maxindex = 0;
  float maxVal = 0;
  for (int i = 0; i < nfft / 2; i++) {
    float value = std::abs(xCorrFFT.at(i));
    if (value > maxVal) {
      maxVal = value;
      maxindex = i;
    }
  }*/

  int nfft = nextPow2(acValues.size());

  acValues.resize(nfft);
  QVector<std::complex<float>> acSigFFT = fft(acValues);

  int maxindex = 0;
  float maxVal = 0;
  for (int i = 0; i < nfft / 2; i++) {
    float value = std::abs(acSigFFT.at(i));
    if (value > maxVal) {
      maxVal = value;
      maxindex = i;
    }
  }

  float freq = maxindex * fs / nfft;

  emit result(pos, 1 / freq, freq, amp, (max - min), min, max, vrms, dc);
}
