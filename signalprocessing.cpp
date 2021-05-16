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

SignalProcessing::SignalProcessing(QObject* parent) : QObject(parent) {

}

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
      blackman[n] = 0.42 - 0.5 * cos(2 * M_PI * n / length) + 0.08 * cos(4 * M_PI * n / length);
  }
}

QVector<std::complex<double>> SignalProcessing::fft(QVector<std::complex<double>> x) {
  int N = x.size(); // Velikost x musí být mocnina 2.

  if (N == 1)
    return x; // Konec rekurze

  QVector<std::complex<double>> Pe, Po; // Sudé a liché koeficienty
  for (int n = 1; n < N; n += 2) {
    Pe.append(x.at(n - 1)); // Sudé
    Po.append(x.at(n));     // Liché
  }

  QVector<std::complex<double>> Xe = fft(Pe), Xo = fft(Po); // Rekurze

  QVector<std::complex<double>> X;
  X.resize(N);
  for (int k = 0; k < N / 2; k++) {

    // exp(i*2*Pi*k/N)
    double arg = M_PI * 2 * k / N;
    std::complex<double> WNk(cos(arg), sin(arg));

    X[k] = Xe[k] + WNk * Xo[k];
    X[k + N / 2] = Xe[k] - WNk * Xo[k];
  }
  return X;
}

void SignalProcessing::getFFTPlot(QSharedPointer<QCPGraphDataContainer> data, FFTType::enumFFTType type, FFTWindow::enumFFTWindow window, bool removeDC, int segmentCount, bool twosided, bool zerocenter, int minNFFT) {
  if (removeDC) {
    // Stejnosměrná složka
    double dc = 0;
    for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++)
      dc += it->value;
    dc /= data->size();

    // Odstranění stejnosměrné složky
    for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++)
      it->value -= dc;
  }

  double fs = data->size() / (data->at(data->size() - 1)->key - data->at(0)->key);

  if (type == FFTType::spectrum || type == FFTType::periodogram) {
    QVector<std::complex<double>> values;
    for (int i = 0; i < data->size(); i++)
      values.append(std::complex<double>(data->at(i)->value, 0));

    double normalization = data->size();
    if (window == FFTWindow::hamming)
      normalization *= 0.54;
    else if (window == FFTWindow::hann)
      normalization *= 0.5;
    else if (window == FFTWindow::blackman)
      normalization *= 0.42;
    double normalizationSquared = normalization * normalization;

    QVector<std::complex<double>> resultValues = calculateSpectrum(values, window, minNFFT);
    int nfft = resultValues.size();

    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
    double freqStep = fs / nfft;
    for (int i = 0; (twosided ? (i < nfft) : (i <= nfft / 2)); i++) {
      double freq = i * freqStep;
      if (zerocenter && i > nfft / 2)
        freq -= nfft * freqStep;
      if (type == FFTType::periodogram) {
        // Výpočet |x|^2 jako x * komplexně sdružené x;
        double absSquared = (resultValues.at(i) * std::complex<double>(resultValues.at(i).real(), -resultValues.at(i).imag())).real();
        // double absSquared = abs(resultValues.at(i)) * abs(resultValues.at(i));
        result->add(QCPGraphData(freq, 10 * log10(absSquared / normalizationSquared)));
      } else
        result->add(QCPGraphData(freq, std::abs(resultValues.at(i)) / normalization));
    }
    emit fftResult(result);
  }

  else if (type == FFTType::pwelch) {
    // Výpočet periodogramu Welchovou metodou

    //Rozdělení na segmenty s 50% překryvem
    // Kolik půl-segmentů se vejde?
    int halfSegmentLength = data->size() / segmentCount;
    // Pokud je počet půlsegmentů sudý, poslední překryvný se nevejde, bude o jeden méně, než se chtělo
    // |___ ___ ___ ___ ___ _|
    // |  ___ ___ ___ ___ ___|
    // V horní řadě je 5 a půl segmentů (lichý počet půlsegmentů), do spodní se vejde taky 5
    //
    // |___ ___ ___ ___ ___|
    // |  ___ ___ ___ ___   |
    // V horní řadě je 5 celých segmentů (sudý počet půlsegmentů), do spodní se vejde je 4
    if ((data->size() / halfSegmentLength) % 2 == 0)
      segmentCount--;

    // Rozdělení na segmenty
    QVector<QVector<std::complex<double>>> segments;
    segments.resize(segmentCount);
    for (int i = 0; i < segments.size(); i++) {
      for (int j = i * halfSegmentLength; j < (i + 2)*halfSegmentLength; j++)
        segments[i].append(std::complex<double>(data->at(j)->value, 0));
    }

    double normalization = 2 * halfSegmentLength;
    if (window == FFTWindow::hamming)
      normalization *= 0.54;
    else if (window == FFTWindow::hann)
      normalization *= 0.5;
    else if (window == FFTWindow::blackman)
      normalization *= 0.42;
    double normalizationSquared = normalization * normalization;

    // Výpočet spektra pro jednotlivé segmenty
    // Funkce calculateSpectrum použije okno a doplní nulami na mocninu dvou
    for (int i = 0; i < segments.size(); i++) {
      segments[i] = calculateSpectrum(segments.at(i), window, minNFFT);
    }

    int nfft = segments.at(0).length();

    // Výpočet periodogramů a zprůměrování
    auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
    double freqStep = fs / nfft;
    for (int i = 0; (twosided ? (i < nfft) : (i <= nfft / 2)); i++) {
      double value = 0;
      double freq = i * freqStep;
      if (zerocenter && i > nfft / 2)
        freq -= nfft * freqStep;
      for (int j = 0; j < segments.size(); j++)
        // Přičte k value |x|^2, Výpočet |x|^2 jako x * komplexně sdružené x;
        value += (segments.at(j).at(i) * std::complex<double>(segments.at(j).at(i).real(), -segments.at(j).at(i).imag())).real();
      //Přidá do výsledku bod - součet hodnot ze segmentů dělený nfft a počtem segmentů. V dB.
      result->add(QCPGraphData(freq, 10 * log10(value / normalizationSquared / segments.size())));
    }
    emit fftResult(result);
  }
}

