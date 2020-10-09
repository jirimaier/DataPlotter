#include "mainwindow.h"

void MainWindow::on_pushButtonConnect_clicked() {
  if (ui->comboBoxCom->currentIndex() >= 0) {
    emit connectSerial(portList.at(ui->comboBoxCom->currentIndex()).portName(), ui->comboBoxBaud->currentText().toInt());
    if (!ui->checkBoxModeManual->isChecked())
      emit setMode(DataMode::unknown);
  }
}

void MainWindow::on_tabs_right_currentChanged(int index) {
  if (index == 2)
    ui->lineEditCommand->setFocus();
}

void MainWindow::on_pushButtonClearChannels_clicked() {
  ui->plot->resetChannels();
  emit resetChannels();
}

void MainWindow::on_pushButtonChannelColor_clicked() {
  QColor color = QColorDialog::getColor(ui->plot->getChColor(ui->spinBoxChannelSelect->value()));
  if (!color.isValid())
    return;
  ui->plot->setChColor(ui->spinBoxChannelSelect->value(), color);
  QPixmap pixmap(30, 30);
  pixmap.fill(color);
  ui->pushButtonChannelColor->setIcon(pixmap);
}

void MainWindow::on_spinBoxChannelSelect_valueChanged(int arg1) {
  if (ui->checkBoxSelectOnlyUsed->isChecked()) {
    if (!ui->plot->isChUsed(arg1)) {
      if (arg1 == CHANNEL_COUNT) {
        lastSelectedChannel = arg1;
        ui->spinBoxChannelSelect->setValue(arg1 - 1);
        return;
      }
      if (arg1 != 1) {
        if (lastSelectedChannel < arg1) {
          ui->spinBoxChannelSelect->setValue(arg1 + 1);
          return;
        }
        if (lastSelectedChannel > arg1) {
          ui->spinBoxChannelSelect->setValue(arg1 - 1);
          return;
        }
      }
    }
  }
  lastSelectedChannel = arg1;
  ui->comboBoxGraphStyle->setCurrentIndex(ui->plot->getChStyle(arg1));
  QPixmap pixmap(30, 30);
  pixmap.fill(ui->plot->getChColor(arg1));
  ui->pushButtonChannelColor->setIcon(pixmap);
  double offset = ui->plot->getChOffset(arg1);
  double scale = ui->plot->getChScale(arg1);
  ui->doubleSpinBoxChOffset->setValue(offset);
  ui->doubleSpinBoxChScale->setValue(scale);
  ui->dialChScale->updatePosition(scale);
  ui->checkBoxChInvert->setChecked(ui->plot->isInverted(arg1));
  updateChScale();
}

void MainWindow::on_doubleSpinBoxChOffset_valueChanged(double arg1) {
  ui->plot->changeChOffset(ui->spinBoxChannelSelect->value(), arg1);
  scrollBarCursorValueChanged();
}

void MainWindow::on_comboBoxGraphStyle_currentIndexChanged(int index) {
  ui->plot->setChStyle(ui->spinBoxChannelSelect->value(), index);
  ui->plot->updateVisuals();
}
void MainWindow::on_doubleSpinBoxChScale_valueChanged(double arg1) {
  ui->plot->changeChScale(ui->spinBoxChannelSelect->value(), arg1 * (ui->checkBoxChInvert->isChecked() ? -1 : 1));
  scrollBarCursorValueChanged();
  updateChScale();
}

void MainWindow::on_pushButtonSelectedCSV_clicked() { exportCSV(false, ui->spinBoxChannelSelect->value() - 1); }

void MainWindow::on_dialZoom_valueChanged(int value) {
  ui->plot->setZoomRange(value);
  ui->horizontalScrollBarHorizontal->setMinimum(value / 2);
  ui->horizontalScrollBarHorizontal->setMaximum(1000 - value / 2);
  ui->horizontalScrollBarHorizontal->setPageStep(value);
}

void MainWindow::on_radioButtonEn_toggled(bool checked) {
  if (checked)
    changeLanguage("en");
}

void MainWindow::on_radioButtonCz_toggled(bool checked) {
  if (checked)
    changeLanguage("cz");
}

void MainWindow::on_pushButtonPrintBuffer_clicked() {}

void MainWindow::on_pushButtonDataModeApply_clicked() {
  emit setMode(ui->comboBoxDataMode->currentIndex());
  BinDataSettings_t settings;
  settings.bits = ui->spinBoxDataBinaryBits->value();
  settings.continuous = ui->checkBoxBinContinuous->isChecked();
  settings.firstCh = ui->spinBoxBinaryDataFirstCh->value();
  settings.numCh = ui->spinBoxBinaryDataNumCh->value();
  settings.timeStep = ui->doubleSpinBoxBinaryTimestep->value();
  settings.valueMax = ui->doubleSpinBoxBinarydataMax->value();
  settings.valueMin = ui->doubleSpinBoxBinaryDataMin->value();
  emit setBinParameters(settings);
}

void MainWindow::on_checkBoxModeManual_toggled(bool checked) {
  emit allowModeChange(!checked);
  if (checked) {
    ui->spinBoxDataBinaryBits->setValue(binSettings.bits);
    ui->doubleSpinBoxBinaryDataMin->setValue(binSettings.valueMin);
    ui->doubleSpinBoxBinarydataMax->setValue(binSettings.valueMax);
    ui->doubleSpinBoxBinaryTimestep->setValue(binSettings.timeStep);
    ui->spinBoxBinaryDataNumCh->setValue(binSettings.numCh);
    ui->spinBoxBinaryDataFirstCh->setValue(binSettings.firstCh);
    ui->checkBoxBinContinuous->setChecked(binSettings.continuous);
  }
}

void MainWindow::on_horizontalSliderLineTimeout_valueChanged(int value) {
  ui->labelLineTimeout->setText(QString::number(logaritmicSettings[value]) + " ms");
  emit changeLineTimeout(logaritmicSettings[value]);
}

void MainWindow::on_lineEditManualInput_returnPressed() {
  emit sendManaulInput(ui->lineEditManualInput->text().toUtf8(), ui->comboBoxManualInput->currentIndex());
  ui->lineEditManualInput->clear();
}

void MainWindow::on_pushButtonDisconnect_clicked() {
  emit disconnectSerial();
  ui->labelPortInfo->setText(tr("Disconnecting..."));
}

void MainWindow::on_pushButtonSendCommand_clicked() {
  QString text = ui->lineEditCommand->text() + lineEndings[ui->comboBoxLineEnding->currentIndex()];
  emit writeToSerial(text.toUtf8());
}
