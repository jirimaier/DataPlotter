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