QVector<std::complex<double>> SignalProcessing::calculateSpectrum(QVector<std::complex<double>> data, FFTWindow::enumFFTWindow window, int minNFFT) {
  if (window == FFTWindow::hamming) {
    resizeHamming(data.size());
    for (int i = 0; i < data.size(); i++)
      data[i] *= hamming.at(i);
  } else if (window == FFTWindow::hann) {
    resizeHann(data.size());
    for (int i = 0; i < data.size(); i++)
      data[i] *= hann.at(i);
  } else if (window == FFTWindow::blackman) {
    resizeBlackman(data.size());
    for (int i = 0; i < data.size(); i++)
      data[i] *= blackman.at(i);
  }

  int nfft = nextPow2(data.size());
  if (nfft < minNFFT)
    nfft = minNFFT;
  data.resize(nfft);

  return fft(data);
}

void SignalProcessing::process(QSharedPointer<QCPGraphDataContainer> data) {
  bool rangefound = false; // Nevyužité, ale je potřeba do funkcí co hledají max/min
  double max = data->valueRange(rangefound).upper;
  double min = data->valueRange(rangefound).lower;

  double fs = (data->size() - 1) / (data->at(data->size() - 1)->key - data->at(0)->key);

  // Stejnosměrná složka
  double dc = 0;
  for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++)
    dc += it->value;
  dc /= data->size();

  // Efektivní hodnota
  double vrms = 0;
  for (QCPGraphDataContainer::iterator it = data->begin(); it != data->end(); it++)
    vrms += (it->value * it->value);
  vrms /= data->size();
  vrms = sqrt(vrms);

  double freq = getStrongestFreq(data, dc, fs);

  double period = 1.0 / freq;

  int samples = data->size();

  // Od teď se počítá jen v posledními dvěma periodami !!!
  data->removeBefore(data->at(data->size() - 1)->key - 2.0 * period);

  auto risefall = getRiseFall(data);

  emit result(period, freq, (max - min), min, max, vrms, dc, fs, risefall.first, risefall.second, samples);
}

