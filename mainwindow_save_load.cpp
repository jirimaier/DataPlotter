//  Copyright (C) 2020-2021  Jiří Maier

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

#include "mainwindow.h"

void MainWindow::initSetables() {
  // Range
  setables["vrange"] = ui->doubleSpinBoxRangeVerticalRange;
  setables["hrange"] = ui->doubleSpinBoxRangeHorizontal;
  setables["vpos"] = ui->sliderVerticalCenter;

  // Plot settings
  setables["vaxis"] = ui->checkBoxVerticalValues;
  setables["haxis"] = ui->comboBoxHAxisType;
  setables["hlabel"] = ui->lineEditHtitle;
  setables["vlabel"] = ui->lineEditVtitle;
  setables["vunit"] = ui->lineEditVUnit;
  setables["opengl"] = ui->checkBoxOpenGL;

  // Connection
  setables["baud"] = ui->comboBoxBaud;
  setables["debuglvl"] = ui->comboBoxOutputLevel;

  // Settings
  setables["clearonrec"] = ui->checkBoxClearOnReconnect;
  setables["manualin"] = ui->checkBoxShowManualInput;
  setables["serialmon"] = ui->checkBoxSerialMonitor;
  setables["sendonrec"] = ui->checkBoxResetCmdEn;
  setables["rstcmd"] = ui->lineEditResetCmd;

  // Send
  setables["sendend"] = ui->comboBoxLineEnding;
  setables["send1"] = ui->lineEditCommand;
  setables["send2"] = ui->lineEditCommand_2;
  setables["send3"] = ui->lineEditCommand_3;
  setables["send4"] = ui->lineEditCommand_4;
  setables["multisend"] = ui->pushButtonMultiplInputs;
}

void MainWindow::applyGuiElementSettings(QWidget* target, QString value) {
  if (QDoubleSpinBox* newTarget = dynamic_cast<QDoubleSpinBox*>(target))
    newTarget->setValue(value.toDouble());
  else if (QComboBox* newTarget = dynamic_cast<QComboBox*>(target))
    newTarget->setCurrentIndex(value.toUInt());
  else if (QSpinBox* newTarget = dynamic_cast<QSpinBox*>(target))
    newTarget->setValue(value.toInt());
  else if (QScrollBar* newTarget = dynamic_cast<QScrollBar*>(target))
    newTarget->setValue(value.toInt());
  else if (QSlider* newTarget = dynamic_cast<QSlider*>(target))
    newTarget->setValue(value.toInt());
  else if (QCheckBox* newTarget = dynamic_cast<QCheckBox*>(target))
    newTarget->setChecked((bool)value.toUInt());
  else if (QPushButton* newTarget = dynamic_cast<QPushButton*>(target))
    newTarget->setChecked((bool)value.toUInt());
  else if (QDial* newTarget = dynamic_cast<QDial*>(target))
    newTarget->setValue(value.toInt());
  else if (QLineEdit* newTarget = dynamic_cast<QLineEdit*>(target))
    newTarget->setText(value);
}

QByteArray MainWindow::readGuiElementSettings(QWidget* target) {
  if (QDoubleSpinBox* newTarget = dynamic_cast<QDoubleSpinBox*>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QSpinBox* newTarget = dynamic_cast<QSpinBox*>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QScrollBar* newTarget = dynamic_cast<QScrollBar*>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QSlider* newTarget = dynamic_cast<QSlider*>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QCheckBox* newTarget = dynamic_cast<QCheckBox*>(target))
    return (newTarget->isChecked() ? "1" : "0");
  if (QPushButton* newTarget = dynamic_cast<QPushButton*>(target))
    return (newTarget->isChecked() ? "1" : "0");
  if (QComboBox* newTarget = dynamic_cast<QComboBox*>(target))
    return (QString::number(newTarget->currentIndex()).toUtf8());
  if (QDial* newTarget = dynamic_cast<QDial*>(target))
    return (QString::number(newTarget->value()).toUtf8());
  else if (QLineEdit* newTarget = dynamic_cast<QLineEdit*>(target))
    return (newTarget->text().toUtf8());
  return "";
}

