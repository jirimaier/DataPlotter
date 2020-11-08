#include "plotdata.h"

PlotData::PlotData(QObject *parent) : QObject(parent) { qDebug() << "PlotData created from " << QThread::currentThreadId(); }

PlotData::~PlotData() { qDebug() << "PlotData deleted from " << QThread::currentThreadId(); }

void PlotData::init() {
  qDebug() << "PlotData initialised from " << QThread::currentThreadId();
  reset();
}

double PlotData::arrayToDouble(QByteArray &array, bool &isok) {
  isok = true;
  if (isdigit(array.at(0)) || array.at(0) == '-') {
    return array.toDouble(&isok);

    // Little endian
  } else if (array.left(2) == "u1") { // unsigned int 8
    return (double)((uint8_t)array.at(2));
  } else if (array.left(2) == "u2") { // unsigned int 16
    char bytes[2] = {array.at(2), array.at(3)};
    return (double)*((uint16_t *)bytes);
  } else if (array.left(2) == "u4") { // unsigned int 32
    char bytes[4] = {array.at(2), array.at(3), array.at(4), array.at(5)};
    return (double)*((uint32_t *)bytes);

  } else if (array.left(2) == "i1") { // signed int 8
    return (double)((int8_t)array.at(2));
  } else if (array.left(2) == "i2") { // signed int 16
    char bytes[2] = {array.at(2), array.at(3)};
    return (double)*((int16_t *)bytes);
  } else if (array.left(2) == "i4") { // signed int 32
    char bytes[4] = {array.at(2), array.at(3), array.at(4), array.at(5)};
    return (double)*((int32_t *)bytes);

  } else if (array.left(2) == "f4") { // float
    char bytes[4] = {array.at(2), array.at(3), array.at(4), array.at(5)};
    return (double)*((float *)bytes);
  } else if (array.left(2) == "d8") { // double
    char bytes[8] = {array.at(2), array.at(3), array.at(4), array.at(5), array.at(6), array.at(7), array.at(8), array.at(9)};
    return (*((double *)bytes));

    // Big endian
  } else if (array.left(2) == "U1") { // unsigned int 8
    return (double)((uint8_t)array.at(2));
  } else if (array.left(2) == "U2") { // unsigned int 16
    char bytes[2] = {array.at(3), array.at(2)};
    return (double)*((uint16_t *)bytes);
  } else if (array.left(2) == "U4") { // unsigned int 32
    char bytes[4] = {array.at(5), array.at(4), array.at(3), array.at(2)};
    return (double)*((uint32_t *)bytes);

  } else if (array.left(2) == "I1") { // signed int 8
    return (double)((int8_t)array.at(2));
  } else if (array.left(2) == "I2") { // signed int 16
    char bytes[2] = {array.at(3), array.at(2)};
    return (double)*((int16_t *)bytes);
  } else if (array.left(2) == "I4") { // signed int 32
    char bytes[4] = {array.at(5), array.at(4), array.at(3), array.at(2)};
    return (double)*((int32_t *)bytes);

  } else if (array.left(2) == "F4") { // float
    char bytes[4] = {array.at(5), array.at(4), array.at(3), array.at(2)};
    return (double)*((float *)bytes);
  } else if (array.left(2) == "D8") { // double
    char bytes[8] = {array.at(9), array.at(8), array.at(7), array.at(6), array.at(5), array.at(4), array.at(3), array.at(2)};
    return (*((double *)bytes));
  }
  isok = false;
  return 0;
}

/*void PlotData::newDataString(QByteArray data) {
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
    emit addVectorToPlot(i + 1, times.at(i), values.at(i), times.at(i)->first() > lastTimes[i], false);
    lastTimes[i] = thisLastTime;
  }
}*/

/*void PlotData::newDataBin(QByteArray data, BinDataSettings_t settings) {
  QVector<QVector<double> *> times, values;
  for (int i = 0; i < settings.numCh; i++) {
    times.append(new QVector<double>);
    values.append(new QVector<double>);
  }
  if (!settings.continuous) {
    for (int i = settings.firstCh; i < settings.numCh + settings.firstCh; i++)
      lastTimes[i - 1] = 0;
  }
  int bytes = qCeil(settings.bits / 8.0f);
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
    emit addVectorToPlot(i + settings.firstCh, times.at(i), values.at(i), settings.continuous, false);
  }
}*/

void PlotData::addPoint(QByteArrayList data) {
  qDebug() << "new point";
  if (data.length() > CHANNEL_COUNT) {
    QByteArray message = QString::number(data.length() - 1).toUtf8();
    emit sendMessageIfAllowed(tr("Too many channels in point (missing ';' ?)").toUtf8(), message, MessageLevel::error);
    return;
  }
  int ch = 0;
  double time;
  for (QByteArrayList::iterator it = data.begin(); it != data.end(); it++) {

    if (ch == 0) {
      if (it->isEmpty()) {
        if (qIsInf(lastTimes[0]))
          time = 0;
        else
          time = lastTimes[0] + defaultTimestep;
      }
      bool isok;
      time = arrayToDouble(*it, isok);
      if (!isok) {
        sendMessageIfAllowed(tr("Can not parse points time").toUtf8(), *it, MessageLevel::error);
        return;
      }
      ch++;
      continue;
    }
    if (it->at(0) != 'b') {
      bool isok;
      double value = arrayToDouble(*it, isok);
      if (!isok) {
        sendMessageIfAllowed(tr("Can not parse points value").toUtf8(), *it, MessageLevel::error);
        return;
      }
      emit addPointToPlot(ch, time, value, time >= lastTimes[ch - 1]);
      lastTimes[ch - 1] = time;
      ch++;
    }
  }
}

void PlotData::addChannel(QByteArray data, unsigned int ch, QByteArray timeRaw) { qDebug() << "New channel data: ch" << ch << "timestep: " << timeRaw << " data: " << data; }

void PlotData::reset() {
  for (int i = 0; i < CHANNEL_COUNT; i++)
    lastTimes[i] = INFINITY;
}

void PlotData::sendMessageIfAllowed(const char *header, QByteArray &message, MessageLevel::enumerator type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message, type);
}
