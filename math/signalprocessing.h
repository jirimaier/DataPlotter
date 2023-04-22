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

#ifndef SIGNALPROCESSING_H
#define SIGNALPROCESSING_H

#include <math.h>

#include <QDebug>
#include <QObject>
#include <complex>
#include <QElapsedTimer>

#include "global.h"
#include "plots/qcustomplot.h"

class SignalProcessing : public QObject {
  Q_OBJECT
 public:
  explicit SignalProcessing(QObject* parent = nullptr);

 private:
  void resizeHamming(int length);
  void resizeHann(int length);
  void resizeBlackman(int length);
  void calculateLookupTable(int NxK);
  QVector<double> hamming, hann, blackman;
  QVector<std::complex<double>> fft(QVector<std::complex<double>> signal);
  inline double getStrongestFreq(QSharedPointer<QCPGraphDataContainer> data, double dc, double fs);
  inline QPair<double, double> getRiseFall(QSharedPointer<QCPGraphDataContainer> data);

 public slots:
  void getFFTPlot(QSharedPointer<QCPGraphDataContainer> data, FFTType::enumFFTType type, FFTWindow::enumFFTWindow window, bool removeDC, int segmentCount, bool twosided, bool zerocenter, int minNFFT);
  QVector<std::complex<double> > calculateSpectrum(QVector<std::complex<double>> data, FFTWindow::enumFFTWindow window, int minNFFT);
  void process(QSharedPointer<QCPGraphDataContainer> data);

 signals:
  void fftResult(QSharedPointer<QCPGraphDataContainer> data);
  void result(double period, double freq, double amp, double min, double max, double vrms, double dc, double fs, double rise, double fall, int samples);
};

#endif // SIGNALPROCESSING_H
