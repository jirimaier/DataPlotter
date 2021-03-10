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

#include "mymodifiedqcptracer.h"

void MyModifiedQCPTracer::updatePosition() {
  // Verze pro graf
  if (mGraph) {
    if (mParentPlot->hasPlottable(mGraph)) {
      if (!mGraph->data()->isEmpty()) {
        QCPGraphDataContainer::const_iterator nearest;
        double nearestDist = Q_INFINITY;
        double dist, difx, dify;
        // Má smysl zjišťovat jen pro zobrazený rozsah, ne pro celý průběh.
        auto range = mGraph->keyAxis()->range();
        for (auto it = mGraph->data()->findBegin(range.lower); it != mGraph->data()->findEnd(range.upper); it++) {
          // Na rozdíl od původního toto porovnává xy souřadnice, ne jen y, funguje lépe zejména na strmé čáře
          // Je potřeba porovnávat vzdálenosti v pixelech, ne souřadnicích na grafu - osy mohou mít různé měřítko
          difx = parentPlot()->xAxis->coordToPixel(it->key) - mPoint.x();
          dify = verticalAxis->coordToPixel(it->value) - mPoint.y();
          dist = difx * difx + dify * dify;
          if (dist < nearestDist) {
            nearestDist = dist;
            nearest = it;
          }
        }
        position->setCoords(nearest->key, nearest->value);
        posIndex = nearest - mGraph->data()->begin();
      }
    }
  }

  // Verze pro křivku (XY graf)
  else if (mCurve) {
    if (mParentPlot->hasPlottable(mCurve)) {
      if (!mCurve->data()->isEmpty()) {
        QCPCurveDataContainer::iterator nearest;
        double nearestDist = Q_INFINITY;
        double dist, difx, dify;
        for (auto it = mCurve->data()->begin(); it != mCurve->data()->end(); it++) {
          difx = parentPlot()->xAxis->coordToPixel(it->key) - mPoint.x();
          dify = verticalAxis->coordToPixel(it->value) - mPoint.y();
          dist = difx * difx + dify * dify;
          if (dist < nearestDist) {
            nearestDist = dist;
            nearest = it;
          }
        }
        position->setCoords(nearest->key, nearest->value);
        posIndex = nearest - mCurve->data()->begin();
      }
    }
  }
}
