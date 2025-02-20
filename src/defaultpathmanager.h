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

#ifndef DEFAULTPATHMANAGER_H
#define DEFAULTPATHMANAGER_H

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <qfile.h>

class DefaultPathManager {
  QMap<QString, QString> defaultPaths;
  explicit DefaultPathManager();

public:
  static DefaultPathManager &getInstance() {
    static DefaultPathManager instance; // Constructed on first use
    return instance;
  }

  void add(QString ID, QString path);
  const QMap<QString, QString> &get() const { return defaultPaths; }

  QString requestOpenFile(QWidget *dialogParent, QString title, QString pathID, QString extensions);

  QString requestSaveFile(QWidget *dialogParent, QString title, QString pathID, QString suggestedName, QString extensions);
};

#endif // DEFAULTPATHMANAGER_H
