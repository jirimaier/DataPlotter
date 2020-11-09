#ifndef PLOTTING_H
#define PLOTTING_H

#include <QDebug>
#include <QObject>
#include <QThread>
#include <QVector>
#include <QWidget>
#include <QtMath>

#include "enums_defines_constants.h"

using namespace ValueType;
class PlotData : public QObject {
  Q_OBJECT
public:
  explicit PlotData(QObject *parent = nullptr);
  ~PlotData();
  void init();

private:
  double getValue(QByteArray number, ValueType::enumerator type);
  ValueType::enumerator getType(QByteArray array);
  OutputLevel::enumerator debugLevel = OutputLevel::info;
  double lastTimes[CHANNEL_COUNT];
  double defaultTimestep = 1;
  void sendMessageIfAllowed(const char *header, QByteArray &message, MessageLevel::enumerator type);
  double arrayToDouble(QByteArray &array, bool &isok);
public slots:
  void addPoint(QByteArrayList data);
  void addChannel(QByteArray data, unsigned int ch, QByteArray timeRaw);
  void reset();
  /// Nastavý úroveň výpisu
  void setDebugLevel(OutputLevel::enumerator debugLevel) { this->debugLevel = debugLevel; }

signals:
  /// Pošle zprávu do výpisu
  void sendMessage(QByteArray header, QByteArray message, MessageLevel::enumerator type, MessageTarget::enumerator target = MessageTarget::serial1);

  /// Předá data do grafu
  void addVectorToPlot(int ch, QVector<double> *time, QVector<double> *value, bool append, bool ignorePause = false);

  /// Předá data do grafu
  void addPointToPlot(int ch, double time, double value, bool append, bool ignorePause = false);
};

#endif // PLOTTING_H
