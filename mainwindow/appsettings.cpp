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

#include "appsettings.h"
#include "defaultpathmanager.h"
#include "mainwindow/mainwindow.h"
#include "ui_developeroptions.h"

AppSettings::AppSettings(MainWindow *parent) : QObject{parent} {
  mainwindow = parent; // Range
  setables["vrange"] = {mainwindow->ui->doubleSpinBoxRangeVerticalRange, false};
  setables["hrange"] = {mainwindow->ui->doubleSpinBoxRangeHorizontal, false};
  setables["vcenter"] = {mainwindow->ui->doubleSpinBoxChOffset, false};

  // Plot settings
  setables["vaxis"] = {mainwindow->ui->checkBoxVerticalValues, false};
  setables["haxis"] = {mainwindow->ui->checkBoxHorizontalValues, false};
  setables["hlabel"] = {mainwindow->ui->lineEditHtitle, false};
  setables["hunit"] = {mainwindow->ui->lineEditHUnit, false};
  setables["vlabel"] = {mainwindow->ui->lineEditVtitle, false};
  setables["vunit"] = {mainwindow->ui->lineEditVUnit, false};

  setables["opengl"] = {mainwindow->ui->checkBoxOpenGL, true};
  setables["filter"] = {mainwindow->ui->comboBoxFIR, true};

  // Settings
  setables["clearonrec"] = {mainwindow->developerOptions->getUi()->checkBoxClearOnReconnect, true};
  setables["rstcmd"] = {mainwindow->developerOptions->getUi()->lineEditResetCmd, true};
  setables["autoautoset"] = {mainwindow->developerOptions->getUi()->checkBoxAutoAutoSet, true};
  setables["nofreeze"] = {mainwindow->developerOptions->getUi()->checkBoxFreezeSafe, true};
}

void AppSettings::applyGuiElementSettings(QWidget *target, QString value) {
  if (QDoubleSpinBox *newTarget = dynamic_cast<QDoubleSpinBox *>(target))
    newTarget->setValue(value.toDouble());
  else if (QComboBox *newTarget = dynamic_cast<QComboBox *>(target))
    newTarget->setCurrentIndex(value.toUInt());
  else if (QSpinBox *newTarget = dynamic_cast<QSpinBox *>(target))
    newTarget->setValue(value.toInt());
  else if (QScrollBar *newTarget = dynamic_cast<QScrollBar *>(target))
    newTarget->setValue(value.toInt());
  else if (QSlider *newTarget = dynamic_cast<QSlider *>(target))
    newTarget->setValue(value.toInt());
  else if (QCheckBox *newTarget = dynamic_cast<QCheckBox *>(target))
    newTarget->setChecked((bool)value.toUInt());
  else if (QPushButton *newTarget = dynamic_cast<QPushButton *>(target))
    newTarget->setChecked((bool)value.toUInt());
  else if (QDial *newTarget = dynamic_cast<QDial *>(target))
    newTarget->setValue(value.toInt());
  else if (QLineEdit *newTarget = dynamic_cast<QLineEdit *>(target))
    newTarget->setText(value);
}

QByteArray AppSettings::readGuiElementSettings(QWidget *target) {
  if (QDoubleSpinBox *newTarget = dynamic_cast<QDoubleSpinBox *>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QSpinBox *newTarget = dynamic_cast<QSpinBox *>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QScrollBar *newTarget = dynamic_cast<QScrollBar *>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QSlider *newTarget = dynamic_cast<QSlider *>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QCheckBox *newTarget = dynamic_cast<QCheckBox *>(target))
    return (newTarget->isChecked() ? "1" : "0");
  if (QPushButton *newTarget = dynamic_cast<QPushButton *>(target))
    return (newTarget->isChecked() ? "1" : "0");
  if (QComboBox *newTarget = dynamic_cast<QComboBox *>(target))
    return (QString::number(newTarget->currentIndex()).toUtf8());
  if (QDial *newTarget = dynamic_cast<QDial *>(target))
    return (QString::number(newTarget->value()).toUtf8());
  else if (QLineEdit *newTarget = dynamic_cast<QLineEdit *>(target))
    return (newTarget->text().toUtf8());
  return "";
}