double SignalProcessing::getStrongestFreq(QSharedPointer<QCPGraphDataContainer> data, double dc, double fs) {


  // Prostě udělám FFT (po odečtení DC) a najdu globální maximum
  QVector<std::complex<double>> acValues;
  for (int i = 0; i < data->size(); i++)
    acValues.append((data->at(i)->value - dc));

  int nfft = acValues.size() * 5;

  nfft = nextPow2(nfft);

  acValues.resize(nfft);
  QVector<std::complex<double>> acSigFFT = fft(acValues);

  int maxindex = 0;
  double maxVal = 0;
  for (int i = 0; i <= nfft / 2; i++) {
    double value = std::abs(acSigFFT.at(i));
    if (value > maxVal) {
      maxVal = value;
      maxindex = i;
    }
  }

  double freq = maxindex * fs / nfft;

  if (freq < fs * (1 + sqrt(4 * nfft + 1)) / (2 * nfft)) {
    int aproxIndex = fs / freq;
    int aproxMin = (aproxIndex * 90) / 100;
    int aproxMax = (aproxIndex * 110) / 100;
    acValues.resize(data->size()); //Odstranění doplněných nul

    int N = acValues.size();

    if (aproxMin < 0)
      aproxMin = 0;
    if (aproxMax >= N)
      aproxMax = N - 1;

    double highestValue = -Q_INFINITY;
    int highestIndex = 0;

    for (int k = aproxMin; k <= aproxMax; k++) {
      double val = 0;
      for (int n = 0; n < N - k; n++)
        val += acValues.at(n + k).real() * acValues.at(n).real();
      if (val > highestValue) {
        highestIndex = k;
        highestValue = val;
      }
    }
    freq = fs / highestIndex;
  }
  return (freq);
}

QPair<double, double> SignalProcessing::getRiseFall(QSharedPointer<QCPGraphDataContainer> data) {
  auto risefall = QPair<double, double>(Q_QNAN, Q_QNAN);

  // Zde se počítá je s posledními dvěma periodami (aby se zamezil vliv náhodných špiček na min/max)
  bool rangefound = false; // Nevyužité, ale je potřeba do funkcí co hledají max/min
  double max = data->valueRange(rangefound).upper;
  double min = data->valueRange(rangefound).lower;

  double top = min + 0.9 * (max - min);    // 90 %
  double bottom = min + 0.1 * (max - min); // 10 %

  int riseEnd = -1;
  int fallEnd = -1;

  // postupuje se od konce - platí poslední vzestup/sestup
  // vzestup
  for (int i = data->size() - 1; i >= 0; i--) {
    if (data->at(i)->value >= top)
      riseEnd = i; // Je nad 90 %
    else if (riseEnd != -1) {
      // Konec už mám, tohle může být začátek, pokud je pod 10 %
      if (data->at(i)->value <= bottom) {
        // Je to začátek (první před koncem co je pod 10 %)
        // Aby to fungovalo i pro málo vzorků, tak to podle začátku a konce
        // nahradím přímkou a spočítám za jak dlouho naroste z min na max
        QCPGraphData end = *data->at(riseEnd);
        QCPGraphData begin = *data->at(i);
        double slope = (end.value - begin.value) / (end.key - begin.key);
        risefall.first = (max - min) / slope * 0.8;
        // Risetime je definován jako čas mezi 10 % a 90 %, toto je od min do max, tedy 0 - 100 %,
        // tedy hodnotu násobím 0.8, aby to odpovídalo.
        break;
      }
    }
  }

  // Falltime, analogicky k předchozímu...
  for (int i = data->size() - 1; i >= 0; i--) {
    if (data->at(i)->value <= bottom)
      fallEnd = i;
    else if (fallEnd != -1) {
      if (data->at(i)->value >= top) {
        QCPGraphData end = *data->at(fallEnd);
        QCPGraphData begin = *data->at(i);
        double slope = (end.value - begin.value) / (end.key - begin.key);
        risefall.second = (min - max) / slope * 0.8; // min a max je prohozeno, aby výsledek nebyl záporný
        break;
      }
    }
  }
  return risefall;
}

