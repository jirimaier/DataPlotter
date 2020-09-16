#ifndef MYPLOT_H
#define MYPLOT_H

#include "channel.h"
#include "enums.h"
#include "plotdata.h"
#include "qcustomplot.h"
#include "settings.h"
#include <QTimer>
#include <QWidget>

class myPlot : public QCustomPlot {
  Q_OBJECT
public:
  explicit myPlot(QWidget *parent = nullptr);
  void updateCursors(double x1, double x2, double y1, double y2);
  void init(Settings *in_settings);

private:
  QTimer updateTimer;
  Settings *settings;
  QVector<QCPItemLine *> zeroLines;
  double curX1 = 0, curX2 = 0, curY1 = 0, curY2 = 0;
  QCPItemLine *cursorX1, *cursorX2, *cursorY1, *cursorY2;
  void initCursors();
  void initZeroLines();
  int plottingStatus = PLOT_STATUS_RUN;
  int plottingRangeType = PLOT_RANGE_FIXED;
  double minTime();
  double maxTime();
  double graphLastTime(quint8 i);
  double minT;
  double maxT;
public slots:
  void setRefreshPeriod(int period) { updateTimer.setInterval(period); }
  void update();
  void setRangeType(int type);
  void pauseClicked();
  void singleTrigerClicked();
  void setVerticalDiv(double value);
  void setHorizontalDiv(double value);
  void setShowVerticalValues(bool enabled);
  void setShowHorizontalValues(bool enabled);
  void setCurXen(bool en);
  void setCurYen(bool en);
  void updateVisuals();
  void resetChannels();
  void rescale(int ch, double relativeScale);
  void reoffset(int ch, double relativeOffset);
  void newData(QVector<Channel *> *channels);

signals:
  void updateHPosSlider(double min, double max, int step);
  void showPlotStatus(int code);
  void setHDivLimits(double hRange);
  void setVDivLimits(double vRange);
  void setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminull, double xmaxfull, double yminfull, double ymaxfull);
  void requestNewData();
};

#endif // MYPLOT_H
