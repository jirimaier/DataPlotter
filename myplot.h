#ifndef MYPLOT_H
#define MYPLOT_H

#include "enums_defines_constants.h"
#include "qcustomplot.h"
#include <QWidget>

class MyPlot : public QCustomPlot {
  Q_OBJECT
public:
  explicit MyPlot(QWidget *parent = nullptr);
  void updateCursors(double *cursorPositions);
  double getVDiv() { return fixedTickerY->tickStep(); }
  double getHDiv() { return fixedTickerX->tickStep(); }

protected:
  QSharedPointer<QCPAxisTickerTime> timeTickerX, longTimeTickerX;
  QSharedPointer<QCPAxisTickerFixed> fixedTickerX, fixedTickerY;
  bool iHaveCursors = false;
  QVector<QCPItemLine *> cursors;
  void initcursors();
  void setMouseControlls(bool enabled);

public slots:
  void setVerticalDiv(double value);
  void setHorizontalDiv(double value);
  void setShowVerticalValues(bool enabled);
  void setShowHorizontalValues(int type);
  void setCursorsAccess(bool allowed);
  void setXTitle(QString title) { this->xAxis->setLabel(title); }
  void setYTitle(QString title) { this->yAxis->setLabel(title); }

private slots:
  void onXRangeChanged(QCPRange range);
  void onYRangeChanged(QCPRange range);

signals:
  void setCursorBounds(PlotFrame_t frame);
};
#endif // MYPLOT_H
