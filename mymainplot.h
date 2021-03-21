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

  /// Jsou v kanálu data?
  bool isChUsed(int chID) { return !graph(chID)->data()->isEmpty(); }

  /// Počet využitých bitů logiky
  int getLogicBitsUsed(int group);

  /// Počet jednotek na krok mřížky
  double getCHDiv(int chID) { return (getVDiv() / channelSettings.at(chID).scale); }

  /// Vrátí zvětšení kanálu
  double getChScale(int chID) { return std::abs(channelSettings.at(chID).scale); }

  /// Je převrácený?
  bool isChInverted(int chID) { return channelSettings.at(chID).inverted; }

  /// Je zapnuta interpolace?
  bool isChInterpolated(int chID) { return channelSettings.at(chID).interpolate; }

  /// Vrátí offset kanálu
  double getChOffset(int chID) { return channelSettings.at(chID).offset; }

  /// Vrátí styl kanálu
  int getChStyle(int chID) { return channelSettings.at(chID).style; }

  /// Vrátí byrvu kanálu
  QColor getChColor(int chID) { return channelSettings.at(chID).color; }

  /// Je kanál viditelný/skrytý?
  bool isChVisible(int chID) { return channelSettings.at(chID).visible; }


  /// Nastavý styl
  void setChStyle(int chID, int style);

  /// Nastaví barvu
  void setChColor(int chID, QColor color);

  /// Nastaví offset
  void setChOffset(int chID, double offset);

  /// Nastaví zvětšení
  void setChScale(int chID, double scale);

  /// Nastaví převrácení
  void setChInvert(int chID, bool inverted);

  /// Nastaví interpolaci
  void setChInterpolate(int chID, bool enabled);;

  /// Nastaví zobrazení/skrytí
  void setChVisible(int chID, bool visible);


  /// Nastaví offset
  void setLogicOffset(int group, double offset);

  /// Nastaví zvětšení
  void setLogicScale(int group, double scale);

  /// Nastaví styl
  void setLogicStyle(int group, int style);

  /// Nastaví barvu
  void setLogicColor(int group, QColor color);

  /// Nastaví zobrazení/skrytí
  void setLogicVisibility(int group, bool visible);



  /// Vrátí zvětšení
  double getLogicScale(int group) { return logicSettings.at(group).scale; }

  /// Vrátí offset
  double getLogicOffset(int group) { return logicSettings.at(group).offset; }

  /// Vrátí styl
  int getLogicStyle(int group) { return logicSettings.at(group).style; }

  /// Vrátí barvu
  QColor getLogicColor(int group) { return logicSettings.at(group).color; }

  /// Je zobrazený/skrytý?
  bool isLogicVisible(int group) { return logicSettings.at(group).visible; }

  /// Exportuje jeden analogový kanál
  QByteArray exportChannelCSV(char separator, char decimal, int chID, int precision, bool onlyInView);

  /// Exportuje skupinu logických kanálů
  QByteArray exportLogicCSV(char separator, char decimal, int group, int precision, bool onlyInView);

  /// Exportuje vše (včetně logických)
  QByteArray exportAllCSV(char separator, char decimal, int precision, bool onlyInView, bool includeHidden);

  /// Vrátí osu hodnot zadaného kanálu
  QCPAxis* getAnalogAxis(int chID)const {return analogAxis.at(chID);}

  /// Fronta kanálů pro interpolování
  QVector<QSharedPointer<QCPGraphDataContainer>> dataToBeInterpolated;

  /// Nastaví OpenGL a překreslí graf
  void setOpenGL(bool enable) {QCustomPlot::setOpenGl(enable); redraw();}

 private:
  void redraw();

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
  void update();
  void verticalAxisRangeChanged();
  void mouseMoved(QMouseEvent* event);
  void mousePressed(QMouseEvent* event);

 public slots:
  /// Přepne pauzu/běh
  void togglePause();

  /// Vymaže kanály
  void resetChannels();

  /// Nastaví typ rozsahu
  void setRangeType(PlotRange::enumPlotRange type);

  /// Nastaví délku rolling režimu
  void setRollingRange(double value);

  /// Nastavý pozici vodorovně (při zoomu)
  void setHorizontalPos(double value);

  /// Nastaví svislí rozsah
  void setVerticalRange(double value);

  /// Nastaví zoom
  void setZoom(int value);

  /// Nastaví svyslí střed (-100 až 100, 0 je symetricky)
  void setVerticalCenter(int value);

  /// Krok předbíhání rolling režimu v % délky
  void setShiftStep(int step);

  /// Vymaže skupinu logických bitů
  void clearLogicGroup(int number, int fromBit);

  /// Vymaže kanál
  void clearCh(int chID);

  /// Přidá bod do kanálu
  void newDataPoint(int chID, double time, double value, bool append);

  /// Přepíše data v kanálu, pokud je zde jen jeden bod, přidá ho jako bod (nepřepíše původní).
  void newDataVector(int chID, QSharedPointer<QCPGraphDataContainer> data, bool ignorePause = false);

  /// Přepíše graf interpolace a graf s kanálem který je interpolován
  void newInterpolatedVector(int chID, QSharedPointer<QCPGraphDataContainer> dataOriginal, QSharedPointer<QCPGraphDataContainer> dataInterpolated, bool dataIsFromInterpolationBuffer);

  /// Vymaže interpolaci kanálu
  void clearInterpolation(int interpolationID) {graph(INTERPOLATION_CHID(interpolationID))->data()->clear();}

 signals:
  //void updateHPosSlider(double min, double max, int step);

  /// Změnil se stav (běží/pauza)
  void showPlotStatus(PlotStatus::enumPlotStatus type);

  /// Změna dat, přepočítat polohu kurzorů
  void requestCursorUpdate();

  /// Offset kanálu tažen myší
  void offsetChangedByMouse(int chid);
};

#endif // MYMAINPLOT_H