QByteArray AppSettings::getSettings() {
  QByteArray settings;

  for (auto it = setables.begin(); it != setables.end(); it++)
    if (it.value().second == true)
      settings.append(QString(it.key() + ':' + readGuiElementSettings(it.value().first) + ";\n").toUtf8());

  settings.append(QString("baud:%1;\n").arg(mainwindow->ui->comboBoxBaud->currentText().toUInt()).toLocal8Bit());

  if (mainwindow->developerOptions->getUi()->checkBoxTriggerLineEn->checkState() == Qt::Checked)
    settings.append("trigline:on;\n");
  else if (mainwindow->developerOptions->getUi()->checkBoxTriggerLineEn->checkState() == Qt::Unchecked)
    settings.append("trigline:off;\n");
  else if (mainwindow->developerOptions->getUi()->checkBoxTriggerLineEn->checkState() == Qt::PartiallyChecked)
    settings.append("trigline:auto;\n");

  if (!recommendOpenGL)
    settings.append("noopengldialog;\n");

  if (checkForUpdatesAtStartup)
    settings.append("checkforupdates;\n");

  settings.append(mainwindow->ui->radioButtonEn->isChecked() ? "lang:en" : "lang:cz");
  settings.append(";\n");
  settings.append(mainwindow->ui->radioButtonCSVDot->isChecked() ? "csvsep:dc" : "csvsep:cs");
  settings.append(";\n");
  settings.append(mainwindow->ui->radioButtonLight->isChecked() ? "theme:light" : "theme:dark");
  settings.append(";\n");

  QColor xyclr1 = mainwindow->ui->plotxy->getClr1();
  QColor xyclr2 = mainwindow->ui->plotxy->getClr2();
  settings.append(QString("xyclr:%1,%2,%3,%4,%5,%6").arg(xyclr1.red()).arg(xyclr1.green()).arg(xyclr1.blue()).arg(xyclr2.red()).arg(xyclr2.green()).arg(xyclr2.blue()).toUtf8());
  settings.append(";\n");

  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
    settings.append("ch:" + QString::number(i + 1).toUtf8());
    QColor clr1 = mainwindow->ui->plot->getChColorForTheme(i, 1);
    QColor clr2 = mainwindow->ui->plot->getChColorForTheme(i, 2);
    settings.append(QString(":clr:%1,%2,%3,%4,%5,%6").arg(clr1.red()).arg(clr1.green()).arg(clr1.blue()).arg(clr2.red()).arg(clr2.green()).arg(clr2.blue()).toUtf8());
    settings.append(";\n");
  }

  for (int i = 0; i < LOGIC_GROUPS; i++) {
    settings.append("log:" + QString::number(i + 1).toUtf8());
    QColor clr = mainwindow->ui->plot->getLogicColor(i);
    settings.append(QString(":clr:%1,%2,%3").arg(clr.red()).arg(clr.green()).arg(clr.blue()).toUtf8());
    settings.append(";\n");
  }

  auto defaultPaths = DefaultPathManager::getInstance().get();
  for (auto it = defaultPaths.begin(); it != defaultPaths.end(); it++)
    settings.append(QString("%1:%2;\n").arg(it.key(), it.value()).toUtf8());

  return settings;
}