QByteArray MainWindow::getSettings() {
  QByteArray settings;

  for (QMap<QString, QWidget*>::iterator it = setables.begin(); it != setables.end(); it++)
    settings.append(QString(it.key() + ':' + readGuiElementSettings(it.value()) + ";\n").toUtf8());

  if (ui->radioButtonFixedRange->isChecked())
    settings.append("plotrange:fix;\n");
  else if (ui->radioButtonFreeRange->isChecked())
    settings.append("plotrange:free;\n");
  else if (ui->radioButtonRollingRange->isChecked())
    settings.append("plotrange:roll;\n");

  if (ui->pushButtonTerminalClickToSend->isChecked())
    settings.append("terminal:clicksend;\n");
  else if (ui->pushButtonTerminalClickToSend->isChecked())
    settings.append("terminal:select;\n");
  else
    settings.append("terminal:nointeract;\n");

  settings.append(ui->radioButtonEn->isChecked() ? "lang:en" : "lang:cz");
  settings.append(";\n");
  settings.append(ui->radioButtonCSVDot->isChecked() ? "csvsep:dc" : "csvsep:cs");
  settings.append(";\n");

  QColor xyclr = ui->plotxy->graphXY->pen().color();
  settings.append(QString("xyclr:%1,%2,%3").arg(xyclr.red()).arg(xyclr.green()).arg(xyclr.blue()).toUtf8());
  settings.append(";\n");

  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
    settings.append("ch:" + QString::number(i + 1).toUtf8());
    settings.append(":sty:" + QString::number(ui->plot->getChStyle(i)).toUtf8());
    settings.append(";\n");
    settings.append("ch:" + QString::number(i + 1).toUtf8());
    QColor clr = ui->plot->getChColor(i);
    settings.append(QString(":clr:%1,%2,%3").arg(clr.red()).arg(clr.green()).arg(clr.blue()).toUtf8());
    settings.append(";\n");
  }

  for (int i = 0; i < LOGIC_GROUPS; i++) {
    settings.append("log:" + QString::number(i + 1).toUtf8());
    settings.append(":sty:" + QString::number(ui->plot->getLogicStyle(i)).toUtf8());
    settings.append(";\n");
    settings.append("log:" + QString::number(i + 1).toUtf8());
    QColor clr = ui->plot->getLogicColor(i);
    settings.append(QString(":clr:%1,%2,%3").arg(clr.red()).arg(clr.green()).arg(clr.blue()).toUtf8());
    settings.append(";\n");
  }
  return settings;
}

