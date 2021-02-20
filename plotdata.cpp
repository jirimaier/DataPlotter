//  Copyright (C) 2020-2021  Jiří Maier

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "plotdata.h"

PlotData::PlotData(QObject *parent) : QObject(parent) {
  for (int i = 0; i < LOGIC_GROUPS; i++) {
    logicTargets[i] = -1;
    logicBits[i] = 0;
  }
  for (int i = 0; i < MATH_COUNT; i++) {
    mathFirsts[i] = 0;
    mathSeconds[i] = 0;
  }
  reset();
}

PlotData::~PlotData() {}

double PlotData::getValue(QPair<ValueType, QByteArray> value, bool &isok) {
  if (!value.first.isBinary) {
    return value.second.toDouble(&isok);
  }
  isok = true;
  if (value.first.bigEndian) {
    // Big endian
    if (value.first.type == ValueType::Type::unsignedint) {
      if (value.first.bytes == 1) { // unsigned int 8
        return (double)((uint8_t)value.second.at(0)) * value.first.multiplier;
      } else if (value.first.bytes == 2) { // unsigned int 16
        char bytes[2] = {value.second.at(1), value.second.at(0)};
        return (double)*((uint16_t *)bytes) * value.first.multiplier;
      } else if (value.first.bytes == 3) { // unsigned int 32
        char bytes[4] = {value.second.at(2), value.second.at(1), value.second.at(0), 0};
        return (double)*((uint32_t *)bytes) * value.first.multiplier;
      } else if (value.first.bytes == 4) { // unsigned int 32
        char bytes[4] = {value.second.at(3), value.second.at(2), value.second.at(1), value.second.at(0)};
        return (double)*((uint32_t *)bytes) * value.first.multiplier;
      }
    } else if (value.first.type == ValueType::Type::integer) {
      if (value.first.bytes == 1) { // signed int 8
        return (double)((int8_t)value.second.at(0)) * value.first.multiplier;
      } else if (value.first.bytes == 2) { // signed int 16
        char bytes[2] = {value.second.at(1), value.second.at(0)};
        return (double)*((int16_t *)bytes) * value.first.multiplier;
      } else if (value.first.bytes == 4) { // signed int 32
        char bytes[4] = {value.second.at(3), value.second.at(2), value.second.at(1), value.second.at(0)};
        return (double)*((int32_t *)bytes) * value.first.multiplier;
      }
    } else if (value.first.type == ValueType::Type::floatingpoint) {
      if (value.first.bytes == 4) { // float
        char bytes[4] = {value.second.at(3), value.second.at(2), value.second.at(1), value.second.at(0)};
        return (double)*((float *)bytes) * value.first.multiplier;
      } else if (value.first.bytes == 8) { // double
        char bytes[8] = {value.second.at(7), value.second.at(6), value.second.at(5), value.second.at(4), value.second.at(3), value.second.at(2), value.second.at(1), value.second.at(0)};
        return (*((double *)bytes)) * value.first.multiplier;
      }
    }
  } else {
    if (value.first.type == ValueType::Type::unsignedint) {
      if (value.first.bytes == 1) {
        return (double)((uint8_t)value.second.at(0)) * value.first.multiplier;
      } else if (value.first.bytes == 2) { // unsigned int 16
        char bytes[2] = {value.second.at(0), value.second.at(1)};
        return (double)*((uint16_t *)bytes) * value.first.multiplier;
      } else if (value.first.bytes == 3) { // unsigned int 32
        char bytes[4] = {value.second.at(0), value.second.at(1), value.second.at(2), 0};
        return (double)*((uint32_t *)bytes) * value.first.multiplier;
      } else if (value.first.bytes == 4) { // unsigned int 32
        char bytes[4] = {value.second.at(0), value.second.at(1), value.second.at(2), value.second.at(3)};
        return (double)*((uint32_t *)bytes) * value.first.multiplier;
      }
    } else if (value.first.type == ValueType::Type::integer) {
      if (value.first.bytes == 1) { // signed int 8
        return (double)((int8_t)value.second.at(0)) * value.first.multiplier;
      } else if (value.first.bytes == 2) { // signed int 16
        char bytes[2] = {value.second.at(0), value.second.at(1)};
        return (double)*((int16_t *)bytes) * value.first.multiplier;
      } else if (value.first.bytes == 4) { // signed int 32
        char bytes[4] = {value.second.at(0), value.second.at(1), value.second.at(2), value.second.at(3)};
        return (double)*((int32_t *)bytes) * value.first.multiplier;
      }
    } else if (value.first.type == ValueType::Type::floatingpoint) {
      if (value.first.bytes == 4) { // float
        char bytes[4] = {value.second.at(0), value.second.at(1), value.second.at(2), value.second.at(3)};
        return (double)*((float *)bytes) * value.first.multiplier;
      } else if (value.first.bytes == 8) { // double
        char bytes[8] = {value.second.at(0), value.second.at(1), value.second.at(2), value.second.at(3), value.second.at(4), value.second.at(5), value.second.at(6), value.second.at(7)};
        return (*((double *)bytes)) * value.first.multiplier;
      }
    }
  }
  isok = false;
  return 0;
}

