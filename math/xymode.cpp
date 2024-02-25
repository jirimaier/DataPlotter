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

#include "xymode.h"

XYMode::XYMode(QObject* parent): QObject(parent) {

}

void XYMode::calculateXY(QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2, bool removeDC) {
  if (in1->size() != in2->size()) {                                                  // Mají kanály stejný počet vzorků? Když ne, budou ustřihnut začátk nebo konec.
    double mint = MAX(in1->at(0)->key, in2->at(0)->key);                             // Nejnižší společný čas
    double maxt = MIN(in1->at(in1->size() - 1)->key, in2->at(in2->size() - 1)->key); // Nejvyšší společný čas
    in1->removeBefore(mint);
    in2->removeBefore(mint);
    in1->removeAfter(maxt);
    in2->removeAfter(maxt);
  }

  double dc1 = 0, dc2 = 0;

  if (removeDC) {
    for (int i = 0; i < in1->size(); i++) {
      dc1 += in1->at(i)->value;
      dc2 += in2->at(i)->value;
    }
    dc1 /= in1->size();
    dc2 /= in2->size();
  }

  auto result = QSharedPointer<QCPCurveDataContainer>(new QCPCurveDataContainer());
  for (int i = 0; i < in1->size(); i++) {
    result->add(QCPCurveData(in1->at(i)->key, in1->at(i)->value - dc1, in2->at(i)->value - dc2));
  }
  emit sendResultXY(result);
}