void AppSettings::useSettings(QByteArray settings, MessageTarget::enumMessageTarget source) {
  settings.replace('\n', "");
  settings.replace('\r', "");

  if (settings == "clearlog") {
    mainwindow->ui->plot->clearLogicGroup(2, 0);
  }

  else if (settings == "noopengldialog") {
    recommendOpenGL = false;
  }

  else if (settings == "checkforupdates") {
    checkForUpdatesAtStartup = true;
    mainwindow->updateChecker.checkForUpdates(true);
  }

  else {
    if (!settings.contains(':')) {
      if (source == MessageTarget::manual || mainwindow->ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
        mainwindow->printMessage(tr("Invalid settings").toUtf8(), settings, MessageLevel::error, source);
      return;
    }
    QByteArray type = settings.left(settings.indexOf(':', 0));
    QByteArray value = settings.mid(settings.indexOf(':', 0) + 1);
    type = type.simplified().toLower();

    if (setables.contains(type))
      applyGuiElementSettings(setables[type].first, value);

    else if (type.startsWith("path_"))
      DefaultPathManager::getInstance().add(type, value);

    else if (type == "baud") {
      mainwindow->ui->comboBoxBaud->setEditText(QString::number(value.toUInt()));
    }

    else if (type == "trigline") {
      if (value == "on")
        mainwindow->developerOptions->getUi()->checkBoxTriggerLineEn->setCheckState(Qt::Checked);
      if (value == "off")
        mainwindow->developerOptions->getUi()->checkBoxTriggerLineEn->setCheckState(Qt::Unchecked);
      if (value == "auto")
        mainwindow->developerOptions->getUi()->checkBoxTriggerLineEn->setCheckState(Qt::PartiallyChecked);
    }

    else if (type == "trigch") {
      int chid = value.toUInt() - 1;
      if (chid < 0)
        chid = 0;
      if (chid >= ANALOG_COUNT)
        chid = ANALOG_COUNT - 1;
      mainwindow->ui->plot->setTriggerLineChannel(chid);
      if (mainwindow->developerOptions->getUi()->checkBoxTriggerLineEn->checkState() == Qt::PartiallyChecked) {
        mainwindow->ui->plot->setTriggerLineVisible(true);
        mainwindow->triggerLineTimer.start();
      }
    }

    else if (type == "trigpos") {
      mainwindow->ui->plot->setTriggerLineValue(value.toDouble());
      if (mainwindow->developerOptions->getUi()->checkBoxTriggerLineEn->checkState() == Qt::PartiallyChecked) {
        mainwindow->ui->plot->setTriggerLineVisible(true);
        mainwindow->triggerLineTimer.start();
      }
    }

    else if (type == "lang") {
      if (value == "en")
        mainwindow->ui->radioButtonEn->setChecked(true);
      if (value == "cz")
        mainwindow->ui->radioButtonCz->setChecked(true);
    }

    else if (type == "theme") {
      if (value == "light")
        mainwindow->ui->radioButtonLight->setChecked(true);
      if (value == "dark")
        mainwindow->ui->radioButtonDark->setChecked(true);
    }

    else if (type == "csvsep") {
      if (value == "cs")
        mainwindow->ui->radioButtonCSVComma->setChecked(true);
      if (value == "dc")
        mainwindow->ui->radioButtonCSVDot->setChecked(true);
    }

    else if (type == "noclickclr" || type == "clickclr") {
      QByteArrayList list = value.replace('.', ';').split(',');
      mainwindow->developerOptions->getUi()->listWidgetTerminalBlacklist->clear();
      foreach (QByteArray item, list) {
        if (!mainwindow->developerOptions->addColorToBlacklist(item))
          mainwindow->printMessage(tr("Invalid color-code").toUtf8(), item, MessageLevel::error, source);
      }

      mainwindow->developerOptions->getUi()->comboBoxTerminalColorListMode->setCurrentIndex(type == "noclickclr" ? 0 : 1);
      mainwindow->developerOptions->updateColorBlacklist();
    }

    else if (type == "clearch") {
      mainwindow->ui->plot->clearCh(value.toUInt() - 1);
    }

    else if (type == "xyclr") {
      QByteArrayList rgb = value.split(',');
      if (rgb.length() != 3 && rgb.length() != 6) {
        if (source == MessageTarget::manual || mainwindow->ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
          mainwindow->printMessage(tr("Invalid color").toUtf8(), settings, MessageLevel::error, source);
        return;
      }
      QColor clr1 = QColor(rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt());
      QColor clr2 = clr1;
      if (rgb.length() == 6)
        clr2 = QColor(rgb.at(3).toInt(), rgb.at(4).toInt(), rgb.at(5).toInt());
      mainwindow->ui->plotxy->setColor(clr1, 1);
      mainwindow->ui->plotxy->setColor(clr2, 2);
    }

    else if (type == "ch") {
      int ch = value.left(value.indexOf(':', 0)).toUInt();
      if (ch > ANALOG_COUNT + MATH_COUNT || ch == 0) {
        if (source == MessageTarget::manual || mainwindow->ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
          mainwindow->printMessage(tr("Invalid channel in settings").toUtf8(), QString::number(ch).toUtf8(), MessageLevel::error, source);
        return;
      }
      // V nastavení se čísluje od jedné, ale v příkazech od nuly
      ch = ch - 1;
      QByteArray subtype = value.mid(value.indexOf(':', 0) + 1).toLower();
      subtype = subtype.left(subtype.indexOf(':'));
      QByteArray subvalue = value.mid(value.lastIndexOf(':') + 1);

      if (subtype == "sty")
        mainwindow->ui->plot->setChStyle(ch, subvalue.toUInt());
      else if (subtype == "clr") {
        QByteArrayList rgb = subvalue.mid(subvalue.indexOf(':')).split(',');
        if (rgb.length() != 3 && rgb.length() != 6) {
          if (source == MessageTarget::manual || mainwindow->ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
            mainwindow->printMessage(tr("Invalid color").toUtf8(), settings, MessageLevel::error, source);
          return;
        }
        QColor clr1 = QColor(rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt());
        QColor clr2 = clr1;
        if (rgb.length() == 6)
          clr2 = QColor(rgb.at(3).toInt(), rgb.at(4).toInt(), rgb.at(5).toInt());
        mainwindow->ui->plot->setChColor(ch, clr1, 1);
        mainwindow->ui->plot->setChColor(ch, clr2, 2);
        mainwindow->colorUpdateNeeded = true;
      }
      if (mainwindow->ui->comboBoxSelectedChannel->currentIndex() == ch)
        mainwindow->on_comboBoxSelectedChannel_currentIndexChanged(mainwindow->ui->comboBoxSelectedChannel->currentIndex());
    }

    else if (type == "log") {
      int group = value.left(value.indexOf(':', 0)).toUInt() - 1;
      if (group >= LOGIC_GROUPS) {
        if (source == MessageTarget::manual || mainwindow->ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
          mainwindow->printMessage(tr("Invalid logic in settings").toUtf8(), QString::number(group).toUtf8(), MessageLevel::error, source);
        return;
      }
      QByteArray subtype = value.mid(value.indexOf(':', 0) + 1).toLower();
      subtype = subtype.left(subtype.indexOf(':'));
      QByteArray subvalue = value.mid(value.lastIndexOf(':') + 1);

      if (subtype == "sty")
        mainwindow->ui->plot->setLogicStyle(group, subvalue.toUInt());
      else if (subtype == "clr") {
        QByteArrayList rgb = subvalue.mid(subvalue.indexOf(':')).split(',');
        if (rgb.length() != 3 && rgb.length() != 6) {
          if (source == MessageTarget::manual || mainwindow->ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
            dynamic_cast<MainWindow *>(parent())->printMessage(tr("Invalid color").toUtf8(), settings, MessageLevel::error, source);
          return;
        }
        QColor clr1 = QColor(rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt());
        QColor clr2 = clr1;
        if (rgb.length() == 6)
          clr2 = QColor(rgb.at(3).toInt(), rgb.at(4).toInt(), rgb.at(5).toInt());
        mainwindow->ui->plot->setLogicColor(group, clr1, 1);
        mainwindow->ui->plot->setLogicColor(group, clr2, 2);
        mainwindow->colorUpdateNeeded = true;
      }
      if (mainwindow->ui->comboBoxSelectedChannel->currentIndex() == group + ANALOG_COUNT + MATH_COUNT)
        dynamic_cast<MainWindow *>(parent())->on_comboBoxSelectedChannel_currentIndexChanged(mainwindow->ui->comboBoxSelectedChannel->currentIndex());
    }

    // Error
    else {
      if (source == MessageTarget::manual || mainwindow->ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
        mainwindow->printMessage(tr("Unknown setting").toUtf8(), type, MessageLevel::error, source);
      return;
    }
  }
  if (source == MessageTarget::manual || mainwindow->ui->comboBoxOutputLevel->currentIndex() == MessageLevel::info)
    mainwindow->printMessage(tr("Applied settings").toUtf8(), settings, MessageLevel::info, source);
}

void AppSettings::resetSettings() {
  QFile defaults(":/text/settings/default.cfg");
  if (defaults.open(QFile::ReadOnly | QFile::Text)) {
    emit mainwindow->sendManualInput("$$S" + defaults.readAll().replace("\n", "") + "$$U");
    defaults.close();
  }
  saveSettings();
}

void AppSettings::loadSettings() {
  QFile settingsFile(mainwindow->configFilePath);
  if (settingsFile.open(QFile::ReadOnly | QFile::Text)) {
    emit mainwindow->sendManualInput("$$S" + settingsFile.readAll().replace("\n", "") + "$$U");
    settingsFile.close();
  } else {
    mainwindow->on_pushButtonReset_clicked();
  }
}

void AppSettings::saveSettings() {
  QDir configDir(QFileInfo(mainwindow->configFilePath).absolutePath());
  if (!configDir.exists()) {
    if (!configDir.mkpath(".")) {
      qWarning() << "Failed to create configuration directory";
    }
  }

  QFile file(mainwindow->configFilePath);
  if (file.open(QFile::WriteOnly | QFile::Truncate)) {
    file.write(getSettings());
    file.close();
  } else {
    qWarning() << "Unable to save settings";
  }
}
