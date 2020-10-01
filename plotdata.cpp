#include "plotdata.h"
#include <QDebug>

PlotData::PlotData() {}

PlotData::~PlotData() { qDebug() << "PlotData deleted"; }

void PlotData::init() { reset(); }

void PlotData::newDataString(QByteArray data) {
  QVector<QVector<double> *> times;
  QVector<QVector<double> *> values;
  QByteArrayList pointList = data.split(';');
  for (long i = 0; i < pointList.length(); i++) {
    QByteArrayList val = pointList.at(i).split(',');
    for (int ch = 1; ch < val.length(); ch++) {
      if (times.length() < ch) {
        times.append(new QVector<double>);
        values.append(new QVector<double>);
      }
      if (!val.at(ch).isEmpty()) {
        values.at(ch - 1)->append(val.at(ch).toDouble());
        times.at(ch - 1)->append(val.at(0).isEmpty() ? i : val.at(0).toDouble());
      }
    }
  }
  for (int i = 0; i < values.length(); i++) {
    double thisLastTime = times.at(i)->last();
    emit updatePlot(i + 1, times.at(i), values.at(i), times.at(i)->first() > lastTimes[i], true);
    lastTimes[i] = thisLastTime;
  }
}

void PlotData::newDataBin(QByteArray data, BinDataSettings_t settings) {
  QVector<QVector<double> *> times, values;
  for (int i = 0; i < settings.numCh; i++) {
    times.append(new QVector<double>);
    values.append(new QVector<double>);
  }
  if (!settings.continuous) {
    for (int i = settings.firstCh; i < settings.numCh + settings.firstCh; i++)
      lastTimes[i - 1] = 0;
  }
  int bytes = ceil(settings.bits / 8.0f);
  if (data.length() % bytes != 0)
    data = data.left(data.length() - data.length() % bytes);
  for (int i = 0; i < data.length() - 1; i += bytes) {
    quint64 value = 0;
    for (int byte = 0; byte < bytes; byte++) {
      value = value << 8;
      value |= (quint8)data.at(i + byte);
    }
    double value_d = value;
    int chIndex = ((i / bytes) % settings.numCh);
    value_d = (value_d / (1 << settings.bits) * (settings.valueMax - settings.valueMin)) + settings.valueMin;
    values.at(chIndex)->append(value_d);
    if (times.at(chIndex)->isEmpty())
      times.at(chIndex)->append(lastTimes[chIndex + settings.firstCh - 1]);
    else
      times.at(chIndex)->append(times.at(chIndex)->last() + settings.timeStep);
  }
  for (int i = 0; i < settings.numCh; i++) {
    lastTimes[i + settings.firstCh - 1] = times.at(i)->last();
    emit updatePlot(i + settings.firstCh, times.at(i), values.at(i), settings.continuous, true);
  }
}

void PlotData::reset() {
  for (int i = 0; i < CHANNEL_COUNT; i++)
    lastTimes[i] = 0;
}
