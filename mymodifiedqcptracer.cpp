#include "mymodifiedqcptracer.h"

void MyModifiedQCPTracer::updatePosition() {
  // Moje verze pro graf
  if (mGraph && use2DPositionForGraphToo) {
    if (mParentPlot->hasPlottable(mGraph)) {
      if (!mGraph->data()->isEmpty()) {
        QCPGraphDataContainer::iterator nearest;
        double nearestDist = Q_INFINITY;
        double dist, difx, dify;
        for (QCPGraphDataContainer::iterator it = mGraph->data()->begin(); it != mGraph->data()->end(); it++) {
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
      } else
        qDebug() << Q_FUNC_INFO << "graph has no data";
    } else
      qDebug() << Q_FUNC_INFO << "graph not contained in QCustomPlot instance (anymore)";
  }

  // Původní verze pro graf
  else if (mGraph) {
    if (mParentPlot->hasPlottable(mGraph)) {
      if (mGraph->data()->size() > 1) {
        QCPGraphDataContainer::const_iterator first = mGraph->data()->constBegin();
        QCPGraphDataContainer::const_iterator last = mGraph->data()->constEnd() - 1;
        if (mGraphKey <= first->key)
          position->setCoords(first->key, first->value);
        else if (mGraphKey >= last->key)
          position->setCoords(last->key, last->value);
        else {
          QCPGraphDataContainer::const_iterator it = mGraph->data()->findBegin(mGraphKey);
          if (it != mGraph->data()->constEnd()) // mGraphKey is not exactly on last iterator,
                                                // but somewhere between iterators
          {
            QCPGraphDataContainer::const_iterator prevIt = it;
            ++it; // won't advance to constEnd because we handled that case
                  // (mGraphKey >= last->key) before
            if (mInterpolating) {
              // interpolate between iterators around mGraphKey:
              double slope = 0;
              if (!qFuzzyCompare((double)it->key, (double)prevIt->key))
                slope = (it->value - prevIt->value) / (it->key - prevIt->key);
              position->setCoords(mGraphKey, (mGraphKey - prevIt->key) * slope + prevIt->value);
            } else {
              // find iterator with key closest to mGraphKey:
              if (mGraphKey < (prevIt->key + it->key) * 0.5) {
                position->setCoords(prevIt->key, prevIt->value);
                posIndex = prevIt - mGraph->data()->begin();
              } else {
                position->setCoords(it->key, it->value);
                posIndex = it - mGraph->data()->begin();
              }
            }
          } else { // mGraphKey is exactly on last iterator (should actually be
                   // caught when comparing first/last keys, but this is a
                   // failsafe for fp uncertainty)
            position->setCoords(it->key, it->value);
            posIndex = it - mGraph->data()->begin();
          }
        }
      } else if (mGraph->data()->size() == 1) {
        QCPGraphDataContainer::const_iterator it = mGraph->data()->constBegin();
        position->setCoords(it->key, it->value);
      } else
        qDebug() << Q_FUNC_INFO << "graph has no data";
    } else
      qDebug() << Q_FUNC_INFO << "graph not contained in QCustomPlot instance (anymore)";
  }

  // Moje verze pro křivku (XY graf)
  else if (mCurve) {
    if (mParentPlot->hasPlottable(mCurve)) {
      if (!mCurve->data()->isEmpty()) {
        QCPCurveDataContainer::iterator nearest;
        double nearestDist = Q_INFINITY;
        double dist, difx, dify;
        for (QCPCurveDataContainer::iterator it = mCurve->data()->begin(); it != mCurve->data()->end(); it++) {
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
      } else
        qDebug() << Q_FUNC_INFO << "curve has no data";
    } else
      qDebug() << Q_FUNC_INFO << "curve not contained in QCustomPlot instance (anymore)";
  }
}
