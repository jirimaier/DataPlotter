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

private:
  MainWindow *mainwindow;
  void applyGuiElementSettings(QWidget *target, QString value);
  QByteArray readGuiElementSettings(QWidget *target);
  QMap<QString, QPair<QWidget *, bool>> setables;

signals:
};

#endif // APPSETTINGS_H
