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

#include "averager.h"

Averager::Averager(QObject* parent) : QObject(parent) {
  for (int i = 0; i < ANALOG_COUNT; i++)
    averageCount[i] = 8;
  listsChannels.resize(ANALOG_COUNT);
  channelsResults.resize(ANALOG_COUNT);
  points.resize(ANALOG_COUNT);
}

void Averager::reset() {
  for (int i = 0; i < ANALOG_COUNT; i++) {
    listsChannels[i].clear();
    channelsResults[i].clear();
    points[i].clear();
    pointResults[i] = 0;
  }
}

void Averager::setCount(int chID, int count) {
  this->averageCount[chID] = count;

  while (listsChannels.at(chID).length() > count) {
    for (int i = 0; i < channelsResults[chID].size(); i++) {
      channelsResults[chID][i] -= listsChannels[chID].first().at(i);
    }
    listsChannels[chID].removeFirst();
  }

  while (points.at(chID).length() > count) {
    pointResults[chID] -= points.at(chID).first().second;
    points[chID].removeFirst();
  }
}

void Averager::newDataVector(int chID, double timeStep, QSharedPointer<QCPGraphDataContainer> data) {
  if (data->size() != channelsResults.at(chID).size() || timeStep != lastChannelSamplingPeriod[chID]) {
    channelsResults[chID].clear();
    listsChannels[chID].clear();
  }

  lastChannelSamplingPeriod[chID] = timeStep;

  if (channelsResults.at(chID).isEmpty()) {
    channelsResults[chID].resize(data->size());
  }

  listsChannels[chID].append(QVector<double>());

  for (auto it = data->begin(); it != data->end(); it++) {
    listsChannels[chID].last().append(it->value);
  }

  if (listsChannels[chID].size() == averageCount[chID] + 1) {
    for (int i = 0; i < data->size(); i++) {
      channelsResults[chID][i] -= listsChannels[chID].first().at(i);
    }
    listsChannels[chID].removeFirst();
  }

  Q_ASSERT(listsChannels[chID].size() <= averageCount[chID]);

  for (int i = 0; i < data->size(); i++) {
    channelsResults[chID][i] += data->at(i)->value;
  }

  for (auto it = data->begin(); it != data->end(); it++) {
    it->value = channelsResults[chID][it - data->begin()];
    it->value /= listsChannels[chID].size();
  }

  emit addVectorToPlot(chID, data);
}

void Averager::newDataPoint(int chID, double time, double value, bool append) {
  if (!append) {
    points[chID].clear();
    pointResults[chID] = 0;
  }

  points[chID].append(QPair<double, double>(time, value));

  if (points[chID].size() == averageCount[chID] + 1) {
    pointResults[chID] -= points[chID].first().second;
    points[chID].removeFirst();
  }

  Q_ASSERT(listsChannels[chID].size() <= averageCount[chID]);

  pointResults[chID] += value;

  double midTime = (points.at(chID).first().first + points.at(chID).last().first) / 2.0;
  emit addPointToPlot(chID, midTime, pointResults[chID] / points.at(chID).size(), append);
}
