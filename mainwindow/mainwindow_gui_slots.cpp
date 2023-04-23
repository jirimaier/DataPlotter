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

void MainWindow::rangeTypeChanged() {
  if (ui->radioButtonFixedRange->isChecked()) {
    ui->plot->setRangeType(PlotRange::fixedRange);
    ui->frameRollingRange->setHidden(true);
    ui->frameZoom->setVisible(true);
  } else if (ui->radioButtonFreeRange->isChecked())
    ui->plot->setRangeType(PlotRange::freeMove);
  else if (ui->radioButtonRollingRange->isChecked()) {
    ui->plot->setRangeType(PlotRange::rolling);
    ui->frameZoom->setHidden(true);
    ui->frameRollingRange->setVisible(true);
  }
}

void MainWindow::plotLayoutChanged() {
  if (ui->radioButtonLayoutAll->isChecked()) {
    ui->plot->setVisible(true);
    ui->plotxy->setVisible(true);
    ui->plotFFT->setVisible(true);
  } else if (ui->radioButtonLayoutTime->isChecked()) {
    ui->plot->setVisible(true);
    ui->plotxy->setVisible(false);
    ui->plotFFT->setVisible(false);
  } else if (ui->radioButtonLayoutFFT->isChecked()) {
    ui->plot->setVisible(false);
    ui->plotxy->setVisible(false);
    ui->plotFFT->setVisible(true);
  } else { /*if (ui->radioButtonLayoutAll->isChecked())*/
    ui->plot->setVisible(false);
    ui->plotxy->setVisible(true);
    ui->plotFFT->setVisible(false);
  }

}

void MainWindow::on_doubleSpinBoxChOffset_valueChanged(double arg1) {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChOffset(ui->comboBoxSelectedChannel->currentIndex(), arg1);
  else
    ui->plot->setLogicOffset(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, arg1);
}

void MainWindow::on_comboBoxGraphStyle_currentIndexChanged(int index) {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT) {
    ui->plot->setChStyle(ui->comboBoxSelectedChannel->currentIndex(), index);
  } else {
    if (recommendOpenGL && (index == GraphStyle::logicFilled || index == GraphStyle::logicSquareFilled) && !ui->checkBoxOpenGL->isChecked()) {
      QMessageBox msgBox(this);
      msgBox.setText(tr("It is recommended to enable OpenGL when using fill under graph."));
      msgBox.setInformativeText(tr("Enable OpenGL? (enabling may take a second or two)"));
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::Yes);
      msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
      msgBox.setButtonText(QMessageBox::No, tr("No"));
      auto checkBox = new QCheckBox(&msgBox);
      checkBox->setText(tr("Dont show again"));
      msgBox.setCheckBox(checkBox);
      int returnValue = msgBox.exec();
      if (returnValue == QMessageBox::Yes)
        ui->checkBoxOpenGL->setChecked(true);
      if (checkBox->isChecked()) {
        recommendOpenGL = false;
        QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/settings/default.cfg");
        QFile file(defaultName);
        if (file.open(QFile::WriteOnly | QFile::Append)) {
          file.write("noopengldialog;\n");
        } else {
          QMessageBox msgBox(this);
          msgBox.setText(tr("Can not write to default settings file"));
          msgBox.setDetailedText(defaultName);
          msgBox.setIcon(QMessageBox::Critical);
          msgBox.exec();
        }
      }
    }
    ui->plot->setLogicStyle(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, index);
  }
}

void MainWindow::on_pushButtonConnect_clicked()
{
  SerialSettingsDialog::Settings settings = serialSettingsDialog->settings();
  if(ui->listWidgetCom->currentItem() != NULL)
    emit toggleSerialConnection(ui->listWidgetCom->currentItem()->data(Qt::UserRole).toString(), ui->comboBoxBaud->currentText().toInt(), settings.dataBits, settings.parity, settings.stopBits, settings.flowControl);
  else
    emit disconnectSerial();
}

void MainWindow::on_doubleSpinBoxChScale_valueChanged(double arg1) {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChScale(ui->comboBoxSelectedChannel->currentIndex(), arg1);
  else
    ui->plot->setLogicScale(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, arg1);
  updateChScale();
}

