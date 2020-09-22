#include "plotdata.h"

PlotData::PlotData() {
  for (int i = 0; i < CHANNEL_COUNT; i++)
    channels.append(new Channel());
}

PlotData::~PlotData() {}

void PlotData::newDataString(QByteArray data) {
  QByteArrayList pointList = data.split(';');
  for (long i = 0; i < pointList.length(); i++) {
    QByteArrayList values = pointList.at(i).split(',');
    for (int ch = 1; ch < values.length(); ch++) {
      if (!values.at(ch).isEmpty())
        channels.at(ch - 1)->addValue(values.at(ch).toDouble(), values.at(0).isEmpty() ? channels.at(0)->lastAddedTime + 1 : values.at(0).toDouble());
    }
  }
  emit dataReady(channels);
}

void PlotData::clearChannels() {
  for (int ch = 0; ch < CHANNEL_COUNT; ch++)
    channels.at(ch)->clear();
}

void PlotData::newDataBin(QByteArray data, binDataSettings_t settings) {
  int bytes = ceil(settings.bits / 8.0f);
  if (data.length() % bytes != 0)
    data = data.left(data.length() - data.length() % bytes);
  if (!settings.continuous)
    for (int ch = settings.firstCh; ch < settings.firstCh + settings.numCh; ch++)
      this->channels.at(ch - 1)->lastAddedTime = 0;
  for (int i = 0; i < data.length() - 1; i += bytes) {
    quint64 value = 0;
    for (int byte = 0; byte < bytes; byte++) {
      value = value << 8;
      value |= (quint8)data.at(i + byte);
    }
    double value_d = value;
    value_d = (value_d / (1 << settings.bits) * (settings.valueMax - settings.valueMin)) + settings.valueMin;
    channels.at(settings.firstCh + ((i / bytes) % settings.numCh) - 1)->addValue(value_d, channels.at(settings.firstCh + (i % settings.numCh) - 1)->lastAddedTime + settings.timeStep);
  }
  emit dataReady(channels);
}
