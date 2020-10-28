#include "mainwindow.h"

void MainWindow::initSetables() {
  setables["vrange"] = ui->doubleSpinBoxRangeVerticalRange;
  setables["hrange"] = ui->doubleSpinBoxRangeHorizontal;
  setables["hdiv"] = ui->dialhorizontalDiv;
  setables["vdiv"] = ui->dialVerticalDiv;
  setables["plottype"] = ui->comboBoxPlotRangeType;
  setables["mathvro"] = ui->checkBoxMathVRO;
  setables["mathios"] = ui->checkBoxMathIOS;
  setables["xyen"] = ui->checkBoxXY;
  setables["xyvro"] = ui->checkBoxXYVRO;
  setables["xyios"] = ui->checkBoxXYIOS;
  setables["xyxch"] = ui->spinBoxXYFirst;
  setables["xyych"] = ui->spinBoxXYSecond;
  setables["xyautosize"] = ui->checkBoxXYAutoSize;

  for (int i = 0; i < MATH_COUNT; i++) {
    setables[QString("math") + QString::number(i + 1) + QString("en")] = mathEn[i];
    setables[QString("math") + QString::number(i + 1) + QString("first")] = mathFirst[i];
    setables[QString("math") + QString::number(i + 1) + QString("second")] = mathSecond[i];
    setables[QString("math") + QString::number(i + 1) + QString("op")] = mathOp[i];
  }
}

void MainWindow::applyGuiElementSettings(QWidget *target, QString value) {
  if (QDoubleSpinBox *newTarget = dynamic_cast<QDoubleSpinBox *>(target))
    newTarget->setValue(value.toDouble());
  else if (QSpinBox *newTarget = dynamic_cast<QSpinBox *>(target))
    newTarget->setValue(value.toInt());
  else if (QCheckBox *newTarget = dynamic_cast<QCheckBox *>(target))
    newTarget->setChecked((bool)value.toUInt());
  else if (QComboBox *newTarget = dynamic_cast<QComboBox *>(target))
    newTarget->setCurrentIndex(value.toUInt());
  else if (QDial *newTarget = dynamic_cast<QDial *>(target))
    newTarget->setValue(value.toInt());
}

QByteArray MainWindow::readGuiElementSettings(QWidget *target) {
  if (QDoubleSpinBox *newTarget = dynamic_cast<QDoubleSpinBox *>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QSpinBox *newTarget = dynamic_cast<QSpinBox *>(target))
    return (QString::number(newTarget->value()).toUtf8());
  if (QCheckBox *newTarget = dynamic_cast<QCheckBox *>(target))
    return (newTarget->isChecked() ? "1" : "0");
  if (QComboBox *newTarget = dynamic_cast<QComboBox *>(target))
    return (QString::number(newTarget->currentIndex()).toUtf8());
  if (QDial *newTarget = dynamic_cast<QDial *>(target))
    return (QString::number(newTarget->value()).toUtf8());
  return "error";
}

QByteArray MainWindow::getSettings() {
  QByteArray settings;

  for (QMap<QString, QWidget *>::iterator it = setables.begin(); it != setables.end(); it++)
    settings.append(QString(it.key() + ':' + readGuiElementSettings(it.value()) + '\n').toUtf8());

  settings.append(ui->radioButtonEn->isChecked() ? "lang:en" : "lang:cz");
  settings.append('\n');
  settings.append(ui->radioButtonCSVDot->isChecked() ? "csvdel:dc" : "csvdel:cs");
  settings.append('\n');

  for (int i = 1; i <= CHANNEL_COUNT + MATH_COUNT; i++) {
    settings.append("ch:" + QString::number(i).toUtf8());
    settings.append(":off:" + QString::number(ui->plot->getChOffset(i)).toUtf8());
    settings.append('\n');
    settings.append("ch:" + QString::number(i).toUtf8());
    settings.append(":sca:" + QString::number(ui->plot->getChScale(i)).toUtf8());
    settings.append('\n');
    settings.append("ch:" + QString::number(i).toUtf8());
    settings.append(":inv:" + QString::number(ui->plot->isInverted(i) ? 1 : 0).toUtf8());
    settings.append('\n');
    settings.append("ch:" + QString::number(i).toUtf8());
    settings.append(":sty:" + QString::number(ui->plot->getChStyle(i)).toUtf8());
    settings.append('\n');
    settings.append("ch:" + QString::number(i).toUtf8());
    settings.append(":name:" + ui->plot->getChName(i).toUtf8());
    settings.append('\n');
    settings.append("ch:" + QString::number(i).toUtf8());
    QColor clr = ui->plot->getChColor(i);
    settings.append(QString(":col:%1,%2,%3").arg(clr.red()).arg(clr.green()).arg(clr.blue()).toUtf8());
    settings.append('\n');
  }
  return settings;
}

