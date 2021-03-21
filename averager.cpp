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
    count[i] = 8;
  lists.resize(ANALOG_COUNT);
  results.resize(ANALOG_COUNT);
}

void Averager::reset() {
  for (int i = 0; i < ANALOG_COUNT; i++) {
    lists[i].clear();
    results[i].clear();
  }
}

void Averager::setCount(int chID, int count) {
  this->count[chID] = count;

  while (lists.at(chID).length() > count) {
    for (int i = 0; i < results[chID].size(); i++) {
      results[chID][i] -= lists[chID].first().at(i);
    }
    lists[chID].removeFirst();
  }
}

void Averager::newDataVector(int chID, double timeStep, QSharedPointer<QCPGraphDataContainer> data) {
  if (data->size() != results.at(chID).size() || timeStep != lastTimeStep[chID]) {
    results[chID].clear();
    lists[chID].clear();
  }

  lastTimeStep[chID] = timeStep;

  if (results.at(chID).isEmpty()) {
    results[chID].resize(data->size());
  }

  lists[chID].append(QVector<double>());

  for (auto it = data->begin(); it != data->end(); it++) {
    lists[chID].last().append(it->value);
  }

  if (lists[chID].size() == count[chID] + 1) {
    for (int i = 0; i < data->size(); i++) {
      results[chID][i] -= lists[chID].first().at(i);
    }
    lists[chID].removeFirst();
  }

  Q_ASSERT(lists[chID].size() <= count[chID]);

  for (int i = 0; i < data->size(); i++) {
    results[chID][i] += data->at(i)->value;
  }

  for (auto it = data->begin(); it != data->end(); it++) {
    it->value = results[chID][it - data->begin()];
    it->value /= lists[chID].size();
  }

  emit addVectorToPlot(chID, data);
}