uint32_t PlotData::getBits(QPair<ValueType, QByteArray> value) {
  if (value.first.bigEndian) {
    // Big endian
    if (value.first.bytes == 1)
      return ((uint32_t)((uint8_t)value.second.at(0)));
    if (value.first.bytes == 2) {
      char bytes[2] = {value.second.at(1), value.second.at(0)};
      return ((uint32_t) * ((uint16_t *)bytes));
    }
    if (value.first.bytes == 3) {
      char bytes[4] = {value.second.at(2), value.second.at(1), value.second.at(0), 0};
      return (*((uint32_t *)bytes));
    }
    if (value.first.bytes == 4) {
      char bytes[4] = {value.second.at(3), value.second.at(2), value.second.at(1), value.second.at(0)};
      return (*((uint32_t *)bytes));
    }
  } else {
    // Little endian
    if (value.first.bytes == 1)
      return ((uint32_t)((uint8_t)value.second.at(0)));
    if (value.first.bytes == 2) {
      char bytes[2] = {value.second.at(0), value.second.at(1)};
      return ((uint32_t) * ((uint16_t *)bytes));
    }
    if (value.first.bytes == 3) {
      char bytes[4] = {value.second.at(0), value.second.at(1), value.second.at(2), 0};
      return (*((uint32_t *)bytes));
    }
    if (value.first.bytes == 4) {
      char bytes[4] = {value.second.at(0), value.second.at(1), value.second.at(2), value.second.at(3)};
      return (*((uint32_t *)bytes));
    }
  }
  return 0;
}

