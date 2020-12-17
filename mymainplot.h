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

  QPair<long, long> getChVisibleSamplesRange(int chid);
  bool isChUsed(int ch) { return !graph(ch)->data()->isEmpty(); }
  int getLogicBitsUsed(int group);

  double getCHDiv(int ch) { return (getVDiv() / channelSettings.at(ch).scale); }
  double getChScale(int ch) { return std::abs(channelSettings.at(ch).scale); }
  bool isChInverted(int ch) { return channelSettings.at(ch).inverted; }
  double getChOffset(int ch) { return channelSettings.at(ch).offset; }
  int getChStyle(int ch) { return channelSettings.at(ch).style; }
  QColor getChColor(int ch) { return channelSettings.at(ch).color; }
  bool isChVisible(int ch) { return channelSettings.at(ch).visible; }

  void setChStyle(int ch, int style);
  void setChColor(int ch, QColor color);
  void setChOffset(int ch, double offset);
  void setChScale(int ch, double scale);
  void setChInvert(int ch, bool inverted);
  void setChVisible(int ch, bool visible);

  void setLogicOffset(int group, double offset);
  void setLogicScale(int group, double scale);
  void setLogicStyle(int group, int style);
  void setLogicColor(int group, QColor color);
  void setLogicVisibility(int group, bool visible);

  double getLogicScale(int group) { return logicSettings.at(group).scale; }
  double getLogicOffset(int group) { return logicSettings.at(group).offset; }
  int getLogicStyle(int group) { return logicSettings.at(group).style; }
  QColor getLogicColor(int group) { return logicSettings.at(group).color; }
  bool isLogicVisible(int group) { return logicSettings.at(group).visible; }

  QPair<QVector<double>, QVector<double>> getDataVector(int ch, bool onlyInView = false);
  double getChMax(int chid);
  double getChMin(int chid);

  QByteArray exportChannelCSV(char separator, char decimal, int channel, int precision, bool onlyInView);
  QByteArray exportLogicCSV(char separator, char decimal, int channel, int precision, bool onlyInView);
  QByteArray exportAllCSV(char separator, char decimal, int precision, bool onlyInView, bool includeHidden);

private:
  QTimer plotUpdateTimer;

  void resume();
  void pause();
  void redraw();
  void initZeroLines();
  void updateMinMaxTimes();

  bool newData = true;

  QList<QCPAxis *> analogAxis, logicGroupAxis;
  QVector<QSharedPointer<QCPGraphDataContainer>> pauseBuffer;
  QVector<ChannelSettings_t> channelSettings;
  QVector<ChannelSettings_t> logicSettings;
  PlotSettings_t plotSettings;
  QVector<QCPItemLine *> zeroLines;
  PlotStatus::enumerator plottingStatus = PlotStatus::run;
  PlotRange::enumerator plotRangeType = PlotRange::fixedRange;
  double minT = 0.0, maxT = 1.0;
  int shiftStep = 0;
  double presetVRange = 10, presetVCenterRatio = 0;

  void reOffsetAndRescaleCH(int chid);
  void reOffsetAndRescaleLogic(int group);

private slots:
  void verticalAxisRangeChanged();

public slots:
  void togglePause();
  void resetChannels();
  void update();

  void setRangeType(PlotRange::enumerator type);
  void setRollingRange(double value);
  void setHorizontalPos(double value);
  void setVerticalRange(double value);
  void setZoomRange(int value);
  void setVerticalCenter(int value);
  void setShiftStep(int step);

  void clearLogicGroup(int number, int fromBit);
  void clearCh(int chid) { this->graph(chid)->data().data()->clear(); }

  void newDataPoint(int chID, double time, double value, bool append);
  void newDataVector(int ch, QSharedPointer<QCPGraphDataContainer> data, bool ignorePause = false);

signals:
  void updateHPosSlider(double min, double max, int step);
  void showPlotStatus(PlotStatus::enumerator type);
  void requestCursorUpdate();
};

#endif // MYMAINPLOT_H