void MainWindow::useSettings(QByteArray settings, MessageTarget::enumMessageTarget source = MessageTarget::manual) {
  settings.replace('\n', "");
  settings.replace('\r', "");
  if (!settings.contains(':')) {
    if (source == MessageTarget::manual || ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
      printMessage(tr("Invalid settings").toUtf8(), settings, MessageLevel::error, source);
    return;
  }
  QByteArray type = settings.left(settings.indexOf(':', 0));
  QByteArray value = settings.mid(settings.indexOf(':', 0) + 1);
  type = type.simplified().toLower();

  if (setables.contains(type))
    applyGuiElementSettings(setables[type], value);

  else if (type == "lang") {
    if (value == "en")
      ui->radioButtonEn->setChecked(true);
    if (value == "cz")
      ui->radioButtonCz->setChecked(true);
  }

  else if (type == "terminal") {
    if (value == "clicksend")
      ui->pushButtonTerminalClickToSend->setChecked(true);
    if (value == "select")
      ui->pushButtonTerminalSelect->setChecked(true);
    if (value == "nointeract") {
      ui->pushButtonTerminalClickToSend->setChecked(false);
      ui->pushButtonTerminalSelect->setChecked(false);
    }
  }

  else if (type == "csvsep") {
    if (value == "cs")
      ui->radioButtonCSVComma->setChecked(true);
    if (value == "dc")
      ui->radioButtonCSVDot->setChecked(true);
  }

  else if (type == "plotrange") {
    if (value == "fix")
      ui->radioButtonFixedRange->setChecked(true);
    if (value == "free")
      ui->radioButtonFreeRange->setChecked(true);
    if (value == "roll")
      ui->radioButtonRollingRange->setChecked(true);
  }

  else if (type == "xyclr") {
    QByteArrayList rgb = value.split(',');
    if (rgb.length() != 3) {
      if (source == MessageTarget::manual || ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
        printMessage(tr("Invalid color: ").toUtf8(), settings, MessageLevel::error, source);
      return;
    }
    QColor clr = QColor(rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt());
    ui->plotxy->graphXY->setPen(clr);
  }

  else if (type == "ch") {
    int ch = value.left(value.indexOf(':', 0)).toUInt();
    if (ch > ANALOG_COUNT + MATH_COUNT || ch == 0) {
      if (source == MessageTarget::manual || ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
        printMessage(tr("Invalid channel in settings").toUtf8(), QString::number(ch).toUtf8(), MessageLevel::error, source);
      return;
    }
    // V nastavení se čísluje od jedné, ale v příkazech od nuly
    ch = ch - 1;
    QByteArray subtype = value.mid(value.indexOf(':', 0) + 1).toLower();
    subtype = subtype.left(subtype.indexOf(':'));
    QByteArray subvalue = value.mid(value.lastIndexOf(':') + 1);

    if (subtype == "sty")
      ui->plot->setChStyle(ch, subvalue.toUInt());
    else if (subtype == "clr") {
      QByteArrayList rgb = subvalue.mid(subvalue.indexOf(':')).split(',');
      if (rgb.length() != 3) {
        if (source == MessageTarget::manual || ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
          printMessage(tr("Invalid color: ").toUtf8(), settings, MessageLevel::error, source);
        return;
      }
      QColor clr = QColor(rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt());
      ui->plot->setChColor(ch, clr);
      colorUpdateNeeded = true;
    }
    if (ui->comboBoxSelectedChannel->currentIndex() == ch)
      on_comboBoxSelectedChannel_currentIndexChanged(ui->comboBoxSelectedChannel->currentIndex());
  }

  else if (type == "log") {
    int group = value.left(value.indexOf(':', 0)).toUInt() - 1;
    if (group >= LOGIC_GROUPS) {
      if (source == MessageTarget::manual || ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
        printMessage(tr("Invalid logic in settings").toUtf8(), QString::number(group).toUtf8(), MessageLevel::error, source);
      return;
    }
    QByteArray subtype = value.mid(value.indexOf(':', 0) + 1).toLower();
    subtype = subtype.left(subtype.indexOf(':'));
    QByteArray subvalue = value.mid(value.lastIndexOf(':') + 1);

    if (subtype == "off")
      ui->plot->setLogicOffset(group, subvalue.toDouble());
    else if (subtype == "sca")
      ui->plot->setLogicScale(group, subvalue.toDouble());
    else if (subtype == "sty")
      ui->plot->setLogicStyle(group, subvalue.toUInt());
    else if (subtype == "clr") {
      QByteArrayList rgb = subvalue.mid(subvalue.indexOf(':')).split(',');
      if (rgb.length() != 3) {
        if (source == MessageTarget::manual || ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
          printMessage(tr("Invalid color: ").toUtf8(), settings, MessageLevel::error, source);
        return;
      }
      QColor clr = QColor(rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt());
      ui->plot->setLogicColor(group, clr);
      colorUpdateNeeded = true;
    }
    if (ui->comboBoxSelectedChannel->currentIndex() == group + ANALOG_COUNT + MATH_COUNT)
      on_comboBoxSelectedChannel_currentIndexChanged(ui->comboBoxSelectedChannel->currentIndex());
  }

  // Error
  else {
    if (source == MessageTarget::manual || ui->comboBoxOutputLevel->currentIndex() >= MessageLevel::error)
      printMessage(tr("Unknown setting").toUtf8(), type, MessageLevel::error, source);
    return;
  }
  if (source == MessageTarget::manual || ui->comboBoxOutputLevel->currentIndex() == MessageLevel::info)
    printMessage(tr("Applied settings").toUtf8(), settings, MessageLevel::info, source);
}

void MainWindow::on_pushButtonLoadFile_clicked() {
  QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/settings/");
  QString fileName = QFileDialog::getOpenFileName(this, tr("Load file"), defaultName, tr("Text file (*.txt);;Any file (*.*)"));
  if (fileName.isEmpty())
    return;
  QFile file(fileName);
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    sendFileToParser(file.readAll(), false, ui->checkBoxAddSemicolum->isChecked());
  } else {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant open file."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_pushButtonDefaults_clicked() {
  QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/settings/default.txt");
  QFile file(defaultName);
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    sendFileToParser(file.readAll(), true);
  } else {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant open file."));
    msgBox.setInformativeText(defaultName);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_pushButtonSaveSettings_clicked() {
  QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/settings/");
  QString fileName = QFileDialog::getSaveFileName(this, tr("Load settings"), defaultName, tr("Text file (*.txt)"));
  if (fileName.isEmpty())
    return;
  QFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Truncate)) {
    file.write("$$S\n" + getSettings() + "$$U");
  } else {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant open file."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_pushButtonReset_clicked() {
  QFile defaults(":/text/settings/default.txt");
  if (defaults.open(QFile::ReadOnly | QFile::Text))
    sendFileToParser(defaults.readAll(), true);
}

void MainWindow::setUp() {
  QString userDefaults = QString(QCoreApplication::applicationDirPath()) + QString("/settings/default.txt");
  QFile userDefaultsFile(userDefaults);
  if (userDefaultsFile.open(QFile::ReadOnly | QFile::Text))
    sendFileToParser(userDefaultsFile.readAll(), true);
  else {
    QFile defaults(":/text/settings/default.txt");
    if (defaults.open(QFile::ReadOnly | QFile::Text))
      sendFileToParser(defaults.readAll(), true);
  }

  QString style = "";
#ifdef _WIN32
  if (QSysInfo::productVersion() == "10")
    style = ":/text/stylesheets/styleSheetWindows10.txt";
  else
    style = ":/text/stylesheets/styleSheetWindows.txt";
#endif           // Windows
#ifdef __linux__ // Linux
  style = ":/text/stylesheets/styleSheetLinux.txt";
#endif // Linux
  QFile styleSheet(style);

  if (styleSheet.open(QFile::ReadOnly | QFile::Text)) {
    qApp->setStyleSheet(styleSheet.readAll());
    qDebug() << "Using stylesheet: " << style;
  }
}

void MainWindow::sendFileToParser(QByteArray text, bool removeLastNewline, bool addSemicolums) {
  if (removeLastNewline && text.right(1) == "\n")
    text.remove(text.length() - 1, 1);
  if (addSemicolums)
    text.replace("\n", ";");
  emit sendManualInput(text);
}