void MainWindow::useSettings(QByteArray settings) {
  QByteArrayList settingsList = settings.split('\n');
  foreach (QByteArray line, settingsList) {
    line = line.simplified().trimmed();
    if (line.isEmpty())
      continue;
    if (!line.contains(':')) {
      emit parseError(QString(tr("Invalid settings: ")).toUtf8() + line);
      continue;
    }
    QByteArray type = line.left(line.indexOf(':', 0));
    QByteArray value = line.mid(line.indexOf(':', 0) + 1);
    type = type.simplified().toLower();

    if (setables.contains(type))
      applyGuiElementSettings(setables[type], value);

    else if (type == "lang") {
      if (value == "en")
        ui->radioButtonEn->setChecked(true);
      if (value == "cz")
        ui->radioButtonCz->setChecked(true);
    }

    else if (type == "csvdel") {
      if (value == "cs")
        ui->radioButtonCSVComma->setChecked(true);
      if (value == "dc")
        ui->radioButtonCSVDot->setChecked(true);
    }

    else if (type == "ch") {
      int ch = value.left(value.indexOf(':', 0)).toUInt();
      if (ch > CHANNEL_COUNT + MATH_COUNT) {
        emit parseError((QString(tr("Invalid channel in settings: ")) + QString::number(ch)).toUtf8());
        continue;
      }
      QByteArray subtype = value.mid(value.indexOf(':', 0) + 1);
      subtype = subtype.left(subtype.indexOf(':'));
      QByteArray subvalue = value.mid(value.lastIndexOf(':') + 1);

      if (subtype == "off")
        ui->plot->changeChOffset(ch, subvalue.toDouble());
      else if (subtype == "sca")
        ui->plot->changeChScale(ch, subvalue.toDouble());
      else if (subtype == "inv") {
        if (ui->plot->isInverted(ch) ^ (bool)subvalue.toUInt())
          ui->plot->changeChScale(ch, ui->plot->getChScale(ch) * (-1));
      } else if (subtype == "sty")
        ui->plot->setChStyle(ch, subvalue.toUInt());
      else if (subtype == "name")
        ui->plot->setChName(ch, subvalue);

      else if (subtype == "col") {
        QByteArrayList rgb = subvalue.mid(subvalue.indexOf(':')).split(',');
        if (rgb.length() != 3) {
          emit parseError(QString(tr("Invalid color: ")).toUtf8() + line);
          continue;
        }
        QColor clr = QColor(rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt());
        ui->plot->setChColor(ch, clr);
      }
      updateSelectedChannel(ui->spinBoxChannelSelect->value());
    }

    // Speciální
    else if (type == "cmd")
      emit sendManaulInput(value, DataLineType::command);
    else if (type == "data")
      emit sendManaulInput(value, DataLineType::dataEnded);

    // Error
    else
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

  QFile styleSheet(":/styles/settings/styleSheet.txt");
  if (styleSheet.open(QFile::ReadOnly | QFile::Text))
    qApp->setStyleSheet(styleSheet.readAll());
}
