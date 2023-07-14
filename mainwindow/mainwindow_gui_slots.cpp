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
#include "ui_freqtimeplotdialog.h"

void MainWindow::setPlotLayout(QString type) {
  bool fft = ui->pushButtonFFT->isChecked();
  bool xy = ui->pushButtonXY->isChecked();

  if (type == "all") {
    ui->plot->setVisible(true);
    ui->plotFFT->setVisible(fft);
    ui->plotxy->setVisible(xy);
    ui->frameUpperPlots->setVisible(fft || xy);
    ui->pushButtonFFT_Maximize->setVisible(true);
    ui->pushButtonXY_Maximize->setVisible(true);
    hasMaximizedPlot = false;
    return;
  }

  if (type == "fft") {
    ui->plot->setVisible(false);
    ui->plotFFT->setVisible(true);
    ui->plotxy->setVisible(false);
    ui->frameUpperPlots->setVisible(true);
  } else if (type == "xy") {
    ui->plot->setVisible(false);
    ui->plotFFT->setVisible(false);
    ui->plotxy->setVisible(true);
    ui->frameUpperPlots->setVisible(true);
  } else
    Q_ASSERT(true);
  hasMaximizedPlot = true;
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
      }
    }
    ui->plot->setLogicStyle(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, index);
  }
}

void MainWindow::on_pushButtonConnect_clicked() {
  SerialSettingsDialog::Settings settings = serialSettingsDialog->settings();
  if (ui->listWidgetCom->currentItem() != NULL)
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

void MainWindow::on_radioButtonEn_toggled(bool checked) {
  if (checked)
    changeLanguage("en");
}

void MainWindow::on_radioButtonCz_toggled(bool checked) {
  if (checked)
    changeLanguage("cz");
}

void MainWindow::on_pushButtonScrollDown_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
};

void MainWindow::on_comboBoxOutputLevel_currentIndexChanged(int index) {
  if (index >= 0)
    emit setSerialMessageLevel((OutputLevel::enumOutputLevel)index);
}

void MainWindow::on_pushButtonScrollDown_3_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole_3->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole_3->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
}

void MainWindow::on_comboBoxSelectedChannel_currentIndexChanged(int index) {
  // Zablokuje signáli, aby se po nastavení sočasných rovnou neposlali současné
  // hodnoty jako změna.
  ui->doubleSpinBoxChOffset->blockSignals(true);
  ui->doubleSpinBoxChScale->blockSignals(true);
  // ui->dialChScale->blockSignals(true);
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
    // ui->dialChScale->updatePosition(scale);
    ui->pushButtonHideCh->setChecked(!ui->plot->isLogicVisible(group));
  } else {
    setChStyleSelection(index >= ANALOG_COUNT ? GraphType::math : GraphType::analog);
    ui->comboBoxGraphStyle->setCurrentIndex(ui->plot->getChStyle(index));
    double offset = ui->plot->getChOffset(index);
    double scale = ui->plot->getChScale(index);
    ui->doubleSpinBoxChOffset->setValue(offset);
    ui->doubleSpinBoxChScale->setValue(scale);
    // ui->dialChScale->updatePosition(scale);
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
  // ui->dialChScale->blockSignals(false);
  ui->pushButtonHideCh->blockSignals(false);
  ui->comboBoxGraphStyle->blockSignals(false);
  ui->pushButtonInvert->blockSignals(false);
}

void MainWindow::on_pushButtonInvert_toggled(bool checked) {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChInvert(ui->comboBoxSelectedChannel->currentIndex(), checked);
}

void MainWindow::on_pushButtonOpenHelpCZ_clicked() { openResourceFileCopiedToLocal(":/docs/documentation/Manual.pdf"); }

void MainWindow::on_pushButtonProtocolGuideEN_clicked() { openResourceFileCopiedToLocal(":/docs/documentation/Data protocol guide en.pdf"); }

