#ifndef PLOTTING_H
#define PLOTTING_H

#include <QObject>
#include <QVector>
#include <QWidget>

#include "channel.h"
#include "enums.h"
#include "settings.h"

class PlotData : public QObject {
  Q_OBJECT
public:
  PlotData(Settings *in_settings);
  ~PlotData();
  void clearChannels();
  bool newDataFlag = false;
  QVector<Channel *> channels;

private:
  Settings *settings;

public slots:
  void newDataString(QByteArray data);
  void newDataBin(QByteArray data);
};

#endif // PLOTTING_H