void PlotData::addPoint(QList<QPair<ValueType, QByteArray>> data) {
  if (data.length() > ANALOG_COUNT) {
    QByteArray message = QString::number(data.length() - 1).toUtf8();
    sendMessageIfAllowed(tr("Too many channels in point (missing ';' ?)").toUtf8(), message, MessageLevel::error);
    return;
  }
  bool isok;
  double time;
  if (data.at(0).second.isEmpty()) {
    if (qIsInf(lastTimes[0]))
      time = 0;
    else
      time = lastTimes[0] + defaultTimestep;
  } else {
    time = getValue(data.at(0), isok);
    if (!isok) {
      sendMessageIfAllowed(tr("Can not parse points time").toUtf8(), data.at(0).second, MessageLevel::error);
      return;
    }
  }
  for (unsigned int ch = 1; (int)ch < data.length(); ch++) {
    if (data.at(ch).second.isEmpty())
      continue;
    double value = getValue(data.at(ch), isok);
    if (!isok) {
      sendMessageIfAllowed(tr("Can not parse points value").toUtf8(), data.at(ch).second, MessageLevel::error);
      return;
    }

    bool isLogic = false;
    for (int i = 0; i < LOGIC_GROUPS; i++)
      if (logicTargets[i] == ch)
        isLogic = true;
    if (isLogic) {
      if (data.at(ch).first.type == ValueType::Type::unsignedint) {
        unsigned int bits = 8 * data.at(ch).first.bytes;
        QVector<double> digitalChannels;
        uint32_t digitalValue = getBits(data.at(ch));
        for (uint8_t bit = 0; bit < bits; bit++)
          digitalChannels.append((((bool)((digitalValue) & ((uint32_t)1 << (bit)))) + bit * 3));
        for (int logicGroup = 0; logicGroup < LOGIC_GROUPS; logicGroup++) {
          if (logicTargets[logicGroup] != ch)
            continue;
          if (logicBits[ch - 1] > 0 && logicBits[ch - 1] < bits)
            bits = logicBits[ch - 1];
          for (uint8_t bit = 0; bit < bits; bit++) {
            emit addPointToPlot(getLogicChannelID(logicGroup, bit), time, digitalChannels.at(bit), time >= lastTimes[ch - 1]);
          }
        }
      } else {
        sendMessageIfAllowed(tr("Can not show channel %1 as logic").arg(ch), tr("digital mode is only available for unsigned integer data type").toUtf8(), MessageLevel::warning);
      }
    }

    emit addPointToPlot(ch - 1, time, value, time >= lastTimes[ch - 1]);
    for (int math = 0; math < MATH_COUNT; math++) {
      if (mathFirsts[math] == ch) {
        auto point = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
        point->add(QCPGraphData(time, value));
        emit addMathData(math, true, point);
      }
      if (mathSeconds[math] == ch) {
        auto point = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
        point->add(QCPGraphData(time, value));
        emit addMathData(math, false, point);
      }
    }
    lastTimes[ch - 1] = time;
  }
  if (debugLevel == OutputLevel::info)
    emit sendMessage(tr("Received point").toUtf8(), tr("%1 channels").arg(data.length() - 1).toUtf8(), MessageLevel::info);
}