void MainWindow::on_pushButtonIntroVideoCZ_clicked() { QDesktopServices::openUrl(QUrl("https://www.youtube.com/watch?v=TpJgz6kfPvA")); }

void MainWindow::on_labelLogo_clicked() { QDesktopServices::openUrl(QUrl("https://embedded.fel.cvut.cz/platformy")); }

void MainWindow::on_comboBoxFIR_currentIndexChanged(int index) {
  switch (index) {
  case 0:
    emit setInterpolationFilter("hamm_x8", 8);
    break;
  case 1:
    emit setInterpolationFilter("kaiser_x8", 8);
    break;
  case 2:
    emit setInterpolationFilter("kaiser_x16", 16);
    break;
  case 3:
    emit setInterpolationFilter("kaiser_x32", 32);
    break;
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
    ui->plot->setChColor(ui->comboBoxSelectedChannel->currentIndex(), color, currentThemeDark ? 2 : 1);
  else
    ui->plot->setLogicColor(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, color, currentThemeDark ? 2 : 1);
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

  setPlotLayout("all");
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
    ui->labelSig1Vrms->setText(floatToNiceString(vrms, 4, false, false) + ui->plot->getYUnit().text);
    ui->labelSig1Min->setText(floatToNiceString(min, 4, false, false) + ui->plot->getYUnit().text);
    ui->labelSig1Max->setText(floatToNiceString(max, 4, false, false) + ui->plot->getYUnit().text);
    ui->labelSig1Dc->setText(floatToNiceString(dc, 4, false, false) + ui->plot->getYUnit().text);
    ui->labelSig1Amp->setText(floatToNiceString(amp, 4, false, false) + (ui->plot->getYUnit().text == "V" ? "Vpp" : ui->plot->getYUnit().text));
  } else {
    ui->labelSig1Vrms->setText(QString::number(vrms, 'g', 4) + ui->plot->getYUnit().text);
    ui->labelSig1Min->setText(QString::number(min, 'g', 4) + ui->plot->getYUnit().text);
    ui->labelSig1Max->setText(QString::number(max, 'g', 4) + ui->plot->getYUnit().text);
    ui->labelSig1Dc->setText(QString::number(dc, 'g', 4) + ui->plot->getYUnit().text);
    ui->labelSig1Amp->setText(QString::number(amp, 'g', 4) + ui->plot->getYUnit().text);
  }

  if (timeUseUnits) {
    ui->labelSig1Period->setText(floatToNiceString(period, 4, false, false) + ui->plot->getXUnit().text);
    // Pokud je falltime nabo risetime menší než 2 periody vzorkování, je
    // považován za nepřesný (znaménko menší než)
    ui->labelSig1rise->setText((rise < 2.0 / fs ? "<" : "") + floatToNiceString(rise, 4, false, false) + ui->plot->getXUnit().text);
    ui->labelSig1fall->setText((fall < 2.0 / fs ? "<" : "") + floatToNiceString(fall, 4, false, false) + ui->plot->getXUnit().text);
  } else {
    ui->labelSig1Period->setText(floatToNiceString(period, 4, false, false) + ui->plot->getXUnit().text);
    // Pokud je falltime nabo risetime menší než 2 periody vzorkování, je
    // považován za nepřesný (znaménko menší než)
    ui->labelSig1rise->setText((rise < 2.0 / fs ? "<" : "") + QString::number(rise, 'g', 4) + ui->plot->getXUnit().text);
    ui->labelSig1fall->setText((fall < 2.0 / fs ? "<" : "") + QString::number(fall, 'g', 4) + ui->plot->getXUnit().text);
  }

  if (freqUseUnits) {
    ui->labelSig1Freq->setText(floatToNiceString(freq, 4, false, false) + ui->plotFFT->getXUnit().text);
    ui->labelSig1fs->setText(floatToNiceString(fs, 4, false, false) + ui->plotFFT->getXUnit().text);
  } else {
    ui->labelSig1Freq->setText(QString::number(freq, 'g', 4) + ui->plotFFT->getXUnit().text);
    ui->labelSig1fs->setText(QString::number(fs, 'g', 4) + ui->plotFFT->getXUnit().text);
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
    ui->labelSig2Vrms->setText(floatToNiceString(vrms, 4, false, false) + ui->plot->getYUnit().text);
    ui->labelSig2Min->setText(floatToNiceString(min, 4, false, false) + ui->plot->getYUnit().text);
    ui->labelSig2Max->setText(floatToNiceString(max, 4, false, false) + ui->plot->getYUnit().text);
    ui->labelSig2Dc->setText(floatToNiceString(dc, 4, false, false) + ui->plot->getYUnit().text);
    ui->labelSig2Amp->setText(floatToNiceString(amp, 4, false, false) + (ui->plot->getYUnit().text == "V" ? "Vpp" : ui->plot->getYUnit().text));
  } else {
    ui->labelSig2Vrms->setText(QString::number(vrms, 'g', 4) + ui->plot->getYUnit().text);
    ui->labelSig2Min->setText(QString::number(min, 'g', 4) + ui->plot->getYUnit().text);
    ui->labelSig2Max->setText(QString::number(max, 'g', 4) + ui->plot->getYUnit().text);
    ui->labelSig2Dc->setText(QString::number(dc, 'g', 4) + ui->plot->getYUnit().text);
    ui->labelSig2Amp->setText(QString::number(amp, 'g', 4) + ui->plot->getYUnit().text);
  }

  if (timeUseUnits) {
    ui->labelSig2Period->setText(floatToNiceString(period, 4, false, false) + ui->plot->getXUnit().text);
    // Pokud je falltime nabo risetime menší než 2 periody vzorkování, je
    // považován za nepřesný (znaménko menší než)
    ui->labelSig2rise->setText((rise < 2.0 / fs ? "<" : "") + floatToNiceString(rise, 4, false, false) + ui->plot->getXUnit().text);
    ui->labelSig2fall->setText((fall < 2.0 / fs ? "<" : "") + floatToNiceString(fall, 4, false, false) + ui->plot->getXUnit().text);
  } else {
    ui->labelSig2Period->setText(floatToNiceString(period, 4, false, false) + ui->plot->getXUnit().text);
    // Pokud je falltime nabo risetime menší než 2 periody vzorkování, je
    // považován za nepřesný (znaménko menší než)
    ui->labelSig2rise->setText((rise < 2.0 / fs ? "<" : "") + QString::number(rise, 'g', 4) + ui->plot->getXUnit().text);
    ui->labelSig2fall->setText((fall < 2.0 / fs ? "<" : "") + QString::number(fall, 'g', 4) + ui->plot->getXUnit().text);
  }

  if (freqUseUnits) {
    ui->labelSig2Freq->setText(floatToNiceString(freq, 4, false, false) + ui->plotFFT->getXUnit().text);
    ui->labelSig2fs->setText(floatToNiceString(fs, 4, false, false) + ui->plotFFT->getXUnit().text);
  } else {
    ui->labelSig2Freq->setText(QString::number(freq, 'g', 4) + ui->plotFFT->getXUnit().text);
    ui->labelSig2fs->setText(QString::number(fs, 'g', 4) + ui->plotFFT->getXUnit().text);
  }

  ui->labelSig2samples->setText(QString::number(samples));
  measureRefreshTimer2.start(250);
}

