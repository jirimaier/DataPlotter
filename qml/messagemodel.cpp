#include "messagemodel.h"

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_messages.count();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
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

QHash<int, QByteArray> MessageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TimeRole] = "time";
    roles[MessageRole] = "message";
    roles[TypeRole] = "type";
    return roles;
}

void MessageModel::addMessage(const QString &message, QChar type)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_messages.append({ message, QTime::currentTime(), type });
    endInsertRows();
}

void MessageModel::clear()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}
