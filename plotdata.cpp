#include "plotdata.h"

PlotData::PlotData(QObject *parent) : QObject(parent) { qDebug() << "PlotData created from " << QThread::currentThreadId(); }

PlotData::~PlotData() { qDebug() << "PlotData deleted from " << QThread::currentThreadId(); }

void PlotData::init() {
  qDebug() << "PlotData initialised from " << QThread::currentThreadId();
  for (int i = 0; i < ANALOG_COUNT; i++)
    digitalChannel[i] = 0;
  for (int i = 0; i < LOGIC_COUNT; i++)
    logicBits[i] = 0;
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

uint32_t PlotData::getBits(QByteArray data, enumerator type) {
  // Little endian
  if (type == u1) {
    return ((uint32_t)((uint8_t)data.at(0)));
  } else if (type == u2) { // unsigned int 16
    char bytes[2] = {data.at(0), data.at(1)};
    return ((uint32_t) * ((uint16_t *)bytes));
  } else if (type == u4) { // unsigned int 32
    char bytes[4] = {data.at(0), data.at(1), data.at(2), data.at(3)};
    return (*((uint32_t *)bytes));

    // Big endian
  } else if (type == U1) { // unsigned int 8
    return ((uint32_t)((uint8_t)data.at(0)));
  } else if (type == U2) { // unsigned int 16
    char bytes[2] = {data.at(1), data.at(0)};
    return ((uint32_t) * ((uint16_t *)bytes));
  } else if (type == U4) { // unsigned int 32
    char bytes[4] = {data.at(3), data.at(2), data.at(1), data.at(0)};
    return (*((uint32_t *)bytes));
  }
  return 0;
}

void PlotData::addPoint(QByteArrayList data) {
  Q_ASSERT(!data.isEmpty());
  if (data.length() > ANALOG_COUNT) {
    QByteArray message = QString::number(data.length() - 1).toUtf8();
    sendMessageIfAllowed(tr("Too many channels in point (missing ';' ?)").toUtf8(), message, MessageLevel::error);
    return;
  }
  bool isok;
  double time;
  if (data.at(0).isEmpty()) {
    if (qIsInf(lastTimes[0]))
      time = 0;
    else
      time = lastTimes[0] + defaultTimestep;
  } else {
    time = arrayToDouble(data[0], isok);

    if (!isok) {
      sendMessageIfAllowed(tr("Can not parse points time").toUtf8(), data[0], MessageLevel::error);
      return;
    }
  }
  for (int ch = 1; ch < data.length(); ch++) {
    if (data.at(ch).at(0) != 'b' && data.at(ch).at(0) != 'B') {
      // Analog
      double value = arrayToDouble(data[ch], isok);
      if (!isok) {
        sendMessageIfAllowed(tr("Can not parse points value").toUtf8(), data[ch], MessageLevel::error);
        return;
      }

      emit addPointToPlot(ch - 1, time, value, time >= lastTimes[ch - 1]);

    } else {
      // Logika
      int bits = (data.at(ch).mid(1, 1).toUInt(&isok, 16)) * 8;
      if (!isok) {
        sendMessageIfAllowed(tr("Invalid logic bytes").toUtf8(), data[ch], MessageLevel::error);
        return;
      }

      if (data.at(ch).at(0) == 'b')
        data[ch].replace(0, 1, "u");
      else if (data.at(ch).at(0) == 'B')
        data[ch].replace(0, 1, "U");

      if (logicBits[ch - 1] > 0)
        bits = logicBits[ch - 1];

      uint32_t digitalValue = getBits(data.at(ch).mid(2), getType(data.at(ch).left(2)));
      for (uint8_t bit = 0; bit < bits; bit++)
        emit addPointToPlot(GlobalFunctions::getLogicChannelID(ch - 1, bit), time, (double)((bool)((digitalValue) & ((uint32_t)1 << (bit)))) + bit * 3, time >= lastTimes[ch - 1]);
    }
    lastTimes[ch - 1] = time;
  }
  if (debugLevel == OutputLevel::info)
    emit sendMessage(tr("Received point").toUtf8(), tr("%1 channels").arg(data.length() - 1).toUtf8(), MessageLevel::info);
}

void PlotData::addChannel(QByteArray data, unsigned int ch, QByteArray timeRaw, int bits, QByteArray min, QByteArray max) {
  // Zjistí datový typ vstupu
  ValueType::enumerator type = getType(data.left(2));
  if (type == unrecognised) {
    return;
  }

  // Převede časový interval na číslo
  bool isok;
  double timeStep = arrayToDouble(timeRaw, isok);
  if (!isok) {
    sendMessageIfAllowed(tr("Can not parse channel time step").toUtf8(), timeRaw, MessageLevel::error);
    return;
  }

  // Přemapování provede poud je vyplněno max
  bool remap = !max.isEmpty();

  // Převede minimální hodnotu na číslo na číslo
  double minimum = 0;
  if (!min.isEmpty()) {
    if (!remap)
      sendMessageIfAllowed(tr("Minumum value is stated, but maximum is not").toUtf8(), "Value will not be remaped!", MessageLevel::warning);
    minimum = arrayToDouble(min, isok);
    if (!isok) {
      sendMessageIfAllowed(tr("Can not parse minimum value").toUtf8(), min, MessageLevel::error);
      return;
    }
  }

  // Převede minimální hodnotu na číslo na číslo
  double maximum = 0;
  if (remap || !min.isEmpty()) {
    maximum = arrayToDouble(max, isok);
    if (!isok) {
      sendMessageIfAllowed(tr("Can not parse maximum value").toUtf8(), max, MessageLevel::error);
      return;
    }
  }

  // Zjistí počet bajtů na hodnotu a z dat odebere identifikátor typu
  uint8_t bytesPerValue = data.mid(1, 1).toUInt(nullptr, 16);
  data.remove(0, 2);

  // Informace o přijatém kanálu
  if (debugLevel == OutputLevel::info) {
    QByteArray message = tr("%1 samples, time step %2, %3 bits").arg(data.length() / bytesPerValue).arg(timeStep).arg(bits).toUtf8();
    if (remap)
      message.append(tr(", from %1 to %2").arg(minimum).arg(maximum).toUtf8());
    emit sendMessage(tr("Received channel %1").arg(ch).toUtf8(), message, MessageLevel::info);
  }

  bool showAsLogic = digitalChannel[ch - 1] > 0;
  if (showAsLogic && type != u1 && type != u2 && type != u3 && type != u4 && type != U1 && type != U2 && type != U3 && type != U4) {
    showAsLogic = false;
    sendMessageIfAllowed(tr("Can not show channel %1 as logic").arg(ch), tr("digital mode is only available for unsigned integer data type").toUtf8(), MessageLevel::warning);
  }

  double remapMultiple = 1;
  if (remap)
    remapMultiple = (maximum - minimum) / (1 << bits);

  // Vektory se pošlou jako pointer, graf je po zpracování smaže.
  auto analogData = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
  QVector<double> times;
  QVector<uint32_t> valuesDigital;
  QCPGraphData point;
  for (uint32_t i = 0; i < (uint32_t)data.length(); i += bytesPerValue) {
    times.append((i / bytesPerValue) * timeStep);
    point.key = times.last();
    if (remap)
      point.value = minimum + getValue(data.mid(i, bytesPerValue), type) * remapMultiple;
    else
      point.value = getValue(data.mid(i, bytesPerValue), type);
    if (showAsLogic)
      valuesDigital.append(getBits(data.mid(i, bytesPerValue), type));
    analogData->add(point);
  }

  // Pošle kanál do grafu

  emit addVectorToPlot(ch - 1, analogData);

  if (showAsLogic) {
    if (logicBits[ch - 1] > 0)
      bits = logicBits[ch - 1];
    // Pošle do grafu logický kanál
    int logicGroup = digitalChannel[ch - 1];
    QVector<QSharedPointer<QCPGraphDataContainer>> digitalChannels;
    for (uint8_t bit = 0; bit < bits; bit++)
      digitalChannels.append(QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer));
    for (int i = 0; i < valuesDigital.length(); i++)
      for (uint8_t bit = 0; bit < bits; bit++)
        digitalChannels.at(bit)->add(QCPGraphData(times.at(i), ((bool)((valuesDigital.at(i)) & ((uint32_t)1 << (bit)))) + bit * 3));

    for (uint8_t bit = 0; bit < bits; bit++) {
      emit addVectorToPlot(GlobalFunctions::getLogicChannelID(logicGroup - 1, bit), digitalChannels.at(bit));
    }
    emit clearLogic(logicGroup - 1, bits);
  }
}

void PlotData::reset() {
  for (int i = 0; i < ANALOG_COUNT; i++)
    lastTimes[i] = INFINITY;
}

void PlotData::setDigitalChannel(int chid, int target) {
  if (target == 0)
    emit clearLogic(digitalChannel[chid] - 1, 0);
  digitalChannel[chid] = target;
}

void PlotData::sendMessageIfAllowed(QString header, QByteArray message, MessageLevel::enumerator type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message, type);
}
