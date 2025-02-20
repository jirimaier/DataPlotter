//  Copyright (C) 2020-2024  Jiří Maier

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

#ifndef MYFFTPLOT_H
#define MYFFTPLOT_H

#include <QObject>

#include "myplot.h"

class MyFFTPlot : public MyPlot {
  Q_OBJECT
 public:
  explicit MyFFTPlot(QWidget* parent = nullptr);

  /// Exportuje oba kanály (nebo jeden, když nejsou oba využité)
  QByteArray exportCSV(char separator, char decimal, int precision);

  /// Vrátí rozsah vzorků v daném kanálu, které jsou ve viditelném rozsahu osy X
  QPair<unsigned int, unsigned int> getVisibleSamplesRange(int chID);

  /// Přizpůsobý barvu a popis, vrátí true, pokud se barva změnila
  bool setChSorce(int ch, int sourceChannel, QColor color);

  void setOutputPeakValue(bool newOutputPeakValue);

  private:
  void autoset();
  void setMouseCursorStyle(QMouseEvent* event);
  void updateTracerText(int index);
  int currentTracerIndex = -1;
  int chSourceChannel[2];
  QColor chSourceColor[2];
  QPair<QVector<double>, QVector<double> > getDataVector(int chID);
  bool holdmax[2] = {false, false};
  bool firstAutoset = true;
  bool outputPeakValue;

 public slots:
  /// Přidá data (přepíše stará)
  void newData(int chID, QSharedPointer<QCPGraphDataContainer> data);

  /// Vymaže kanál
  void clear(int chID);

  /// Vymaže celý graf
  void clear();

  /// Nastaví styl kanálu
  void setStyle(int chID, int style);

  void setHoldMax1(bool en) {holdmax[0] = en;}
  void setHoldMax2(bool en) {holdmax[1] = en;}

 private slots:
  void mouseMoved(QMouseEvent* event);
  void mousePressed(QMouseEvent* event);

 signals:
  void newPeakValues(int chID, double freq);
};

#endif // MYFFTPLOT_H
