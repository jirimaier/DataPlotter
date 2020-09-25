#ifndef MYPLOT_H
#define MYPLOT_H

#include "channel.h"
#include "plotdata.h"
#include "qcustomplot.h"
#include "settings.h"
#include <QTimer>
#include <QWidget>

class MyPlot : public QCustomPlot {
  Q_OBJECT
public:
  explicit MyPlot(QWidget *parent = nullptr);
  void updateCursors(double x1, double x2, double y1, double y2);
  void init();
  double getVDiv() { return vdiv; }
  double getHDiv() { return hdiv; }
  double getCHDiv(int ch) { return vdiv / abs(channelSettings.at(ch - 1)->scale); }
  double getChScale(int ch) { return abs(channelSettings.at(ch - 1)->scale); }
  double getChOffset(int ch) { return channelSettings.at(ch - 1)->offset; }
  int getChStyle(int ch) { return channelSettings.at(ch - 1)->style; }
  QColor getChColor(int ch) { return channelSettings.at(ch - 1)->color; }
  void setChStyle(int ch, int style);
  void setChColor(int ch, QColor color);
  void changeChOffset(int ch, double offset);
  void changeChScale(int ch, double scale);

private:
  double vdiv, hdiv;
  PlotSettings_t plotSettings;
  QVector<ChannelSettings_t *> channelSettings;
  int plotRangeType = PLOT_RANGE_FIXED;
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
  double minT = 0.0, maxT = 1.0;

public slots:
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
  void newData(QVector<Channel *> channels);
  void setRollingRange(double value);
  void setHorizontalPos(double value);
  void setVerticalRange(double value);
  void setZoomRange(int value);
  void setVerticalCenter(int value);

signals:
  void updateHPosSlider(double min, double max, int step);
  void showPlotStatus(int code);
  void updateDivs(double vertical, double horizontal);
  void setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminull, double xmaxfull, double yminfull, double ymaxfull);
};

#endif // MYPLOT_H