void MainWindow::fftResult1(QSharedPointer<QCPGraphDataContainer> data) {
  if (ui->pushButtonFFT->isChecked())
    ui->plotFFT->newData(0, data);

  // Výběr min nfft se změní na délku tohoto výsledku, pro polovinu spektra
  // délka neodpovídá (je o 1 vyšší než polovina skutečné), ale to se zaokrouhlí
  // (na nejbližší vyšší mocninu dvou) ve funkci setValue, takže se prakticky
  // zdvojnásobí na správnou hodnotu.
  // ui->spinBoxFFTSamples1->setValue(data->size());

  fftTimer1.start();
}

void MainWindow::fftResult2(QSharedPointer<QCPGraphDataContainer> data) {
  if (ui->pushButtonFFT->isChecked())
    ui->plotFFT->newData(1, data);

  // Výběr min nfft se změní na délku tohoto výsledku, pro polovinu spektra
  // délka neodpovídá (je o 1 vyšší než polovina skutečné), ale to se zaokrouhlí
  // (na nejbližší vyšší mocninu dvou) ve funkci setValue, takže se prakticky
  // zdvojnásobí na správnou hodnotu.
  // ui->spinBoxFFTSamples2->setValue(data->size());

  fftTimer2.start();
}

void MainWindow::xyResult(QSharedPointer<QCPCurveDataContainer> data) {
  if (ui->pushButtonXY->isChecked())
    ui->plotxy->newData(data);
  xyTimer.start();
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

  ui->plotFFT->setOutputPeakValue(false);
  setPlotLayout("all");
}

