#ifndef MYPLOT_H
#define MYPLOT_H

#include "enums_defines_constants.h"
#include "qcustomplot.h"
#include <QWidget>

class MyPlot : public QCustomPlot {
  Q_OBJECT
public:
  MyPlot(QWidget *parent = nullptr);
  void updateCursors(double *cursorPositions);

protected:
  bool iHaveCursors = false;
  QVector<QCPItemLine *> cursors;
  void initcursors();
  void setMouseControlls(bool enabled);

public slots:
  void setCursorsAccess(bool allowed);

private slots:
  void onXRangeChanged(QCPRange range);
  void onYRangeChanged(QCPRange range);

signals:
  void setCursorBounds(PlotFrame_t frame);
};
#endif // MYPLOT_H
