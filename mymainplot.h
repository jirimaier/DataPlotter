//  Copyright (C) 2020-2021  Jiří Maier

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef MYMAINPLOT_H
#define MYMAINPLOT_H

#include <QTimer>

#include "myplot.h"
#include "plotdata.h"

class MyMainPlot : public MyPlot {
  Q_OBJECT
 public:
  explicit MyMainPlot(QWidget* parent = nullptr);
  ~MyMainPlot();
  void init();

  /// Rozsah vzorků který je vydět v zobrazení
  QPair<unsigned int, unsigned int> getChVisibleSamplesRange(int chID);
  bool isChUsed(int chID) { return !graph(chID)->data()->isEmpty(); }
  int getLogicBitsUsed(int group);

  double getCHDiv(int chID) { return (getVDiv() / channelSettings.at(chID).scale); }
  double getChScale(int chID) { return std::abs(channelSettings.at(chID).scale); }
  bool isChInverted(int chID) { return channelSettings.at(chID).inverted; }
  bool isChInterpolated(int chID) { return channelSettings.at(chID).interpolate; }
  double getChOffset(int chID) { return channelSettings.at(chID).offset; }
  int getChStyle(int chID) { return channelSettings.at(chID).style; }
  QColor getChColor(int chID) { return channelSettings.at(chID).color; }
  bool isChVisible(int chID) { return channelSettings.at(chID).visible; }

  void setChStyle(int chID, int style);
  void setChColor(int chID, QColor color);
  void setChOffset(int chID, double offset);
  void setChScale(int chID, double scale);
  void setChInvert(int chID, bool inverted);
  void setChInterpolate(int chID, bool enabled);;
  void setChVisible(int chID, bool visible);

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

  QByteArray exportChannelCSV(char separator, char decimal, int chID, int precision, bool onlyInView);
  QByteArray exportLogicCSV(char separator, char decimal, int group, int precision, bool onlyInView);
  QByteArray exportAllCSV(char separator, char decimal, int precision, bool onlyInView, bool includeHidden);

  void redraw();

  QCPAxis* getAnalogAxis(int chID)const {return analogAxis.at(chID);}

  QVector<QSharedPointer<QCPGraphDataContainer>> dataToBeInterpolated;

 private:
  QTimer plotUpdateTimer;

  void resume();
  void pause();
  void initZeroLines();
  void updateMinMaxTimes();
  void reOffsetAndRescaleCH(int chID);
  void reOffsetAndRescaleLogic(int chID);
  QPair<QVector<double>, QVector<double>> getDataVector(int chID, bool onlyInView = false);
  void updateTracerText(int index);
  int currentTracerIndex = -1;

  int mouseDragChIndex = 0;
  void setMouseCursorStyle(QMouseEvent* event);

  bool newData = true;
  double minT = 0.0, maxT = 1.0;
  int shiftStep = 0;
  double presetVRange = 10, presetVCenterRatio = 0;
  double rollingRange = 100;
  int zoom = 1000;
  double horizontalPos = 500;

  QList<QCPAxis*> analogAxis, logicGroupAxis;
  QVector<QSharedPointer<QCPGraphDataContainer>> pauseBuffer;
  QVector<ChannelSettings_t> channelSettings;
  QVector<ChannelSettings_t> logicSettings;
  QVector<QCPItemLine*> zeroLines;
  PlotStatus::enumPlotStatus plottingStatus = PlotStatus::run;
  PlotRange::enumPlotRange plotRangeType = PlotRange::fixedRange;

 private slots:
  void verticalAxisRangeChanged();
  void mouseMoved(QMouseEvent* event);
  void mousePressed(QMouseEvent* event);

 public slots:
  void togglePause();
  void resetChannels();
  void update();

  void setRangeType(PlotRange::enumPlotRange type);
  void setRollingRange(double value);
  void setHorizontalPos(double value);
  void setVerticalRange(double value);
  void setZoomRange(int value);
  void setVerticalCenter(int value);
  void setShiftStep(int step);

  void clearLogicGroup(int number, int fromBit);
  void clearCh(int chID);

  void newDataPoint(int chID, double time, double value, bool append);
  void newDataVector(int chID, QSharedPointer<QCPGraphDataContainer> data, bool ignorePause = false);
  void newInterpolatedVector(int chID, QSharedPointer<QCPGraphDataContainer> dataOriginal, QSharedPointer<QCPGraphDataContainer> dataInterpolated, bool dataIsFromInterpolationBuffer);

  void clearInterpolation(int interpolationID) {graph(INTERPOLATION_CHID(interpolationID))->data()->clear();}

 signals:
  void updateHPosSlider(double min, double max, int step);
  void showPlotStatus(PlotStatus::enumPlotStatus type);
  void requestCursorUpdate();
  void offsetChangedByMouse(int chid);
};

#endif // MYMAINPLOT_H
