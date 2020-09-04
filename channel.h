#ifndef CHANNEL_H
#define CHANNEL_H

#include <QObject>
#include <QVector>

#include "enums.h"
#include "qcustomplot.h"

class Channel {
public:
  Channel(int number, QCustomPlot *plot, QColor color);
  void changeColor(QColor color);
  void changeOffset(double offset);
  void changeScale(double in_scale);
  void addValue(QByteArray in_value, QByteArray in_time);
  void addValue(double in_value, double in_time);
  void clear();
  void draw();
  void setStyle(int type);
  double upperTimeRange();
  double lastDrawnTime();
  double firstTime() { return time.at(0); }
  double getOffset() { return offset; }
  double getScale() { return scale; }
  bool isEmpty() { return time.length() == 0; }
  int getStyle() { return style; }
  QColor getColor() { return color; }

private:
  QCustomPlot *plot;
  QColor color;
  QPen dataPen, zeroPen;
  QVector<double> value;
  QVector<double> time;
  QCPItemLine *zeroLine;
  int dataChannelNumber;
  int lastDrawnIndex = 0;
  double lastSetMaxTime = -INFINITY;
  int style = PLOT_STYLE_LINE;
  double offset = 0;
  double scale = 1;
};

#endif // CHANNEL_H
