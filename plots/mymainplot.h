//  Copyright (C) 2020-2024  Jiří Maier

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

#include "communication/plotdata.h"
#include "myplot.h"

class MyMainPlot : public MyPlot {
  Q_OBJECT
public:
  explicit MyMainPlot(QWidget *parent = nullptr);
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
  QColor getChColor(int chID) { return channelSettings.at(chID).color(chClrTheme); }

  /// Vrátí byrvu kanálu
  QColor getChColorForTheme(int chID, int theme) { return channelSettings.at(chID).color(theme); }

  /// Je kanál viditelný/skrytý?
  bool isChVisible(int chID) { return channelSettings.at(chID).visible; }

  /// Nastavý styl
  void setChStyle(int chID, int style);

  /// Nastaví barvu
  void setChColor(int chID, QColor color, int themeIndex);

  /// Nastaví offset
  void setChOffset(int chID, double offset);

  /// Nastaví zvětšení
  void setChScale(int chID, double scale);

  /// Nastaví převrácení
  void setChInvert(int chID, bool inverted);

  /// Nastaví interpolaci
  void setChInterpolate(int chID, bool enabled);

  /// Nastaví zobrazení/skrytí
  void setChVisible(int chID, bool visible);

  /// Nastaví offset
  void setLogicOffset(int group, double offset);

  /// Nastaví zvětšení
  void setLogicScale(int group, double scale);

  /// Nastaví styl
  void setLogicStyle(int group, int style);

  /// Nastaví barvu
  void setLogicColor(int group, QColor color, int themeIndex);

  /// Nastaví zobrazení/skrytí
  void setLogicVisibility(int group, bool visible);

  /// Zobrazí/sryje čáru označující trigger
  void setTriggerLineVisible(bool visible);

  /// Nastaví pozici čáry označující trigger
  void setTriggerLineValue(double value);

  /// Nastaví kanál čáry označující trigger
  void setTriggerLineChannel(int chid);

  /// Vrátí zvětšení
  double getLogicScale(int group) { return logicSettings.at(group).scale; }

  /// Vrátí offset
  double getLogicOffset(int group) { return logicSettings.at(group).offset; }

  /// Vrátí styl
  int getLogicStyle(int group) { return logicSettings.at(group).style; }

  /// Vrátí barvu
  QColor getLogicColor(int group) { return logicSettings.at(group).color(chClrTheme); }

  /// Je zobrazený/skrytý?
  bool isLogicVisible(int group) { return logicSettings.at(group).visible; }

  /// Exportuje jeden analogový kanál
  QByteArray exportChannelCSV(char separator, char decimal, int chID, int precision, bool onlyInView);

  /// Exportuje skupinu logických kanálů
  QByteArray exportLogicCSV(char separator, char decimal, int group, int precision, bool onlyInView);

  /// Exportuje vše (včetně logických)
  QByteArray exportAllCSV(char separator, char decimal, int precision, bool onlyInView, bool includeHidden);

  /// Vrátí osu hodnot zadaného kanálu
  QCPAxis *getAnalogAxis(int chID) const { return analogAxis.at(chID); }

  /// Fronta kanálů pro interpolování
  QVector<QSharedPointer<QCPGraphDataContainer>> dataToBeInterpolated;

  /// Nastaví OpenGL a překreslí graf
  void setOpenGL(bool enable) {
    QCustomPlot::setOpenGl(enable);
    redraw();
  }

  double getMinT() const;

  double getMaxT() const;

  bool getRollingMode() const;

  void setRollingMode(bool newRollingMode);

  bool getLastDataTypeWasPoint() const;

  bool getAutoVRage() const;
  void setAutoVRage(bool newAutoVRage);

private:
  void redraw();

  QTimer plotUpdateTimer;