void PlotData::addChannel(QPair<ValueType, QByteArray> data, unsigned int ch, QPair<ValueType, QByteArray> timeRaw, int zeroIndex, int bits, QPair<ValueType, QByteArray> min, QPair<ValueType, QByteArray> max) {
  // Zjistí datový typ vstupu
  QByteArray typeID, numberBytes;

  // Převede časový interval na číslo
  bool isok;
  double timeStep = getValue(timeRaw, isok);
  if (!isok) {
    sendMessageIfAllowed(tr("Can not parse channel time step").toUtf8(), timeRaw.second, MessageLevel::error);
    return;
  }

  // Přemapování provede poud je vyplněno max
  bool remap = !max.second.isEmpty();

  // Převede minimální hodnotu na číslo na číslo
  double minimum = 0;
  if (!min.second.isEmpty()) {
    if (!remap)
      sendMessageIfAllowed(tr("Minumum value is stated, but maximum is not").toUtf8(), tr("Value will not be remaped!").toUtf8(), MessageLevel::warning);
    minimum = getValue(min, isok);
    if (!isok) {
      sendMessageIfAllowed(tr("Can not parse minimum value").toUtf8(), min.second, MessageLevel::error);
      return;
    }
  }

  // Převede minimální hodnotu na číslo na číslo
  double maximum = 0;
  if (remap || !min.second.isEmpty()) {
    maximum = getValue(max, isok);
    if (!isok) {
      sendMessageIfAllowed(tr("Can not parse maximum value").toUtf8(), max.second, MessageLevel::error);
      return;
    }
  }

  // Informace o přijatém kanálu
  if (debugLevel == OutputLevel::info) {
    QByteArray message = tr("%1 samples, sampling period %2s, %4 bits").arg(data.second.length() / data.first.bytes).arg(floatToNiceString(timeStep, 4, false, false)).arg(bits).toUtf8();
    if (remap)
      message.append(tr(", from %1 to %2").arg(minimum).arg(maximum).toUtf8());
    emit sendMessage(tr("Received channel %1").arg(ch).toUtf8(), message, MessageLevel::info);
  }

  bool isLogic = false;
  for (int i = 0; i < LOGIC_GROUPS; i++)
    if (logicTargets[i] == ch)
      isLogic = true;
  if (isLogic && data.first.type != ValueType::Type::unsignedint) {
    isLogic = false;
    sendMessageIfAllowed(tr("Can not show channel %1 as logic").arg(ch), tr("digital mode is only available for unsigned integer data type").toUtf8(), MessageLevel::warning);
  }

  if (remap)
    data.first.multiplier *= (maximum - minimum) / ((1 << bits) - 1);

  // Vektory se pošlou jako pointer, graf je po zpracování smaže.
  auto analogData = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer);
  QVector<double> times;
  QVector<uint32_t> valuesDigital;
  QCPGraphData point;
  for (int i = 0; i < data.second.length(); i += data.first.bytes) {
    times.append(((i / data.first.bytes) - zeroIndex) * timeStep);
    point.key = times.last();
    point.value = minimum + getValue(QPair<ValueType, QByteArray>(data.first, data.second.mid(i, data.first.bytes)), isok);
    if (isLogic)
      valuesDigital.append(getBits(QPair<ValueType, QByteArray>(data.first, data.second.mid(i, data.first.bytes))));
    analogData->add(point);
  }

  // Pošle kanál do grafu a případně do výpočtů
  for (int math = 0; math < MATH_COUNT; math++) {
    if (mathFirsts[math] == ch)
      emit addMathData(math, true, analogData);
    if (mathSeconds[math] == ch)
      emit addMathData(math, false, analogData);
  }
  // if (xyFirst == ch) emit addXYData(true, analogData);
  // if (xySecond == ch) emit addXYData(false, analogData);

  emit addVectorToPlot(ch - 1, analogData);

  if (isLogic) {
    // Pošle do grafu logický kanál
    QVector<QSharedPointer<QCPGraphDataContainer>> digitalChannels;
    for (uint8_t bit = 0; bit < bits; bit++)
      digitalChannels.append(QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer));
    for (int i = 0; i < valuesDigital.length(); i++)
      for (uint8_t bit = 0; bit < bits; bit++)
        digitalChannels.at(bit)->add(QCPGraphData(times.at(i), ((bool)((valuesDigital.at(i)) & ((uint32_t)1 << (bit)))) + bit * 3));

    for (int logicGroup = 0; logicGroup < LOGIC_GROUPS; logicGroup++) {
      if (logicTargets[logicGroup] != ch)
        continue;
      if (logicBits[ch - 1] > 0 && logicBits[ch - 1] < (unsigned int)bits)
        bits = logicBits[ch - 1];
      for (uint8_t bit = 0; bit < bits; bit++) {
        emit addVectorToPlot(getLogicChannelID(logicGroup, bit), digitalChannels.at(bit));
      }
    }
  }
}

void PlotData::reset() {
  for (int i = 0; i < ANALOG_COUNT; i++)
    lastTimes[i] = INFINITY;
}

void PlotData::setDigitalChannel(int logicGroup, int ch) {
  logicTargets[logicGroup - 1] = ch;
  emit clearLogic(logicGroup - 1, 0);
}

void PlotData::setLogicBits(int target, int bits) {
  logicBits[target - 1] = bits;
  emit clearLogic(target - 1, bits);
}

void PlotData::sendMessageIfAllowed(QString header, QByteArray message, MessageLevel::enumMessageLevel type) {
  if ((int)debugLevel >= (int)type)
    emit sendMessage(header, message, type);
}
