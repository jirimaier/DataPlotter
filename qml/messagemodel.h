#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include <QTime>

class MessageModel : public QAbstractListModel {
  Q_OBJECT

public:
  enum MessageRoles { TimeRole = Qt::UserRole + 1, MessageRole, TypeRole };

  explicit MessageModel(QObject *parent = nullptr);

  // Implement methods required by QAbstractListModel
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  // Add method to add messages to the model
  Q_INVOKABLE void addMessage(const QString &message, QChar type);

  // Add method to clear the messages in the model
  Q_INVOKABLE void clear();

private:
  struct Message {
    QString message;
    QTime time;
    QChar type;
  };

  QList<Message> m_messages;

  Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged FINAL)

  bool empty() const;

signals:
  void emptyChanged(bool newEmpty);
};

#endif // MESSAGEMODEL_H
