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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "global.h"
#include <QObject>
#include <QWidget>

class MainWindow;

class AppSettings : public QObject {
  Q_OBJECT
public:
  explicit AppSettings(MainWindow *parent = nullptr);

  QByteArray getSettings();

  void useSettings(QByteArray settings, MessageTarget::enumMessageTarget source = MessageTarget::manual);

  void saveSettings();
  void saveToFile(QByteArray data);
  void loadSettings();
  void resetSettings();

  bool checkForUpdatesAtStartup = false;
  bool recommendOpenGL = true;

  static QString getPlatformInfo();
  static QString getPlatformInfoText();

private:
  MainWindow *mainwindow;
  void applyGuiElementSettings(QWidget *target, QString value);
  QByteArray readGuiElementSettings(QWidget *target);
  QMap<QString, QPair<QWidget *, bool>> setables;

signals:
};

#endif // APPSETTINGS_H