  void resume();
  void pause();
  void initZeroLines();
  void initTriggerLine();
  void updateMinMaxTimes();
  void reOffsetAndRescaleCH(int chID);
  void reOffsetAndRescaleLogic(int chID);
  QPair<QVector<double>, QVector<double>> getDataVector(int chID, bool onlyInView = false);
  void updateTracerText(int index);
  int currentTracerIndex = -1;

  void setLastDataTypeWasPoint(bool newLastDataTypeWasPoint);

  int mouseDragChIndex = 0;
  void setMouseCursorStyle(QMouseEvent *event);

  bool newData = true;
  double minT = 0.0, maxT = 1.0;

  bool xRangeUnknown = false;

  int rollingStep = 0;

  QList<QCPAxis *> analogAxis, logicGroupAxis;
  QVector<QSharedPointer<QCPGraphDataContainer>> pauseBuffer;
  QVector<ChannelSettings_t> channelSettings;
  QVector<ChannelSettings_t> logicSettings;
  QVector<QCPItemLine *> zeroLines;
  PlotStatus::enumPlotStatus plottingStatus = PlotStatus::run;
  bool rollingMode = true;
  QCPItemLine *triggerLine;
  QCPGraph *triggerLineCh;
  bool triggerLineEnabled = false;
  QCPItemText *triggerLabel;
  enum Mode { free, growing, rolling, empty, free_locked } mode = empty;
  double lastSignalEnd = 0;
  void updateRollingState(double xMax);
  bool lastDataTypeWasPoint = false;

  bool autoVRage = false;

private slots:
  void update();
  void verticalAxisRangeChanged();
  void mouseMoved(QMouseEvent *event);
  void mousePressed(QMouseEvent *event);
  void onXRangeChanged(QCPRange newRange, QCPRange oldRange);
  void onYRangeChanged(QCPRange newRange, QCPRange oldRange);

public slots:
  /// Přepne pauzu/běh
  void togglePause();

  /// Vymaže kanály
  void resetChannels();

  /// Krok předbíhání rolling režimu v % délky
  void setShiftStep(int step);

  /// Vymaže skupinu logických bitů
  void clearLogicGroup(int number, int fromBit);

  /// Vymaže kanál
  void clearCh(int chID);

  /// Přidá bod do kanálu
  void newDataPoint(int chID, double time, double value, bool append);

  /// Přepíše data v kanálu, pokud je zde jen jeden bod, přidá ho jako bod
  /// (nepřepíše původní).
  void newDataVector(int chID, QSharedPointer<QCPGraphDataContainer> data, bool ignorePause = false);

  /// Přepíše graf interpolace a graf s kanálem který je interpolován
  void newInterpolatedVector(int chID, QSharedPointer<QCPGraphDataContainer> dataOriginal, QSharedPointer<QCPGraphDataContainer> dataInterpolated, bool dataIsFromInterpolationBuffer);

  /// Vymaže interpolaci kanálu
  void clearInterpolation(int interpolationID) { graph(INTERPOLATION_CHID(interpolationID))->data()->clear(); }

  void setVRange(QCPRange range);
  void setVPos(double mid);
  void setHPos(double mid);
  void setHLen(double len);

signals:
  // void updateHPosSlider(double min, double max, int step);

  /// Změnil se stav (běží/pauza)
  void showPlotStatus(PlotStatus::enumPlotStatus type);

  /// Změna dat, přepočítat polohu kurzorů
  void requestCursorUpdate();

  /// Offset kanálu tažen myší
  void offsetChangedByMouse(int chid);
  void vRangeChanged(QCPRange range);
  void vRangeMaxChanged(QCPRange range);
  void hRangeChanged(QCPRange range);
  void hRangeMaxChanged(QCPRange range);
  void rollingModeChanged();
  void lastDataTypeWasPointChanged(bool);
  void autoVRageChanged();

  // MyPlot interface
public:
  void setTheme(QColor fnt, QColor bck, int chClrThemeId);
};

#endif // MYMAINPLOT_H
