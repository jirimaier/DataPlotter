#ifndef AXISTAG_H
#define AXISTAG_H

// Kód v tomto souboru je převzatý z článku
// "Creating dynamic axis tags using items" v návodu k QCustomPlot

// https://www.qcustomplot.com/index.php/tutorials/specialcases/axistags

#include "qcustomplot.h"
#include <QObject>

class AxisTag : public QObject {
  Q_OBJECT
public:
  explicit AxisTag(QCPAxis *parentAxis);
  virtual ~AxisTag();

  // setters:
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setText(const QString &text);

  // getters:
  QPen pen() const { return mLabel->pen(); }
  QBrush brush() const { return mLabel->brush(); }
  QString text() const { return mLabel->text(); }

  // other methods:
  void updatePosition(double value);

protected:
  QCPAxis *mAxis;
  QPointer<QCPItemTracer> mDummyTracer;
  QPointer<QCPItemLine> mArrow;
  QPointer<QCPItemText> mLabel;
};

#endif // AXISTAG_H
