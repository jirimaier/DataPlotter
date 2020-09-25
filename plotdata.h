#ifndef PLOTTING_H
#define PLOTTING_H

#include <QObject>
#include <QVector>
#include <QWidget>

#include "channel.h"
#include "serialparser.h"
#include "settings.h"

class PlotData : public QObject {
  Q_OBJECT
public:
  PlotData();
  ~PlotData();
  void clearChannels();
  bool newDataFlag = false;
  QVector<Channel *> channels;

private:
public slots:
  void newDataString(QByteArray data);
  void newDataBin(QByteArray data, BinDataSettings_t settings);

signals:
  void dataReady(QVector<Channel *> channels);
};

#endif // PLOTTING_H
