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

SignalProcessing::SignalProcessing(QObject *parent) : QObject(parent) {}

void SignalProcessing::calculateHamming(int length) {
  if (hamming.size() != length) {
    hamming.resize(length);
    for (int n = 0; n < length; n++) hamming[n] = 0.54 - 0.46 * cos(2 * M_PI * n / length);
  }
}

void SignalProcessing::calculateHann(int length) {
  if (hann.size() != length) {
    hann.resize(length);
    for (int n = 0; n < length; n++) hann[n] = 0.5 * (1 - cos(2 * M_PI * n / length));
  }
}

void SignalProcessing::calculateBlackman(int length) {
  if (blackman.size() != length) {
    blackman.resize(length);
    for (int n = 0; n < length; n++) blackman[n] = 0.42 - 0.5 * cos(2 * M_PI * n / (length - 1)) + 0.08 * cos(4 * M_PI * n / (length - 1));
  }
}

QVector<std::complex<float>> SignalProcessing::fft(QVector<std::complex<float>> signal) {
  int nfft = signal.size();
  if (nfft == 1) return signal;
  std::complex<float> W = std::exp(std::complex<float>(0, 2.0 * M_PI / (float)nfft));  // exp(2*Pi*i/n)

  QVector<std::complex<float>> Pe, Po;  // Sudé a liché koeficienty
  for (int i = 1; i <= nfft; i++) {
    if (i % 2)  // Liché
      Po.append(signal.at(i - 1));
    else  // Sudé
      Pe.append(signal.at(i - 1));
  }

  QVector<std::complex<float>> ye = fft(Pe), yo = fft(Po);  // Rekurze

  QVector<std::complex<float>> result;
  result.resize(nfft);
  for (int i = 0; i < nfft / 2; i++) {
    std::complex<float> Wi = std::pow(W, i);
    result[i] = ye[i] + Wi * yo[i];
    result[i + nfft / 2] = ye[i] - Wi * yo[i];
  }
  return result;
}

int SignalProcessing::nextPow2(int number) {
  for (int i = 1;; i++)
    if (pow(2, i) >= number) return (pow(2, i));
}

void SignalProcessing::plotFFT(QSharedPointer<QCPGraphDataContainer> data, bool dB, FFTWindow::enumerator window) {
  float fs = 1.0 / (data->at(1)->key - data->at(0)->key);

  QVector<std::complex<float>> values;
  if (window == FFTWindow::rectangular)
    for (int i = 0; i < data->size(); i++) values.append(data->at(i)->value);
  else if (window == FFTWindow::hamming) {
    calculateHamming(data->size());
    for (int i = 0; i < data->size(); i++) values.append(data->at(i)->value * hamming.at(i));
  } else if (window == FFTWindow::hann) {
    calculateHann(data->size());
    for (int i = 0; i < data->size(); i++) values.append(data->at(i)->value * hann.at(i));
  } else if (window == FFTWindow::blackman) {
    calculateBlackman(data->size());
    for (int i = 0; i < data->size(); i++) values.append(data->at(i)->value * blackman.at(i));
  }
  int nfft = nextPow2(values.size());
  if (nfft < 4096) nfft = 4096;
  values.resize(nfft);

  QVector<std::complex<float>> resultValues = fft(values);

  auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
  double freqStep = fs / nfft;
  for (int i = 0; i <= nfft / 2.0; i++) {
    if (dB)
      result->add(QCPGraphData(i * freqStep, 20 * log10(std::abs(resultValues.at(i)))));
    else
      result->add(QCPGraphData(i * freqStep, std::abs(resultValues.at(i))));
  }
  emit fftResult(result);
}

void SignalProcessing::process(QSharedPointer<QCPGraphDataContainer> data) {
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

  float freq;

  // Výpočet frekvence - pro krátký signál (do 4096 vzorků) udělá autokorelaci
  // a z ní FFT, pro délší signál jen FFT přímo ze signálu (rychlé, ale méně přesné)
  if (data->size() <= 4096) {
    QVector<float> acValues;
    for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++) acValues.append(it->value - dc);

    int N = acValues.size();

    QVector<std::complex<float>> xCorr;
    xCorr.resize(2 * N - 1);
    for (int m = N - 1; m >= 0; m--) {
      float val = 0;
      for (int n = 0; n < N - m; n++) val += acValues.at(n + m) * acValues.at(n);
      float unBiassedValue = val / (N - m);
      xCorr[N + m - 1] = unBiassedValue;
      xCorr[N - m - 1] = unBiassedValue;
    }

    int nfft = 4096;

    calculateHamming(xCorr.size());

    for (int i = 0; i <= xCorr.size(); i++) xCorr[i] *= hamming[i];

    xCorr.resize(nfft);
    QVector<std::complex<float>> xCorrFFT = fft(xCorr);

    int maxindex = 0;
    float maxVal = 0;
    for (int i = 0; i < nfft / 2; i++) {
      float value = std::abs(xCorrFFT.at(i));
      if (value > maxVal) {
        maxVal = value;
        maxindex = i;
      }
    }
    freq = maxindex * fs / nfft;
  } else {
    calculateHamming(data->size());
    QVector<std::complex<float>> acValues;
    for (int i = 0; i < data->size(); i++) acValues.append((data->at(i)->value - dc) * hamming.at(i));

    int nfft = nextPow2(acValues.size());

    if (nfft < 4096) nfft = 4096;

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
    freq = maxindex * fs / nfft;
  }
  emit result(1 / freq, freq, amp, (max - min), min, max, vrms, dc);
}
