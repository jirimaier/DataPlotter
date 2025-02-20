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
#include "global.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVersionNumber>
#include <qcoreapplication.h>

UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent) {
  if (QSslSocket::sslLibraryVersionString().isEmpty()) {
    qDebug() << "OpenSSL dll not found (will not check for updates)";
    canCheckForUpdates = false;
  } else
    qDebug() << "Using OpenSSL: " << QSslSocket::sslLibraryVersionString();
}

void UpdateChecker::checkForUpdates(bool showOnlyPositiveResult) {
  if (!canCheckForUpdates)
    return;
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
    reply->deleteLater();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QJsonObject obj = doc.object();

    QString latestVersionTag = obj.value("tag_name").toString();
    latestVersionTag.remove("v", Qt::CaseInsensitive);

    auto latestVersion = QVersionNumber::fromString(latestVersionTag).normalized();
    auto appVersion = QVersionNumber::fromString(QCoreApplication::applicationVersion()).normalized();

    if (latestVersion > appVersion) {
      emit checkedVersion(true, tr("New version available: %1\n Current version: %2").arg(latestVersion.toString(), appVersion.toString()));
      return;
    } else if (!onlyPosRes) {
      if (latestVersion == appVersion)
        emit checkedVersion(false, tr("You have the latest version (%1).").arg(appVersion.toString()));
      else
        emit checkedVersion(false, tr("Your version (%1) is higher than latest official release (%2).").arg(appVersion.toString(), latestVersion.toString()));
    }

  } else if (!onlyPosRes)
    emit checkedVersion(false, tr("Version check failed."));
}
