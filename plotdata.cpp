#include "plotdata.h"

PlotData::PlotData(Settings *in_settings) {
  this->settings = in_settings;
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
        channels.at(ch - 1)->addValue(values.at(ch).toDouble(), values.at(0).isEmpty() ? channels.at(0)->lastTime() + 1 : values.at(0).toDouble());
    }
  }
  newDataFlag = true;
}

void PlotData::clearChannels() {
  for (int ch = 0; ch < CHANNEL_COUNT; ch++)
    channels.at(ch)->clear();
  newDataFlag = false;
}

void PlotData::newDataBin(QByteArray data) {
  int bytes = ceil(settings->binDataSettings.bits / 8.0f);
  if (data.length() % bytes != 0)
    data = data.left(data.length() - data.length() % bytes);
  if (!settings->binDataSettings.continuous)
    for (int ch = settings->binDataSettings.firstCh; ch < settings->binDataSettings.firstCh + settings->binDataSettings.numCh; ch++)
      this->channels.at(ch - 1)->clear();
  for (int i = 0; i < data.length() - 1; i += bytes) {
    quint64 value = 0;
    for (int byte = 0; byte < bytes; byte++) {
      value = value << 8;
      value |= (quint8)data.at(i + byte);
    }
    double value_d = value;
    value_d = (value_d / (1 << settings->binDataSettings.bits) * (settings->binDataSettings.valueMax - settings->binDataSettings.valueMin)) + settings->binDataSettings.valueMin;
    channels.at(settings->binDataSettings.firstCh + ((i / bytes) % settings->binDataSettings.numCh) - 1)->addValue(value_d, channels.at(settings->binDataSettings.firstCh + (i % settings->binDataSettings.numCh) - 1)->lastTime() + settings->binDataSettings.timeStep);
  }
}
