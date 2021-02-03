#ifndef MYMODIFIEDQCPTRACER_H
#define MYMODIFIEDQCPTRACER_H

#include "qcustomplot.h"

class MyModifiedQCPTracer : public QCPItemTracer {
  Q_OBJECT
public:
  explicit MyModifiedQCPTracer(QCustomPlot *parentPlot) : QCPItemTracer(parentPlot) { verticalAxis = parentPlot->yAxis; }

  void setCurve(QCPCurve *curve) { mCurve = curve; }
  void setYAxis(QCPAxis *vAxis) { verticalAxis = vAxis; }
  void setPoint(QPoint point) {
    mPoint = point;
    setGraphKey(point.x());
  }

  void updatePosition();

  int sampleNumber() { return posIndex; }

  bool use2DPositionForGraphToo = false;

protected:
  QCPCurve *mCurve;
  QPoint mPoint;
  int posIndex = 0;
  QCPAxis *verticalAxis;
};

#endif // MYMODIFIEDQCPTRACER_H
