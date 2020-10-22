#ifndef MYXYPLOT_H
#define MYXYPLOT_H

#include "enums_defines_constants.h"
#include "plotdata.h"
#include "qcustomplot.h"
#include <QTimer>
#include <QWidget>

class MyXYPlot : public QCustomPlot {
  Q_OBJECT
public:
  MyXYPlot(QWidget *parent = nullptr);
  void newData(QVector<double> *x, QVector<double> *y);
  void updateCursors(double *cursorPositions);

private:
  QCPCurve *graphXY;
  bool iHaveCursors = false;
  QVector<QCPItemLine *> cursors;
  void initcursors();

public slots:
  void setCursorsAccess(bool allowed);

signals:
  void setCursorBounds(PlotFrame_t frame);
};

#endif // MYXYPLOT_H
