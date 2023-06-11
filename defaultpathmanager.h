
#ifndef DEFAULTPATHMANAGER_H
#define DEFAULTPATHMANAGER_H

#include "qstandardpaths.h"
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

  QString requestOpenFile(QWidget *dialogParent, QString title, QString pathID, QString extensions) {
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

  QString requestSaveFile(QWidget *dialogParent, QString title, QString pathID, QString suggestedName, QString extensions) {
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
      msgBox.setInformativeText(QObject::tr("This may be because file is opened in another program. Or the application does not have permittion to write to this directory."));
      msgBox.setDetailedText(fileName);
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      return "";
    }
    file.close();

    defaultPaths[pathID] = folderPath;

    return fileName;
  }
};

#endif // DEFAULTPATHMANAGER_H
