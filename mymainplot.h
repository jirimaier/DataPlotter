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
  QByteArray exportLogicCSV(char separator, char decimal, int channel, int precision, bool offseted, bool onlyInView);
  QByteArray exportAllCSV(char separator, char decimal, int precision, bool offseted, bool onlyInView, bool includeHidden);
  QPair<QVector<double>, QVector<double>> getDataVector(int ch, bool includeOffsets, bool onlyInView = false);
  void clearCh(int chid) { this->graph(chid)->data().data()->clear(); }
  double getChMax(int chid);
  double getChMin(int chid);
  void changeLogicOffset(int group, double offset);
  void changeLogicScale(int group, double scale);
  void setLogicStyle(int group, int style);
  void setLogicColor(int group, QColor color);
  int getLogicBitsUsed(int group);
  QPair<long, long> getChVisibleSamples(int chid);
  QPair<double, double> setTimeCursor(int cursor, int chid, unsigned int sample);

private:
  void redraw();
  bool newData = true, repaintNeeded = true;
  void setUpLogic();
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
  int shiftStep = 0;

public slots:
  void togglePause();
  void resume();
  void update();
  void setRangeType(int type);
  void resetChannels();
  void rescale(int ch, double relativeScale);
  void reoffset(int ch, double relativeOffset);
  void newDataVector(int ch, QSharedPointer<QVector<double>> time, QVector<double> *value, bool isMath = false);
  void newLogicDataVector(int ch, QSharedPointer<QVector<double>> time, QVector<uint32_t> *value, int bits);
  void setRollingRange(double value);
  void setHorizontalPos(double value);
  void setVerticalRange(double value);
  void setZoomRange(int value);
  void setVerticalCenter(int value);
  void newDataPoint(int ch, double time, double value, bool append);
  void pause();
  void clearLogicGroup(int number);
  void setShiftStep(int step) { shiftStep = step; }

signals:
  void updateHPosSlider(double min, double max, int step);
  void showPlotStatus(PlotStatus::enumerator type);
  void requestCursorUpdate();
};

#endif // MYMAINPLOT_H
