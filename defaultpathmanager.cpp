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

#include "defaultpathmanager.h"
#include "qstandardpaths.h"

DefaultPathManager::DefaultPathManager() {}

void DefaultPathManager::add(QString ID, QString path) { defaultPaths[ID] = path; }

QString DefaultPathManager::requestOpenFile(QWidget *dialogParent, QString title, QString pathID, QString extensions) {
  if (!defaultPaths.contains(pathID))
    defaultPaths[pathID] = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  QString fileName = QFileDialog::getOpenFileName(dialogParent, title, defaultPaths[pathID], extensions);
  if (fileName.isEmpty())
    return "";

  QFileInfo fileInfo(fileName);
  QString folderPath = fileInfo.absoluteDir().path();

  if (!fileInfo.exists()) {
    QMessageBox msgBox(dialogParent);
    msgBox.setText(QObject::tr("Cannot open file!"));
    msgBox.setDetailedText(fileName);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    return "";
  }

  defaultPaths[pathID] = folderPath;

  return fileName;
}

QString DefaultPathManager::requestSaveFile(QWidget *dialogParent, QString title, QString pathID, QString suggestedName, QString extensions) {
  if (!defaultPaths.contains(pathID))
    defaultPaths[pathID] = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  QString fileName = QFileDialog::getSaveFileName(dialogParent, title, defaultPaths[pathID] + suggestedName, extensions);
  if (fileName.isEmpty())
    return "";

  QFileInfo fileInfo(fileName);
  QString folderPath = fileInfo.absoluteDir().path();

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly)) {
    QMessageBox msgBox(dialogParent);
    msgBox.setText(QObject::tr("File is not writable!"));
    msgBox.setInformativeText(QObject::tr("This may be because file is opened in another program. Or the application "
                                          "does not have permittion to write to this directory."));
    msgBox.setDetailedText(fileName);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    return "";
  }
  file.close();

  defaultPaths[pathID] = folderPath;

  return fileName;
}
