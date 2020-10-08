#ifndef PLOTTING_H
#define PLOTTING_H

#include <QDebug>
#include <QObject>
#include <QThread>
#include <QVector>
#include <QWidget>
#include <QtMath>

#include "enums_defines_constants.h"

class PlotData : public QObject {
  Q_OBJECT
public:
  PlotData();
  ~PlotData();
  void init();

private:
  double lastTimes[CHANNEL_COUNT];
public slots:
  void newDataString(QByteArray data);
  void newDataBin(QByteArray data, BinDataSettings_t settings);
  void reset();

signals:
  void updatePlot(int ch, QVector<double> *time, QVector<double> *value, bool continous, bool sorted);
};

#endif // PLOTTING_H
