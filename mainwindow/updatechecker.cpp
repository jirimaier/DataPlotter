#include "updatechecker.h"
#include "mainwindow/version.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent) { qDebug() << "Using open SSL: " << QSslSocket::sslLibraryVersionString(); }

void UpdateChecker::checkForUpdates(bool showOnlyPositiveResult) {
  onlyPosRes = showOnlyPositiveResult;
  QString apiUrl = UpdatesApi;

  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  connect(manager, &QNetworkAccessManager::finished, this, &UpdateChecker::onRequestFinished);

  QNetworkRequest request;
  request.setUrl(QUrl(apiUrl));
  manager->get(request);
}

void UpdateChecker::onRequestFinished(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QJsonObject obj = doc.object();

    QString latestVersion = obj.value("tag_name").toString();
    latestVersion.remove("v", Qt::CaseInsensitive);
    QList<QString> version = latestVersion.split('.');

    int ApplicationVersion_length = sizeof(ApplicationVersion) / sizeof(ApplicationVersion[0]);
    for (int i = 0; i < ApplicationVersion_length; i++) {
      int gitVersion = version.length() >= i ? version.at(i).toInt() : 0;
      int curVersion = ApplicationVersion_length >= i ? ApplicationVersion[i] : 0;

      if (gitVersion > curVersion) {
        emit checkedVersion(true, tr("New version available: %1").arg(latestVersion));
        return;
      } else if (gitVersion < curVersion) {
        if (!onlyPosRes)
          emit checkedVersion(false, tr("Your version is higher than latest official release."));
        return;
      }
    }
    if (!onlyPosRes)
      emit checkedVersion(false, tr("You have the latest version."));
    return;
    reply->deleteLater();
  } else if (!onlyPosRes)
    emit checkedVersion(false, tr("Version check failed."));
}
