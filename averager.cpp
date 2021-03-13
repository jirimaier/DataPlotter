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

}

void Averager::reset(int ch, bool enabled) {
  Q_UNUSED(enabled)
  list.clear();
  channel = ch;
}

void Averager::setCount(int count) {
  this->count = count;

  while (list.length() > count)
    list.removeLast();
}

void Averager::newDataVector(QSharedPointer<QCPGraphDataContainer> data) {

  if (!list.isEmpty()) {
    if (list.first().size() != data->size())
      list.clear();
  }

  list.push_front(*data);

  if (list.length() > count)
    list.removeLast();

  QVector<double> values;
  values.resize(data->size());

  for (auto it = list.begin(); it != list.end(); it++) {
    for (int i = 0; i < data->size(); i++)
      values[i] += it->at(i)->value;
  }

  auto result = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());

  for (int i = 0; i < data->size(); i++)
    result->add(QCPGraphData(data->at(i)->key, values.at(i) / list.size()));

  emit addVectorToPlot(channel - 1, result, false);
}
