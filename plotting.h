#ifndef PLOTTING_H
#define PLOTTING_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include "channel.h"
#include "enums.h"

class Plotting : public QObject {
  Q_OBJECT
public:
  Plotting(QCustomPlot *plotWidget, QScrollBar *horizontalPos);
  ~Plotting();
  void clearChannels();
  Channel *channel(int chNumber) { return channels.at(chNumber - 1); }

private:
  QVector<Channel *> channels;
  QScrollBar *horizontalPos;
  void addPoint(QByteArray point);
  QTimer *timer = new QTimer(this);
  double minTime();
  double maxTime();

public slots:
  void newDataString(QByteArray data);
  void newDataBin(QByteArray data, int bits, double valMin, double valMax, double timeStep, int numCh, int firstCh, bool continuous);
};

#endif // PLOTTING_H
