#include "mainwindow.h"

QByteArray MainWindow::getSettings() {
  QByteArray settings;
  settings.append("vrange:" + QString::number(ui->doubleSpinBoxRangeVerticalRange->value()).toUtf8());
  settings.append('\n');
  settings.append("hrange:" + QString::number(ui->doubleSpinBoxRangeHorizontal->value()).toUtf8());
  settings.append('\n');
  settings.append("vdiv:" + QString::number(ui->dialVerticalDiv->value() - ui->dialVerticalDiv->minimum() + 1).toUtf8());
  settings.append('\n');
  settings.append("linetimeout:" + QString::number(ui->horizontalSliderLineTimeout->value()).toUtf8());
  settings.append('\n');
  settings.append("vpos:" + QString::number(ui->verticalScrollBarVerticalCenter->value()).toUtf8());
  settings.append('\n');
  settings.append("hdiv:" + QString::number(ui->dialhorizontalDiv->value() - ui->dialhorizontalDiv->minimum() + 1).toUtf8());
  settings.append('\n');
  settings.append(ui->radioButtonEn->isChecked() ? "lang:en" : "lang:cz");
  settings.append('\n');
  settings.append("chsou:" + QString::number(ui->checkBoxSelectOnlyUsed->isChecked() ? 1 : 0).toUtf8());
  settings.append('\n');
  settings.append("clrgor:" + QString::number(ui->checkBoxClearOnReconnect->isChecked() ? 1 : 0).toUtf8());
  settings.append('\n');
  for (int i = 1; i <= CHANNEL_COUNT; i++) {
    settings.append("ch:" + QString::number(i).toUtf8());
    settings.append('\n');
    settings.append("choff:" + QString::number(ui->plot->getChOffset(i)).toUtf8());
    settings.append('\n');
    settings.append("chsca:" + QString::number(ui->plot->getChScale(i)).toUtf8());
    settings.append('\n');
    settings.append("chinv:" + QString::number(ui->plot->isInverted(i) ? 1 : 0).toUtf8());
    settings.append('\n');
    settings.append("chsty:" + QString::number(ui->plot->getChStyle(i)).toUtf8());
    settings.append('\n');
    QColor clr = ui->plot->getChColor(i);
    settings.append(QString("chcol:%1,%2,%3").arg(clr.red()).arg(clr.green()).arg(clr.blue()).toUtf8());
    settings.append('\n');
  }
  settings.append("ch:" + QString::number(ui->spinBoxChannelSelect->value()).toUtf8());
  return settings;
}

void MainWindow::useSettings(QByteArray settings) {
  QByteArrayList settingsList = settings.split('\n');
  foreach (QByteArray line, settingsList) {
    line = line.simplified().trimmed();
    if (line.isEmpty())
      continue;
    QByteArrayList parts = line.split(':');
    if (parts.length() != 2) {
      emit parseError(QString(tr("Invalid settings: ")).toUtf8() + line);
      continue;
    }
    QByteArray type = parts.at(0);
    type = type.simplified();
    QByteArray value = parts.at(1);
    type = type.toLower();

    if (type == "vrange")
      ui->doubleSpinBoxRangeVerticalRange->setValue(value.toDouble());
    else if (type == "hrange")
      ui->doubleSpinBoxRangeHorizontal->setValue(value.toDouble());
    else if (type == "linetimeout")
      ui->horizontalSliderLineTimeout->setValue(value.toDouble());
    else if (type == "vpos")
      ui->verticalScrollBarVerticalCenter->setValue(value.toDouble());
    else if (type == "vdiv")
      ui->dialVerticalDiv->setValue(ui->dialVerticalDiv->minimum() + value.toInt() - 1);
    else if (type == "hdiv")
      ui->dialVerticalDiv->setValue(ui->dialhorizontalDiv->minimum() + value.toInt() - 1);
    else if (type == "cmd")
      emit sendManaulInput(value, DataLineType::command);
    else if (type == "data")
      emit sendManaulInput(value, DataLineType::dataEnded);
    else if (type == "choff")
      ui->doubleSpinBoxChOffset->setValue(value.toDouble());
    else if (type == "chsca")
      ui->doubleSpinBoxChScale->setValue(value.toDouble());
    else if (type == "chinv")
      ui->checkBoxChInvert->setChecked((bool)value.toInt());
    else if (type == "chsou")
      ui->checkBoxSelectOnlyUsed->setChecked((bool)value.toInt());
    else if (type == "clrgor")
      ui->checkBoxClearOnReconnect->setChecked((bool)value.toInt());
    else if (type == "chsty")
      ui->comboBoxGraphStyle->setCurrentIndex(value.toInt());
    else if (type == "chcol") {
      QByteArrayList rgb = value.split(',');
      if (rgb.length() != 3) {
        emit parseError(QString(tr("Invalid color: ")).toUtf8() + line);
        continue;
      }
      QColor clr = QColor(rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt());
      ui->plot->setChColor(ui->spinBoxChannelSelect->value(), clr);
      QPixmap pixmap(30, 30);
      pixmap.fill(clr);
      ui->pushButtonChannelColor->setIcon(pixmap);
    } else if (type == "lang") {
      if (value == "en")
        ui->radioButtonEn->setChecked(true);
      if (value == "cz")
        ui->radioButtonCz->setChecked(true);
    } else if (type == "ch") {
      bool b = ui->checkBoxSelectOnlyUsed->isChecked();
      ui->checkBoxSelectOnlyUsed->setChecked(false);
      ui->spinBoxChannelSelect->setValue(value.toInt());
      ui->checkBoxSelectOnlyUsed->setChecked(b);
    } else
      emit parseError(QString(tr("Unknown setting: ")).toUtf8() + type);
  }
}

void MainWindow::on_pushButtonLoadFile_clicked() {
  QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/settings/");
  QString fileName = QFileDialog::getOpenFileName(this, tr("Load settings"), defaultName, tr("Text file (*.txt)"));
  if (fileName.isEmpty())
    return;
  QFile file(fileName);
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    useSettings(file.readAll());
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
    useSettings(file.readAll());
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
    file.write(getSettings());
  } else {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant open file."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_pushButtonReset_clicked() {
  QFile defaults(":/documents/settings/defaultSettings.txt");
  if (defaults.open(QFile::ReadOnly | QFile::Text))
    useSettings(defaults.readAll());
}

void MainWindow::setUp() {
  QString userDefaults = QString(QCoreApplication::applicationDirPath()) + QString("/settings/default.txt");
  QFile userDefaultsFile(userDefaults);
  if (userDefaultsFile.open(QFile::ReadOnly | QFile::Text))
    useSettings(userDefaultsFile.readAll());
  else {
    QFile defaults(":/documents/settings/defaultSettings.txt");
    if (defaults.open(QFile::ReadOnly | QFile::Text))
      useSettings(defaults.readAll());
  }
}