void MainWindow::on_dialZoom_valueChanged(int value) {
  ui->plot->setZoom(value);
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

void MainWindow::on_lineEditManualInput_returnPressed() {
  QByteArray bytes;
  bytes.append(ui->lineEditManualInput->text().toLocal8Bit());
  emit sendManualInput(bytes);
  ui->lineEditManualInput->clear();
  on_pushButtonScrollDown_clicked();
}

void MainWindow::on_pushButtonScrollDown_clicked() {
  QScrollBar* scroll = ui->plainTextEditConsole->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
};

void MainWindow::on_comboBoxOutputLevel_currentIndexChanged(int index) {
  if (index >= 0)
    emit setSerialMessageLevel((OutputLevel::enumOutputLevel)index);
}

void MainWindow::on_pushButtonScrollDown_2_clicked() {
  QScrollBar* scroll = ui->plainTextEditConsole_2->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole_2->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
}

void MainWindow::on_pushButtonScrollDown_3_clicked() {
  QScrollBar* scroll = ui->plainTextEditConsole_3->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole_3->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
}

void MainWindow::on_checkBoxSerialMonitor_toggled(bool checked) {
  ui->frameSerialMonitor->setVisible(checked);
  emit enableSerialMonitor(checked);
}

void MainWindow::on_comboBoxSelectedChannel_currentIndexChanged(int index) {
  // Zablokuje signáli, aby se po nastavení sočasných rovnou neposlali současné hodnoty jako změna.
  ui->doubleSpinBoxChOffset->blockSignals(true);
  ui->doubleSpinBoxChScale->blockSignals(true);
  //ui->dialChScale->blockSignals(true);
  ui->pushButtonHideCh->blockSignals(true);
  ui->comboBoxGraphStyle->blockSignals(true);
  ui->pushButtonInvert->blockSignals(true);

  if (IS_LOGIC_CH(index)) {
    setChStyleSelection(GraphType::logic);
    int group = CH_LIST_INDEX_TO_LOGIC_GROUP(index);
    ui->comboBoxGraphStyle->setCurrentIndex(ui->plot->getLogicStyle(group));
    double offset = ui->plot->getLogicOffset(group);
    double scale = ui->plot->getLogicScale(group);
    ui->doubleSpinBoxChOffset->setValue(offset);
    ui->doubleSpinBoxChScale->setValue(scale);
    //ui->dialChScale->updatePosition(scale);
    ui->pushButtonHideCh->setChecked(!ui->plot->isLogicVisible(group));
  } else {
    setChStyleSelection(index >= ANALOG_COUNT ? GraphType::math : GraphType::analog);
    ui->comboBoxGraphStyle->setCurrentIndex(ui->plot->getChStyle(index));
    double offset = ui->plot->getChOffset(index);
    double scale = ui->plot->getChScale(index);
    ui->doubleSpinBoxChOffset->setValue(offset);
    ui->doubleSpinBoxChScale->setValue(scale);
    //ui->dialChScale->updatePosition(scale);
    ui->pushButtonInvert->setChecked(ui->plot->isChInverted(index));
    ui->pushButtonHideCh->setChecked(!ui->plot->isChVisible(index));
    ui->pushButtonInterpolate->setChecked(ui->plot->isChInterpolated(index));
  }
  updateChScale();

  // Ikona by se sama nezměnila, protože jsou zablokované signály
  if (ui->pushButtonHideCh->isChecked())
    ui->pushButtonHideCh->setIcon(iconHidden);
  else
    ui->pushButtonHideCh->setIcon(iconVisible);

  ui->pushButtonInterpolate->setEnabled(index < ANALOG_COUNT + MATH_COUNT);
  ui->pushButtonInvert->setDisabled(IS_LOGIC_CH(index));

  ui->doubleSpinBoxChOffset->blockSignals(false);
  ui->doubleSpinBoxChScale->blockSignals(false);
  //ui->dialChScale->blockSignals(false);
  ui->pushButtonHideCh->blockSignals(false);
  ui->comboBoxGraphStyle->blockSignals(false);
  ui->pushButtonInvert->blockSignals(false);
}

void MainWindow::on_pushButtonInvert_toggled(bool checked) {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChInvert(ui->comboBoxSelectedChannel->currentIndex(), checked);
}

void MainWindow::on_comboBoxHAxisType_currentIndexChanged(int index) {
  ui->labelHDiv->setEnabled(index <= 1);
  if (index > HAxisType::normal) {
    ui->lineEditHUnit->setText("s");
  }
  updateDivs();
  ui->plot->setShowHorizontalValues(index);
}

void MainWindow::on_pushButtonOpenHelpCZ_clicked() {
  QString helpFile = QCoreApplication::applicationDirPath() + "/Manual.pdf";
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(helpFile))) {
    QMessageBox msgBox(this);
    msgBox.setText(tr("Cant open file."));
    msgBox.setInformativeText(helpFile);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_pushButtonChangeChColor_clicked() {
  QColor oldColor;
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    oldColor = ui->plot->getChColor(ui->comboBoxSelectedChannel->currentIndex());
  else
    oldColor = ui->plot->getLogicColor(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT);
  QColor color = QColorDialog::getColor(oldColor);
  if (!color.isValid())
    return;
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChColor(ui->comboBoxSelectedChannel->currentIndex(), color);
  else
    ui->plot->setLogicColor(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, color);
  colorUpdateNeeded = true;
  updateUsedChannels();
}

void MainWindow::on_horizontalSliderXYGrid_valueChanged(int value) {
  ui->plotxy->setGridHintX(value);
  ui->plotxy->setGridHintY(value);
}

void MainWindow::on_pushButtonXY_toggled(bool checked) {
  if (!checked)
    ui->plotxy->clear();
  else
    updateXY();

  if (checked && ui->radioButtonLayoutTime->isChecked())
    ui->radioButtonLayoutAll->setChecked(true);
  else if (!ui->pushButtonFFT->isChecked() && ui->radioButtonLayoutAll->isChecked())
    ui->radioButtonLayoutTime->setChecked(true);
}

void MainWindow::on_pushButtonHideCh_toggled(bool checked) {
  if (checked)
    ui->pushButtonHideCh->setIcon(iconHidden);
  else
    ui->pushButtonHideCh->setIcon(iconVisible);

  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChVisible(ui->comboBoxSelectedChannel->currentIndex(), !checked);
  else
    ui->plot->setLogicVisibility(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, !checked);
}

void MainWindow::on_pushButtonTerminalDebug_toggled(bool checked) {
  // TODO
}

void MainWindow::insertInTerminalDebug(QString text, QColor textColor) {
  ui->textEditTerminalDebug->setTextColor(textColor);
  ui->textEditTerminalDebug->textCursor().insertText(text);
  ui->textEditTerminalDebug->setTextColor(Qt::black);
}

void MainWindow::signalMeasurementsResult1(double period, double freq, double amp, double min, double max, double vrms, double dc, double fs, double rise, double fall, int samples) {
  if (recordingOfMeasurements1.isOpen()) {
    char decimal = ui->radioButtonCSVDot->isChecked() ? '.' : ',';
    char separator = ui->radioButtonCSVDot->isChecked() ? ',' : ';';

    recordingOfMeasurements1.write(QString::number(uptime.elapsed() / 1000.0, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(period, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(freq, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(amp, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(min, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(max, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(vrms, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(dc, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(fs, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(rise, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(fall, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write(QByteArray(&separator, 1));
    recordingOfMeasurements1.write(QString::number(samples, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements1.write("\n");
  }

  if (valuesUseUnits) {
    ui->labelSig1Vrms->setText(floatToNiceString(vrms, 4, false, false) + ui->plot->getYUnit());
    ui->labelSig1Min->setText(floatToNiceString(min, 4, false, false) + ui->plot->getYUnit());
    ui->labelSig1Max->setText(floatToNiceString(max, 4, false, false) + ui->plot->getYUnit());
    ui->labelSig1Dc->setText(floatToNiceString(dc, 4, false, false) + ui->plot->getYUnit());
    ui->labelSig1Amp->setText(floatToNiceString(amp, 4, false, false) + (ui->plot->getYUnit() == "V" ? "Vpp" : ui->plot->getYUnit()));
  } else {
    ui->labelSig1Vrms->setText(QString::number(vrms, 'g', 4) + ui->plot->getYUnit());
    ui->labelSig1Min->setText(QString::number(min, 'g', 4) + ui->plot->getYUnit());
    ui->labelSig1Max->setText(QString::number(max, 'g', 4) + ui->plot->getYUnit());
    ui->labelSig1Dc->setText(QString::number(dc, 'g', 4) + ui->plot->getYUnit());
    ui->labelSig1Amp->setText(QString::number(amp, 'g', 4) + ui->plot->getYUnit());
  }

  if (timeUseUnits) {
    ui->labelSig1Period->setText(floatToNiceString(period, 4, false, false) + ui->plot->getXUnit());
// Pokud je falltime nabo risetime menší než 2 periody vzorkování, je považován za nepřesný (znaménko menší než)
    ui->labelSig1rise->setText((rise < 2.0 / fs ? "<" : "") + floatToNiceString(rise, 4, false, false) + ui->plot->getXUnit());
    ui->labelSig1fall->setText((fall < 2.0 / fs ? "<" : "") + floatToNiceString(fall, 4, false, false) + ui->plot->getXUnit());
  } else {
    ui->labelSig1Period->setText(floatToNiceString(period, 4, false, false) + ui->plot->getXUnit());
// Pokud je falltime nabo risetime menší než 2 periody vzorkování, je považován za nepřesný (znaménko menší než)
    ui->labelSig1rise->setText((rise < 2.0 / fs ? "<" : "") + QString::number(rise, 'g', 4) + ui->plot->getXUnit());
    ui->labelSig1fall->setText((fall < 2.0 / fs ? "<" : "") + QString::number(fall, 'g', 4) + ui->plot->getXUnit());
  }

  if (freqUseUnits) {
    ui->labelSig1Freq->setText(floatToNiceString(freq, 4, false, false) + ui->plotFFT->getXUnit());
    ui->labelSig1fs->setText(floatToNiceString(fs, 4, false, false) + ui->plotFFT->getXUnit());
  } else {
    ui->labelSig1Freq->setText(QString::number(freq, 'g', 4) + ui->plotFFT->getXUnit());
    ui->labelSig1fs->setText(QString::number(fs, 'g', 4) + ui->plotFFT->getXUnit());
  }

  ui->labelSig1samples->setText(QString::number(samples));
  measureRefreshTimer1.start(250);
}
void MainWindow::signalMeasurementsResult2(double period, double freq, double amp, double min, double max, double vrms, double dc, double fs, double rise, double fall, int samples) {
  if (recordingOfMeasurements2.isOpen()) {
    char decimal = ui->radioButtonCSVDot->isChecked() ? '.' : ',';
    char separator = ui->radioButtonCSVDot->isChecked() ? ',' : ';';

    recordingOfMeasurements2.write(QString::number(uptime.elapsed() / 1000.0, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(period, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(freq, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(amp, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(min, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(max, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(vrms, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(dc, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(fs, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(rise, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(fall, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write(QByteArray(&separator, 1));
    recordingOfMeasurements2.write(QString::number(samples, 'f', ui->spinBoxCSVPrecision->value()).toLocal8Bit().replace('.', decimal));
    recordingOfMeasurements2.write("\n");
  }

  if (valuesUseUnits) {
    ui->labelSig2Vrms->setText(floatToNiceString(vrms, 4, false, false) + ui->plot->getYUnit());
    ui->labelSig2Min->setText(floatToNiceString(min, 4, false, false) + ui->plot->getYUnit());
    ui->labelSig2Max->setText(floatToNiceString(max, 4, false, false) + ui->plot->getYUnit());
    ui->labelSig2Dc->setText(floatToNiceString(dc, 4, false, false) + ui->plot->getYUnit());
    ui->labelSig2Amp->setText(floatToNiceString(amp, 4, false, false) + (ui->plot->getYUnit() == "V" ? "Vpp" : ui->plot->getYUnit()));
  } else {
    ui->labelSig2Vrms->setText(QString::number(vrms, 'g', 4) + ui->plot->getYUnit());
    ui->labelSig2Min->setText(QString::number(min, 'g', 4) + ui->plot->getYUnit());
    ui->labelSig2Max->setText(QString::number(max, 'g', 4) + ui->plot->getYUnit());
    ui->labelSig2Dc->setText(QString::number(dc, 'g', 4) + ui->plot->getYUnit());
    ui->labelSig2Amp->setText(QString::number(amp, 'g', 4) + ui->plot->getYUnit());
  }

  if (timeUseUnits) {
    ui->labelSig2Period->setText(floatToNiceString(period, 4, false, false) + ui->plot->getXUnit());
    // Pokud je falltime nabo risetime menší než 2 periody vzorkování, je považován za nepřesný (znaménko menší než)
    ui->labelSig2rise->setText((rise < 2.0 / fs ? "<" : "") + floatToNiceString(rise, 4, false, false) + ui->plot->getXUnit());
    ui->labelSig2fall->setText((fall < 2.0 / fs ? "<" : "") + floatToNiceString(fall, 4, false, false) + ui->plot->getXUnit());
  } else {
    ui->labelSig2Period->setText(floatToNiceString(period, 4, false, false) + ui->plot->getXUnit());
    // Pokud je falltime nabo risetime menší než 2 periody vzorkování, je považován za nepřesný (znaménko menší než)
    ui->labelSig2rise->setText((rise < 2.0 / fs ? "<" : "") + QString::number(rise, 'g', 4) + ui->plot->getXUnit());
    ui->labelSig2fall->setText((fall < 2.0 / fs ? "<" : "") + QString::number(fall, 'g', 4) + ui->plot->getXUnit());
  }

  if (freqUseUnits) {
    ui->labelSig2Freq->setText(floatToNiceString(freq, 4, false, false) + ui->plotFFT->getXUnit());
    ui->labelSig2fs->setText(floatToNiceString(fs, 4, false, false) + ui->plotFFT->getXUnit());
  } else {
    ui->labelSig2Freq->setText(QString::number(freq, 'g', 4) + ui->plotFFT->getXUnit());
    ui->labelSig2fs->setText(QString::number(fs, 'g', 4) + ui->plotFFT->getXUnit());
  }

  ui->labelSig2samples->setText(QString::number(samples));
  measureRefreshTimer2.start(250);
}

void MainWindow::fftResult1(QSharedPointer<QCPGraphDataContainer> data) {
  if (ui->pushButtonFFT->isChecked())
    ui->plotFFT->newData(0, data);

  // Výběr min nfft se změní na délku tohoto výsledku, pro polovinu spektra délka neodpovídá (je o 1 vyšší než polovina skutečné),
  // ale to se zaokrouhlí (na nejbližší vyšší mocninu dvou) ve funkci setValue, takže se prakticky zdvojnásobí na správnou hodnotu.
  //ui->spinBoxFFTSamples1->setValue(data->size());

  fftTimer1.start();
}

void MainWindow::fftResult2(QSharedPointer<QCPGraphDataContainer> data) {
  if (ui->pushButtonFFT->isChecked())
    ui->plotFFT->newData(1, data);

  // Výběr min nfft se změní na délku tohoto výsledku, pro polovinu spektra délka neodpovídá (je o 1 vyšší než polovina skutečné),
  // ale to se zaokrouhlí (na nejbližší vyšší mocninu dvou) ve funkci setValue, takže se prakticky zdvojnásobí na správnou hodnotu.
  //ui->spinBoxFFTSamples2->setValue(data->size());

  fftTimer2.start();
}

void MainWindow::xyResult(QSharedPointer<QCPCurveDataContainer> data) {
  if (ui->pushButtonXY->isChecked())
    ui->plotxy->newData(data);
  xyTimer.start();
}

void MainWindow::on_listWidgetTerminalCodeList_itemClicked(QListWidgetItem* item) {
  QString code = "";
  if (item->text().contains(" "))
    code = item->text().left(item->text().indexOf(" "));
  else
    code = item->text();

  if (code.at(0) == '\\')
    insertInTerminalDebug(QString(code.left(2)), Qt::blue);

  else if (code == "3?m") {
    QColor color = QColorDialog::getColor(Qt::white);
    if (!color.isValid())
      return;
    QByteArray colorCode = ansiTerminalModel.nearestColorCode(color);
    insertInTerminalDebug(QString("\\e[3" + colorCode + "m").toUtf8(), Qt::red);
  }

  else if (code == "4?m") {
    QColor color = QColorDialog::getColor(Qt::black);
    if (!color.isValid())
      return;
    QByteArray colorCode = ansiTerminalModel.nearestColorCode(color);
    insertInTerminalDebug(QString("\\e[4" + colorCode + "m").toUtf8(), Qt::red);
  }

  else
    insertInTerminalDebug(QString("\\e[" + code).toUtf8(), Qt::red);
}

void MainWindow::on_pushButtonFFT_toggled(bool checked) {
  on_checkBoxFFTCh1_toggled(ui->checkBoxFFTCh1->isChecked());
  on_checkBoxFFTCh2_toggled(ui->checkBoxFFTCh2->isChecked());

  if (!checked) {
    ui->plotFFT->clear(0);
    ui->plotFFT->clear(1);
  } else {
    updateFFT1();
    updateFFT2();
  }

  if (checked && ui->radioButtonLayoutTime->isChecked())
    ui->radioButtonLayoutAll->setChecked(true);
  else if (!ui->pushButtonXY->isChecked() && ui->radioButtonLayoutAll->isChecked())
    ui->radioButtonLayoutTime->setChecked(true);
}

void MainWindow::on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1) {
  ui->doubleSpinBoxChOffset->setSingleStep(pow(10.0, log10(arg1) - 2));
  ui->doubleSpinBoxYCur1->setSingleStep(pow(10.0, floor(log10(arg1)) - 2));
  ui->doubleSpinBoxYCur2->setSingleStep(pow(10.0, floor(log10(arg1)) - 2));
}

void MainWindow::on_pushButtonClearAll_clicked() {
  ui->plot->resetChannels();
  emit resetChannels();
  emit resetAverager();
}

void MainWindow::on_pushButtonChangeXYColor_clicked() {
  QColor color = QColorDialog::getColor(ui->plotxy->graphXY->pen().color());
  if (color.isValid())
    ui->plotxy->graphXY->setPen(QColor(color));
}

void MainWindow::on_pushButtonTerminalDebugSend_clicked() {
  QByteArray data = ui->textEditTerminalDebug->toPlainText().toUtf8();
  data.replace("\n", "\r\n"); // Odřádkování v textovém poli
  data.replace("\\n", "\n");
  data.replace("\\e", "\u001b");
  data.replace("\\r", "\r");
  data.replace("\\t", "\t");
  data.replace("\\b", "\b");
  data.replace("\\a", "\a");

  ansiTerminalModel.printToTerminal(data);
}

void MainWindow::on_textEditTerminalDebug_cursorPositionChanged() {
  if (ui->textEditTerminalDebug->textCursor().selectedText().isEmpty())
    ui->textEditTerminalDebug->setTextColor(Qt::black);
}

void MainWindow::on_myTerminal_cellClicked(int row, int column) {
  if (ui->pushButtonTerminalDebug->isChecked())
    insertInTerminalDebug(QString("\\e[%1;%2H").arg(row + 1).arg(column + 1), Qt::red);
}

void MainWindow::on_comboBoxFFTType_currentIndexChanged(int index) {
  if (index != FFTType::spectrum) {
    ui->plotFFT->setYUnit("dB", false);
    if (IS_FFT_INDEX(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setSuffix("dB");
    if (IS_FFT_INDEX(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setSuffix("dB");
  } else {
    ui->plotFFT->setYUnit(ui->plot->getYUnit(), false);
    if (IS_FFT_INDEX(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setSuffix("");
    if (IS_FFT_INDEX(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setSuffix("");
  } ui->spinBoxFFTSegments1->setVisible(index == FFTType::pwelch);
  ui->spinBoxFFTSegments2->setVisible(index == FFTType::pwelch);
}

void MainWindow::on_lineEditVUnit_textChanged(const QString& arg1) {
  QString unit = arg1.simplified();

  QString prefixChars = "munkMG";

  valuesUseUnits = true;
  if (unit.isEmpty())
    valuesUseUnits = false;
  if (unit.length() >= 2) {
    if (unit.left(2) == "dB" || prefixChars.contains(unit.at(0))) {
      valuesUseUnits = false;
      unit.push_front(' ');
    }
  }

  ui->plot->setYUnit(unit, valuesUseUnits);
  ui->plotFFT->setYUnit(unit, valuesUseUnits);
  ui->plotxy->setYUnit(unit, valuesUseUnits);
  ui->plotxy->setXUnit(unit, valuesUseUnits);
  ui->doubleSpinBoxRangeVerticalRange->setUnit(unit, valuesUseUnits);
  ui->doubleSpinBoxChOffset->setUnit(unit, valuesUseUnits);
  ui->doubleSpinBoxYCur1->setUnit(unit, valuesUseUnits);
  ui->doubleSpinBoxYCur2->setUnit(unit, valuesUseUnits);
  ui->doubleSpinBoxXYCurX1->setUnit(unit, valuesUseUnits);
  ui->doubleSpinBoxXYCurX2->setUnit(unit, valuesUseUnits);
  ui->doubleSpinBoxXYCurY1->setUnit(unit, valuesUseUnits);
  ui->doubleSpinBoxXYCurY2->setUnit(unit, valuesUseUnits);

  updateDivs(); // Aby se aktualizovala jednotka u kroku mřížky
  updateChScale(); // Aby se aktualizovala jednotka u měřítka
}

void MainWindow::on_checkBoxOpenGL_toggled(bool checked) {
  ui->plot->setOpenGl(checked);
}

void MainWindow::on_checkBoxMouseControls_toggled(bool checked) {
  ui->plot->enableMouseCursorControll(checked);
  ui->plotxy->enableMouseCursorControll(checked);
  ui->plotFFT->enableMouseCursorControll(checked);
  ui->plotPeak->enableMouseCursorControll(checked);
}

void MainWindow::on_checkBoxFFTCh1_toggled(bool checked) {
  setComboboxItemVisible(*ui->comboBoxCursor1Channel, FFT_INDEX(0), checked && ui->pushButtonFFT->isChecked());
  setComboboxItemVisible(*ui->comboBoxCursor2Channel, FFT_INDEX(0), checked && ui->pushButtonFFT->isChecked());
}

void MainWindow::on_checkBoxFFTCh2_toggled(bool checked) {
  setComboboxItemVisible(*ui->comboBoxCursor1Channel, FFT_INDEX(1), checked && ui->pushButtonFFT->isChecked());
  setComboboxItemVisible(*ui->comboBoxCursor2Channel, FFT_INDEX(1), checked && ui->pushButtonFFT->isChecked());
}

void MainWindow::on_pushButtonDolarNewline_toggled(bool checked) {
  QString str = ui->plainTextEditConsole_3->toPlainText();
  ui->plainTextEditConsole_3->clear();
  ui->plainTextEditConsole_3->setLineWrapMode(checked ? QPlainTextEdit::LineWrapMode::NoWrap : QPlainTextEdit::LineWrapMode::WidgetWidth);
  str.replace("\n$$","$$");
  if(checked)
    str.replace("$$","\n$$");
  if(str.left(1)=="\n")
    str.remove(0,1);
  ui->plainTextEditConsole_3->setPlainText(str);
  on_pushButtonScrollDown_3_clicked();
}

void MainWindow::on_pushButtonInterpolate_toggled(bool checked) {
  int chid = ui->comboBoxSelectedChannel->currentIndex();

  if (chid < ANALOG_COUNT + MATH_COUNT) { //Když není vybrán analogový, nemělo by být možné zaškrtnout, ale pro jistotu...
    if (checked && ui->comboBoxGraphStyle->currentIndex() == GraphStyle::linePoint)
      ui->comboBoxGraphStyle->setCurrentIndex(GraphStyle::point); // Radši přepne na styl point, aby nebyl zmatek, že v linepoint se pořád spojují body
    ui->plot->setChInterpolate(chid, checked);
    emit setInterpolation(chid, checked);
  }
}

void MainWindow::on_pushButtonTerminalBlacklisAddSelect_clicked()
{
    QColor color = QColorDialog::getColor(Qt::black);
    if (!color.isValid())
      return;
    QByteArray colorCode = ansiTerminalModel.nearestColorCode(color);
    addColorToBlacklist(QString("\\e[3" + colorCode + "m").toUtf8());
    updateColorBlacklist();
}