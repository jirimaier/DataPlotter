#ifndef MYMAINPLOT_H
#define MYMAINPLOT_H

#include "myplot.h"
#include "plotdata.h"
#include <QTimer>

class MyMainPlot : public MyPlot {
  Q_OBJECT
public:
  explicit MyMainPlot(QWidget *parent = nullptr);
  ~MyMainPlot();
  void init();
  bool isChUsed(int ch) { return !graph(ch)->data()->isEmpty(); }
  double getCHDiv(int ch) { return (getVDiv() / std::abs(channelSettings.at(ch).scale)); }
  double getChScale(int ch) { return std::abs(channelSettings.at(ch).scale); }
  bool isInverted(int ch) { return channelSettings.at(ch).scale < 0; }
  double getChOffset(int ch) { return channelSettings.at(ch).offset; }
  int getChStyle(int ch) { return channelSettings.at(ch).style; }
  QColor getChColor(int ch) { return channelSettings.at(ch).color; }
  void setChStyle(int ch, int style);
  void setChColor(int ch, QColor color);
  void changeChOffset(int ch, double offset);
  void changeChScale(int ch, double scale);
  QByteArray exportChannelCSV(char separator, char decimal, int channel, int precision, bool offseted, bool onlyInView);
  QByteArray exportAllCSV(char separator, char decimal, int precision, bool offseted, bool onlyInView, bool includeHidden);
  inline QPair<QVector<double>, QVector<double>> getDataVector(int ch, bool includeOffsets, bool onlyInView = false);
  void clearCh(int chid);
  double getChMax(int chid);
  double getChMin(int chid);

private:
  QVector<QVector<double>> pauseBufferTime;
  QVector<QVector<double>> pauseBufferValue;
  PlotSettings_t plotSettings;
  QVector<ChannelSettings_t> channelSettings;
  PlotRange::enumerator plotRangeType = PlotRange::fixedRange;
  QVector<QCPItemLine *> zeroLines;
  void initZeroLines();
  PlotStatus::enumerator plottingStatus = PlotStatus::run;
  int plottingRangeType = PlotStatus::pause;
  inline double minTime();
  inline double maxTime();
  double minT = 0.0, maxT = 1.0;

public slots:
  void togglePause();
  void resume();
  void update();
  void setRangeType(int type);
  void updateVisuals();
  void resetChannels();
  void rescale(int ch, double relativeScale);
  void reoffset(int ch, double relativeOffset);
  void newDataVector(int ch, QVector<double> *time, QVector<double> *value, bool isMath = false);
  void setRollingRange(double value) { plotSettings.rollingRange = value; }
  void setHorizontalPos(double value) { plotSettings.horizontalPos = value; }
  void setVerticalRange(double value) { plotSettings.verticalRange = value; }
  void setZoomRange(int value) { plotSettings.zoom = value; }
  void setVerticalCenter(int value) { plotSettings.verticalCenter = value; }
  void newDataPoint(int ch, double time, double value, bool append);
  void pause();

signals:
  void updateHPosSlider(double min, double max, int step);
  void showPlotStatus(PlotStatus::enumerator type);
  void updateDivs(double vertical, double horizontal);
};

#endif // MYMAINPLOT_H
