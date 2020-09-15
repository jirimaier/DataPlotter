#ifndef MYPLOT_H
#define MYPLOT_H

#include "enums.h"
#include "qcustomplot.h"
#include <QWidget>

class myPlot : public QCustomPlot {
  Q_OBJECT
public:
  explicit myPlot(QWidget *parent = nullptr);
  void updateCursors(double x1, double x2, double y1, double y2);

private:
  double curX1 = 0, curX2 = 0, curY1 = 0, curY2 = 0;
  QCPItemLine *cursorX1, *cursorX2, *cursorY1, *cursorY2;
  double rollingRange = 100;
  int zoom = 1000;
  int plottingStatus = PLOT_STATUS_RUN;
  int plottingRangeType = PLOT_RANGE_FIXED;
  QScrollBar *horizontalPos;
  void addPoint(QByteArray point);
  QTimer *timer = new QTimer(this);
  void replaceData();
  void clearGraphs();
  double minTime();
  double maxTime();
  double verticalRange = 10;
  int verticalCenter = 0;
  void initCursors();

public slots:
  void update();
  void setVerticalRange(double value) { verticalRange = value; }
  void setVerticalCenter(int value) { verticalCenter = value; }
  void setRangeType(int type);
  void pauseClicked();
  void singleTrigerClicked();
  void setRefreshPeriod(int period) { timer->setInterval(period); }
  void setVerticalDiv(double value);
  void setHorizontalDiv(double value);
  void setShowVerticalValues(bool enabled);
  void setShowHorizontalValues(bool enabled);
  void setRollingLength(double value) { rollingRange = value; }
  void setZoom(int value) { zoom = value; }
  void setCurXen(bool en);
  void setCurYen(bool en);
  void newDataString(QByteArray data);
  void newDataBin(QByteArray data, int bits, double valMin, double valMax, double timeStep, int numCh, int firstCh, bool continuous);
  QString chToCSV(int ch);

signals:
  void showPlotStatus(int code);
  void setHDivLimits(double hRange);
  void setVDivLimits(double vRange);
  void setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminull, double xmaxfull, double yminfull, double ymaxfull);
};

#endif // MYPLOT_H
