#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QNetworkReply>
#include <QObject>

class QNetworkAccessManager;

class UpdateChecker : public QObject {
  Q_OBJECT

public:
  explicit UpdateChecker(QObject *parent = nullptr);

  void checkForUpdates(bool showOnlyPositiveResult);

private slots:
  void onRequestFinished(QNetworkReply *reply);

private:
  QNetworkAccessManager *m_networkManager;
  bool onlyPosRes = false;

signals:
  void checkedVersion(bool isNew, QString message);
};

#endif // UPDATECHECKER_H
