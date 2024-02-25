//  Copyright (C) 2020-2024  Jiří Maier

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

#include "messagemodel.h"

MessageModel::MessageModel(QObject *parent) : QAbstractListModel(parent) {}

int MessageModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return 0;

  return m_messages.count();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();

  const Message &message = m_messages[index.row()];

  switch (role) {
  case TimeRole:
    return message.time.toString("HH:mm:ss");
  case MessageRole:
    return message.message;
  case TypeRole:
    return message.type;
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> MessageModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[TimeRole] = "time";
  roles[MessageRole] = "message";
  roles[TypeRole] = "type";
  return roles;
}

void MessageModel::addMessage(const QString &message, QChar type) {
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  m_messages.append({message, QTime::currentTime(), type});
  endInsertRows();
}

void MessageModel::clear() {
  beginResetModel();
  m_messages.clear();
  endResetModel();
}
