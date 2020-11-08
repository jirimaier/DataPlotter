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
  bool isChUsed(int ch) { return !graph(ch - 1)->data()->isEmpty(); }
  double getCHDiv(int ch) { return (this->getVDiv() / std::abs(scales.at(ch - 1))); }
  double getChScale(int ch) { return std::abs(scales.at(ch - 1)); }
  bool isInverted(int ch) { return scales.at(ch - 1) < 0; }
  double getChOffset(int ch) { return offsets.at(ch - 1); }
  int getChStyle(int ch) { return channelSettings.at(ch - 1)->style; }
  QColor getChColor(int ch) { return channelSettings.at(ch - 1)->color; }
  void setChStyle(int ch, int style);
  void setChColor(int ch, QColor color);
  void setChName(int ch, QString name);
  QString getChName(int ch) { return channelSettings.at(ch - 1)->name; }
  void changeChOffset(int ch, double offset);
  void changeChScale(int ch, double scale);
  bool isPaused() { return plottingStatus == PlotStatus::run; }
  QVector<double> getOffsets() { return offsets; }
  QVector<double> getScales() { return scales; }
  QByteArray exportChannelCSV(char separator, char decimal, int channel, int precision, bool offseted, bool onlyInView);
  QByteArray exportAllCSV(char separator, char decimal, int precision, bool offseted, bool onlyInView, bool includeHidden);
  QPair<QVector<double>, QVector<double>> getDataVector(int ch, bool includeOffsets, bool onlyInView = false);
  double getChMinValue(int ch);
  double getChMaxValue(int ch);
  void clearCh(int ch);

private:
  bool lastWasContinous[CHANNEL_COUNT + MATH_COUNT];
  QVector<QVector<double> *> pauseBufferTime;
  QVector<QVector<double> *> pauseBufferValue;
  QVector<double> offsets;
  QVector<double> scales;
  PlotSettings_t plotSettings;
  QVector<ChannelSettings_t *> channelSettings;
  PlotRange::enumerator plotRangeType = PlotRange::fixedRange;
  QVector<QCPItemLine *> zeroLines;
  void initZeroLines();
  PlotStatus::enumerator plottingStatus = PlotStatus::run;
  int plottingRangeType = PlotStatus::pause;
  double minTime();
  double maxTime();
  double graphLastTime(quint8 i);
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
  void newDataVector(int ch, QVector<double> *time, QVector<double> *value, bool append, bool ignorePause = false);
  void setRollingRange(double value) { plotSettings.rollingRange = value; }
  void setHorizontalPos(double value) { plotSettings.horizontalPos = value; }
  void setVerticalRange(double value) { plotSettings.verticalRange = value; }
  void setZoomRange(int value) { plotSettings.zoom = value; }
  void setVerticalCenter(int value) { plotSettings.verticalCenter = value; }
  void newDataPoint(int ch, double time, double value, bool append, bool ignorePause);
  void pause();

signals:
  void updateHPosSlider(double min, double max, int step);
  void showPlotStatus(PlotStatus::enumerator type);
  void updateDivs(double vertical, double horizontal);
};

#endif // MYMAINPLOT_H
