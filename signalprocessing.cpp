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

void SignalProcessing::resizeHamming(int length) {
  if (hamming.size() != length) {
    hamming.resize(length);
    for (int n = 0; n < length; n++)
      hamming[n] = 0.54 - 0.46 * cos(2 * M_PI * n / length);
  }
}

void SignalProcessing::resizeHann(int length) {
  if (hann.size() != length) {
    hann.resize(length);
    for (int n = 0; n < length; n++)
      hann[n] = 0.5 * (1 - cos(2 * M_PI * n / length));
  }
}

void SignalProcessing::resizeBlackman(int length) {
  if (blackman.size() != length) {
    blackman.resize(length);
    for (int n = 0; n < length; n++)
      blackman[n] = 0.42 - 0.5 * cos(2 * M_PI * n / (length - 1)) + 0.08 * cos(4 * M_PI * n / (length - 1));
  }
}

QVector<std::complex<float>> SignalProcessing::fft(QVector<std::complex<float>> x) {
  int N = x.size();

  if (N == 1)
    return x; // Konec rekurze

  // exp(2*Pi*i/N)
  std::complex<float> Wn = std::exp(std::complex<float>(0, 2.0 * M_PI / (float)N));

  QVector<std::complex<float>> Pe, Po; // Sudé a liché koeficienty
  for (int n = 1; n < N; n += 2) {
    Po.append(x.at(n - 1)); // Liché
    Pe.append(x.at(n));     // Sudé
  }

  QVector<std::complex<float>> Xe = fft(Pe), Xo = fft(Po); // Rekurze

  QVector<std::complex<float>> X;
  X.resize(N);
  for (int k = 0; k < N / 2; k++) {
    std::complex<float> Wi = std::pow(Wn, k);
    X[k] = Xe[k] + Wi * Xo[k];
    X[k + N / 2] = Xe[k] - Wi * Xo[k];
  }
  return X;
}

int SignalProcessing::nextPow2(int number) {
  for (int i = 1;; i++)
    if (pow(2, i) >= number)
      return (pow(2, i));
}

void SignalProcessing::calculateSpectrum(QSharedPointer<QCPGraphDataContainer> data, FFTType::enumerator type, FFTWindow::enumerator window) {
  if (type != FFTType::pwelch) {
    float fs = data->size() / (data->at(data->size() - 1)->key - data->at(0)->key);

    QVector<std::complex<float>> values;
    if (window == FFTWindow::rectangular)
      for (int i = 0; i < data->size(); i++)
        values.append(data->at(i)->value);
    else if (window == FFTWindow::hamming) {
      resizeHamming(data->size());
      for (int i = 0; i < data->size(); i++)
        values.append(data->at(i)->value * hamming.at(i));
    } else if (window == FFTWindow::hann) {
      resizeHann(data->size());
      for (int i = 0; i < data->size(); i++)
        values.append(data->at(i)->value * hann.at(i));
    } else if (window == FFTWindow::blackman) {
      resizeBlackman(data->size());
      for (int i = 0; i < data->size(); i++)
        values.append(data->at(i)->value * blackman.at(i));
    }
    int nfft = nextPow2(values.size());
    if (nfft < 1024)
      nfft = 1024;
    values.resize(nfft);

    QVector<std::complex<float>> resultValues = fft(values);

    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
    double freqStep = fs / nfft;
    for (int i = 0; i <= nfft / 2.0; i++) {
      if (type == FFTType::periodogram) {
        // Výpočet |x|^2 jako x * komplexně sdružené x;
        float absSquared = (resultValues.at(i) * std::complex<float>(resultValues.at(i).real(), -resultValues.at(i).imag())).real();
        // float absSquared = abs(resultValues.at(i)) * abs(resultValues.at(i));
        result->add(QCPGraphData(i * freqStep, 10 * log10(absSquared / nfft)));
      } else
        result->add(QCPGraphData(i * freqStep, std::abs(resultValues.at(i))));
    }
    emit fftResult(result);
  }
}

void SignalProcessing::process(QSharedPointer<QCPGraphDataContainer> data) {
  bool rangefound = false; // Nevyužité, ale je potřeba do funkcí co hledají max/min
  float max = data->valueRange(rangefound).upper;
  float min = data->valueRange(rangefound).lower;

  // Stejnosměrná složka
  float dc = 0;
  for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++)
    dc += it->value;
  dc /= data->size();

  // Efektivní hodnota
  float vrms = 0;
  for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++)
    vrms += (it->value * it->value);
  vrms /= data->size();
  vrms = sqrt(vrms);

  float freq = getStrongestFreq(data, dc);

  emit result(1 / freq, freq, (max - min) / 2, (max - min), min, max, vrms, dc);
}

float SignalProcessing::getStrongestFreq(QSharedPointer<QCPGraphDataContainer> data, float dc) {
  // Vzorkovací frekvence (převrácený interval mezi vzorky, předpokládá se konstantní)
  float fs = data->size() / (data->at(data->size() - 1)->key - data->at(0)->key);

  //    // Výpočet frekvence - pro krátký signál (do 4096 vzorků) udělám autokorelaci
  //    // a z ní FFT; pro delší signál jen FFT přímo ze signálu (rychlé, ale méně přesné)
  //    if (data->size() <= 4096) {
  //      QVector<float> acValues;
  //      for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++)
  //        acValues.append(it->value - dc);

  //      int N = acValues.size();

  //      QVector<std::complex<float>> xCorr;
  //      xCorr.resize(2 * N - 1);
  //      for (int m = N - 1; m >= 0; m--) {
  //        float val = 0;
  //        for (int n = 0; n < N - m; n++)
  //          val += acValues.at(n + m) * acValues.at(n);
  //        float unBiassedValue = val / (N - m);
  //        xCorr[N + m - 1] = unBiassedValue;
  //        xCorr[N - m - 1] = unBiassedValue;
  //      }

  //      int nfft = 4096;         // Pro FFT doplním nulami na 4096
  //      if (xCorr.size() > nfft) // xCorr je delší než původní signál, může být delší než 4096, tedy doplním na nejbližší vyšší pow2.
  //        nfft = nextPow2(nfft);

  //      resizeHamming(xCorr.size());

  //      for (int i = 0; i <= xCorr.size(); i++)
  //        xCorr[i] *= hamming[i];

  //      xCorr.resize(nfft);
  //      QVector<std::complex<float>> xCorrFFT = fft(xCorr);

  //      int maxindex = 0;
  //      float maxVal = 0;
  //      for (int i = 0; i < nfft / 2; i++) {
  //        float value = std::abs(xCorrFFT.at(i));
  //        if (value > maxVal) {
  //          maxVal = value;
  //          maxindex = i;
  //        }
  //      }
  //      freq = maxindex * fs / nfft;
  //    } else {
  QVector<std::complex<float>> acValues;
  for (int i = 0; i < data->size(); i++)
    acValues.append((data->at(i)->value - dc));

  int nfft = nextPow2(acValues.size());

  if (nfft < 4096)
    nfft = 4096;

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
  return (maxindex * fs / nfft);
}
