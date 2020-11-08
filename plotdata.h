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
  explicit PlotData(QObject *parent = nullptr);
  ~PlotData();
  void init();

private:
  MessageLevel::enumerator debugLevel = MessageLevel::info;
  double lastTimes[CHANNEL_COUNT];
  double defaultTimestep = 1;
  void sendMessageIfAllowed(const char *header, QByteArray &message, MessageLevel::enumerator type);
  double arrayToDouble(QByteArray &array, bool &isok);
public slots:
  // void newDataString(QByteArray data);
  // void newDataBin(QByteArray data, BinDataSettings_t settings);
  void addPoint(QByteArrayList data);
  void addChannel(QByteArray data, unsigned int ch, QByteArray timeRaw);
  void reset();

signals:
  /// Předá zprávu od zařízení
  void sendMessage(QByteArray header, QByteArray message, MessageLevel::enumerator type);

  /// Předá data do grafu
  void addVectorToPlot(int ch, QVector<double> *time, QVector<double> *value, bool append, bool ignorePause = false);

  /// Předá data do grafu
  void addPointToPlot(int ch, double time, double value, bool append, bool ignorePause = false);
};

#endif // PLOTTING_H
