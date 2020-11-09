#include "plotdata.h"

PlotData::PlotData(QObject *parent) : QObject(parent) { qDebug() << "PlotData created from " << QThread::currentThreadId(); }

PlotData::~PlotData() { qDebug() << "PlotData deleted from " << QThread::currentThreadId(); }

void PlotData::init() {
  qDebug() << "PlotData initialised from " << QThread::currentThreadId();
  reset();
}

double PlotData::getValue(QByteArray number, ValueType::enumerator type) {
  if (type == u1) {
    return (double)((uint8_t)number.at(0));
  } else if (type == u2) { // unsigned int 16
    char bytes[2] = {number.at(0), number.at(1)};
    return (double)*((uint16_t *)bytes);
  } else if (type == u4) { // unsigned int 32
    char bytes[4] = {number.at(0), number.at(1), number.at(2), number.at(3)};
    return (double)*((uint32_t *)bytes);
  } else if (type == i1) { // signed int 8
    return (double)((int8_t)number.at(0));
  } else if (type == i2) { // signed int 16
    char bytes[2] = {number.at(0), number.at(1)};
    return (double)*((int16_t *)bytes);
  } else if (type == i4) { // signed int 32
    char bytes[4] = {number.at(0), number.at(1), number.at(2), number.at(3)};
    return (double)*((int32_t *)bytes);
  } else if (type == f4) { // float
    char bytes[4] = {number.at(0), number.at(1), number.at(2), number.at(3)};
    return (double)*((float *)bytes);
  } else if (type == d8) { // double
    char bytes[8] = {number.at(0), number.at(1), number.at(2), number.at(3), number.at(4), number.at(5), number.at(6), number.at(7)};
    return (*((double *)bytes));

    // Big endian
  } else if (type == U1) { // unsigned int 8
    return (double)((uint8_t)number.at(0));
  } else if (type == U2) { // unsigned int 16
    char bytes[2] = {number.at(1), number.at(0)};
    return (double)*((uint16_t *)bytes);
  } else if (type == U4) { // unsigned int 32
    char bytes[4] = {number.at(3), number.at(2), number.at(1), number.at(0)};
    return (double)*((uint32_t *)bytes);
  } else if (type == I1) { // signed int 8
    return (double)((int8_t)number.at(0));
  } else if (type == I2) { // signed int 16
    char bytes[2] = {number.at(1), number.at(0)};
    return (double)*((int16_t *)bytes);
  } else if (type == I4) { // signed int 32
    char bytes[4] = {number.at(3), number.at(2), number.at(1), number.at(0)};
    return (double)*((int32_t *)bytes);
  } else if (type == F4) { // float
    char bytes[4] = {number.at(3), number.at(2), number.at(1), number.at(0)};
    return (double)*((float *)bytes);
  } else if (type == D8) { // double
    char bytes[8] = {number.at(7), number.at(6), number.at(5), number.at(4), number.at(3), number.at(2), number.at(1), number.at(0)};
    return (*((double *)bytes));
  }
  return 0;
}

enumerator PlotData::getType(QByteArray array) {
  // Little endian
  if (array.left(2) == "u1") // unsigned int 8
    return u1;
  else if (array.left(2) == "u2") // unsigned int 16;
    return u2;
  if (array.left(2) == "u4") // unsigned int 32
    return u4;
  else if (array.left(2) == "i1") // signed int 8
    return i1;
  else if (array.left(2) == "i2") // signed int 16
    return i2;
  else if (array.left(2) == "i4") // signed int 32
    return i4;
  else if (array.left(2) == "f4") // float
    return f4;
  else if (array.left(2) == "d8") // double
    return d8;

  // Big endian
  else if (array.left(2) == "U1") // unsigned int 8
    return U1;
  else if (array.left(2) == "U2") // unsigned int 16
    return U2;
  else if (array.left(2) == "U4") // unsigned int 32
    return U4;
  else if (array.left(2) == "I1") // signed int 8
    return I1;
  else if (array.left(2) == "I2") // signed int 16;
    return I2;
  else if (array.left(2) == "I4") // signed int 32
    return I4;
  else if (array.left(2) == "F4") // float
    return F4;
  else if (array.left(2) == "D8") // double
    return D8;
  sendMessageIfAllowed(tr("Unknown type").toUtf8(), array, MessageLevel::error);
  return unrecognised;
}

double PlotData::arrayToDouble(QByteArray &array, bool &isok) {
  isok = true;
  if (isdigit(array.at(0)) || array.at(0) == '-') {
    return array.toDouble(&isok);
  }
  enumerator type = getType(array.left(2));
  if (type == unrecognised) {
    isok = false;
    return 0;
  }
  return getValue(array.mid(2), type);
}

void PlotData::addPoint(QByteArrayList data) {
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
  if (debugLevel == OutputLevel::info)
    sendMessage(tr("Received point").toUtf8(), tr("%1 channels").arg(data.length() - 1).toUtf8(), MessageLevel::info);
}

void PlotData::addChannel(QByteArray data, unsigned int ch, QByteArray timeRaw) {
  ValueType::enumerator type = getType(data.left(2));
  if (type == unrecognised) {
    return;
  }
  bool isok;
  double time = arrayToDouble(timeRaw, isok);
  if (!isok) {
    sendMessageIfAllowed(tr("Can not parse channel time step").toUtf8(), timeRaw, MessageLevel::error);
    return;
  }
  uint8_t bytesPerValue = data.mid(1, 1).toUInt(nullptr, 16);
  data.remove(0, 2);
  QVector<double> *times = new QVector<double>;
  QVector<double> *values = new QVector<double>;
  for (uint32_t i = 0; i < (uint32_t)data.length(); i += bytesPerValue) {
    times->append((i / bytesPerValue) * time);
    values->append(getValue(data.mid(i, bytesPerValue), type));
  }
  if (debugLevel == OutputLevel::info)
    sendMessage(tr("Received channel %1").arg(ch).toUtf8(), tr("%1 bytes, time step %2").arg(data.length() / bytesPerValue).arg(time).toUtf8(), MessageLevel::info);
  emit addVectorToPlot(ch, times, values, false, false);
}

void PlotData::reset() {
  for (int i = 0; i < CHANNEL_COUNT; i++)
    lastTimes[i] = INFINITY;
}

void PlotData::sendMessageIfAllowed(const char *header, QByteArray &message, MessageLevel::enumerator type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message, type);
}
