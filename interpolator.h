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

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <QObject>

#include "enumsDefinesConstants.h"
#include "qcustomplot.h"

class Interpolator : public QObject {
  Q_OBJECT
 public:
  explicit Interpolator(QObject* parent = nullptr);

 private:
  /// Filtr pro filterování při interpolaci
  QVector<float> lowPassFIR;

  /// Konvoluce signálu x s odezvou filtru h
  QVector<float> filter(QVector<float> x, QVector<float> h);

 public slots:
  /// Interpoluje signál a výsledek odešle signálem interpolationResult, pokud data pochází z bufferu (přidávání po celých kanálech, jsou
  /// v grafu prepsány i původní vzorky, aby odpovídali průběhu z kterého je vypočtena interpolace. Pokud byla data vzata přímo z grafu,
  /// původní vzorky už v něm jsou a není potřeba je přepisovat.
  void interpolate(int chID, const QSharedPointer<QCPGraphDataContainer> data, QCPRange visibleRange, bool dataIsFromInterpolationBuffer);

 signals:
  /// Odešle interpolovaný průběh, pokud data z kterých byla interpolace počítána pochází z bufferu (přidávání po celých kanálech, jsou
  /// v grafu prepsány i původní vzorky, aby odpovídali průběhu z kterého je vypočtena interpolace. Pokud byla data vzata přímo z grafu,
  /// původní vzorky už v něm jsou a není potřeba je přepisovat.
  void interpolationResult(int chID, QSharedPointer<QCPGraphDataContainer> dataOriginal, QSharedPointer<QCPGraphDataContainer> dataInterpolated, bool dataIsFromInterpolationBuffer);

  /// Oznámí, že interpolace je hotová a tedy je možné zpracovat další průběh
  void finished(int chID);
};

#endif // INTERPOLATOR_H