void MainWindow::on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1) {
  ui->pushButtonRollingAutoRange->setChecked(false);
  ui->dialVerticalRange->updatePosition(arg1);
  auto range = ui->plot->getMaxZoomY();
  if (qFuzzyIsNull(range.lower))
    range.upper = arg1;
  else if (qFuzzyIsNull(range.upper))
    range.lower = -arg1;
  else
    range = QCPRange(range.center() - arg1 / 2, range.center() + arg1 / 2);

  ui->doubleSpinBoxViewCenter->blockSignals(true);
  ui->doubleSpinBoxViewCenter->setValue(range.center());
  ui->doubleSpinBoxViewCenter->blockSignals(false);

  ui->plot->setVRange(range);
}

void MainWindow::pushButtonClearAll_clicked() {
  ui->plot->resetChannels();
  emit resetChannels();
  emit resetAverager();
}

void MainWindow::on_pushButtonChangeXYColor_clicked() {
  QColor color = QColorDialog::getColor(ui->plotxy->graphXY->pen().color());
  if (color.isValid())
    ui->plotxy->setColor(color, currentThemeDark ? 2 : 1);
}

void MainWindow::on_comboBoxFFTType_currentIndexChanged(int index) {
  if (index != FFTType::spectrum) {
    ui->plotFFT->setYUnit(UnitOfMeasure("dB"));
    if (IS_FFT_INDEX(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setSuffix("dB");
    if (IS_FFT_INDEX(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setSuffix("dB");
  } else {
    ui->plotFFT->setYUnit(ui->plot->getYUnit());
    if (IS_FFT_INDEX(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setSuffix("");
    if (IS_FFT_INDEX(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setSuffix("");
  }

  ui->spinBoxFFTSegments1->setEnabled(true);
  ui->spinBoxFFTSegments2->setEnabled(true);
  ui->spinBoxFFTSegments1->setVisible(index == FFTType::pwelch);
  ui->spinBoxFFTSegments2->setVisible(index == FFTType::pwelch);
}

void MainWindow::on_lineEditVUnit_textChanged(const QString &arg1) {
  QString unitRaw = arg1.simplified();
  UnitOfMeasure unit(unitRaw);

  ui->plot->setYUnit(unit);
  ui->plotFFT->setYUnit(unit);
  ui->plotxy->setYUnit(unit);
  ui->plotxy->setXUnit(unit);
  ui->doubleSpinBoxRangeVerticalRange->setUnit(unit);
  ui->doubleSpinBoxChOffset->setUnit(unit);
  ui->doubleSpinBoxYCur1->setUnit(unit);
  ui->doubleSpinBoxYCur2->setUnit(unit);
  ui->doubleSpinBoxXYCurX1->setUnit(unit);
  ui->doubleSpinBoxXYCurX2->setUnit(unit);
  ui->doubleSpinBoxXYCurY1->setUnit(unit);
  ui->doubleSpinBoxXYCurY2->setUnit(unit);
  ui->doubleSpinBoxViewCenter->setUnit(unit);

  updateDivs();    // Aby se aktualizovala jednotka u kroku mřížky
  updateChScale(); // Aby se aktualizovala jednotka u měřítka
}

void MainWindow::on_checkBoxOpenGL_toggled(bool checked) { ui->plot->setOpenGl(checked); }

void MainWindow::checkBoxMouseControls_toggled_new(bool checked) {
  ui->plot->enableMouseCursorControll(checked);
  ui->plotxy->enableMouseCursorControll(checked);
  ui->plotFFT->enableMouseCursorControll(checked);
  freqTimePlotDialog->getUi()->plotPeak->enableMouseCursorControll(checked);
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
  str.replace("\n$$", "$$");
  if (checked)
    str.replace("$$", "\n$$");
  if (str.at(0) == "\n")
    str.remove(0, 1);
  ui->plainTextEditConsole_3->setPlainText(str);
  on_pushButtonScrollDown_3_clicked();
}

void MainWindow::on_pushButtonInterpolate_toggled(bool checked) {
  int chid = ui->comboBoxSelectedChannel->currentIndex();

  if (chid < ANALOG_COUNT + MATH_COUNT) { // Když není vybrán analogový, nemělo by být
                                          // možné zaškrtnout, ale pro jistotu...
    if (checked && ui->comboBoxGraphStyle->currentIndex() == GraphStyle::linePoint)
      ui->comboBoxGraphStyle->setCurrentIndex(GraphStyle::point); // Radši přepne na styl point, aby nebyl zmatek,
                                                                  // že v linepoint se pořád spojují body
    ui->plot->setChInterpolate(chid, checked);
    emit setInterpolation(chid, checked);
  }
}

void MainWindow::on_tabs_right_currentChanged(int index) {
  // TODO
  Q_UNUSED(index);
#if QT_VERSION < 0x050C00
  if (index == -1)
    on_radioButtonDark_toggled(ui->radioButtonDark->isChecked());
#endif
}

void MainWindow::on_pushButtonRangeFit_clicked() {
  on_pushButtonResetChannels_clicked();
  QCPRange chRange;
  bool zbytrecnaPromena;
  chRange = ui->plot->graph(ui->comboBoxSelectedChannel->currentIndex())->getValueRange(zbytrecnaPromena, QCP::sdBoth, ui->plot->xAxis->range());
  ui->doubleSpinBoxViewCenter->setValue(chRange.center());
  ui->doubleSpinBoxRangeVerticalRange->setValue(ceilToNiceValue(chRange.size()));
}

void MainWindow::on_listWidgetCom_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
  Q_UNUSED(previous);
  if (current == NULL) {
    emit disconnectSerial();
    return;
  }

  SerialSettingsDialog::Settings settings = serialSettingsDialog->settings();
  attemptReconnectPort = "";
  emit beginSerialConnection(current->data(Qt::UserRole).toString(), ui->comboBoxBaud->currentText().toInt(), settings.dataBits, settings.parity, settings.stopBits, settings.flowControl);
}

void MainWindow::on_pushButtonSetPositive_clicked() { ui->doubleSpinBoxViewCenter->setValue(ui->doubleSpinBoxRangeVerticalRange->value() / 2); }

void MainWindow::on_pushButtonSetCenter_clicked() { ui->doubleSpinBoxViewCenter->setValue(0); }

void MainWindow::on_pushButtonSetNegative_clicked() { ui->doubleSpinBoxViewCenter->setValue(-ui->doubleSpinBoxRangeVerticalRange->value() / 2); }

void MainWindow::on_pushButtonFvsT_clicked() {
  ui->spinBoxFFTSamples1->setValue(ui->spinBoxFFTSamples1->maximum());
  ui->spinBoxFFTSamples2->setValue(ui->spinBoxFFTSamples2->maximum());
  ui->checkBoxFFTNoDC1->setChecked(true);
  ui->checkBoxFFTNoDC2->setChecked(true);
  ui->pushButtonFFT->setChecked(true);
  freqTimePlotDialog->show();
  ui->plotFFT->setOutputPeakValue(true);
}

void MainWindow::on_pushButtonSerialMonitor_toggled(bool checked) {
  ui->frameSerialMonitor->setEnabled(checked);
  emit enableSerialMonitor(checked);
}

void MainWindow::on_comboBoxXYStyle_currentIndexChanged(int index) { ui->plotxy->setStyle(index); }

void MainWindow::on_comboBoxFFTStyle1_currentIndexChanged(int index) { ui->plotFFT->setStyle(0, index); }

void MainWindow::on_comboBoxFFTStyle2_currentIndexChanged(int index) { ui->plotFFT->setStyle(1, index); }

void MainWindow::on_pushButtonModeRolling_clicked() {
  double hLen = ui->doubleSpinBoxRangeHorizontal->value();
  ui->pushButtonModeFull->setChecked(false);
  ui->pushButtonModeRolling->setChecked(true);
  ui->frameZoom->setVisible(false);
  ui->frameRollingRange->setVisible(true);
  ui->plot->setRollingMode(true);
  ui->plot->setHLen(hLen);
}

void MainWindow::on_pushButtonModeFull_clicked() {
  ui->pushButtonModeRolling->setChecked(false);
  ui->pushButtonModeFull->setChecked(true);
  ui->frameRollingRange->setVisible(false);
  ui->frameZoom->setVisible(true);
  ui->plot->setRollingMode(false);
}

void MainWindow::on_doubleSpinBoxRangeHorizontal_valueChanged(double arg1) {
  ui->plot->setHLen(arg1);
  ui->dialRollingRange->updatePosition(arg1);
}

void MainWindow::on_doubleSpinBoxViewCenter_valueChanged(double arg1) {
  ui->pushButtonRollingAutoRange->setChecked(false);
  auto range = ui->plot->getMaxZoomY();
  range = QCPRange(arg1 - range.size() / 2, arg1 + range.size() / 2);
  ui->plot->setVRange(range);
}

void MainWindow::on_dialZoom_valueChanged(int position) {
  double range = static_cast<double>(position) / ui->dialZoom->maximum() * (ui->plot->getMaxT() - ui->plot->getMinT());
  ui->plot->setHLen(range);
}

void MainWindow::on_listWidgetCom_itemClicked(QListWidgetItem *item) {
  if (item->data(Qt::UserRole) == "~SPECIAL~SIM")
    simulatedInputDialog->open();
  else
    simulatedInputDialog->close();
}

void MainWindow::on_pushButtonFFT_Maximize_clicked() { plotMaximizeButtonClicked("fft"); }

void MainWindow::on_pushButtonXY_Maximize_clicked() { plotMaximizeButtonClicked("xy"); }

void MainWindow::on_verticalScrollBarVertical_valueChanged(int value) {
  int m = ui->dialZoom->maximum();
  auto fr = ui->plot->getMaxZoomY();
  ui->plot->setVPos(fr.lower + (fr.size()) / m * value);
}

void MainWindow::on_horizontalScrollBarHorizontal_valueChanged(int value) {
  int m = ui->dialZoom->maximum();
  double minT = ui->plot->getMinT();
  double maxT = ui->plot->getMaxT();
  ui->plot->setHPos(minT + (maxT - minT) / m * value);
}

void MainWindow::on_pushButtonRollingAutoRange_toggled(bool checked) { ui->plot->setAutoVRage(checked); }
