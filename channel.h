#ifndef CHANNEL_H
#define CHANNEL_H

#include <QObject>
#include <QVector>

#include "enums.h"
#include "qcustomplot.h"

class Channel {
public:
  Channel();
  void addValue(double in_value, double in_time);
  void clear();
  double lastTime();
  double firstTime() { return time.at(0); }
  bool isEmpty() { return time.isEmpty(); }
  QVector<double> value;
  QVector<double> time;
  void applyScaleAndOffset(double scale, double offset);
};

#endif // CHANNEL_H
