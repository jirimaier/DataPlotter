#include "mainwindow.h"

void MainWindow::on_pushButtonConnect_clicked() {
  if (ui->comboBoxCom->currentIndex() >= 0) {
    emit toggleSerialConnection(portList.at(ui->comboBoxCom->currentIndex()).portName(), ui->comboBoxBaud->currentText().toInt());
  }
}

void MainWindow::on_tabs_right_currentChanged(int index) {
  if (index == 2)
    ui->lineEditCommand->setFocus();
}

void MainWindow::on_pushButtonChannelColor_clicked() {
  QColor color = QColorDialog::getColor(ui->plot->getChColor(ui->comboBoxSelectedChannel->currentIndex()));
  if (!color.isValid())
    return;
  ui->plot->setChColor(ui->comboBoxSelectedChannel->currentIndex(), color);
  QPixmap pixmap(30, 30);
  pixmap.fill(color);
  ui->pushButtonChannelColor->setIcon(pixmap);
}

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

void MainWindow::on_doubleSpinBoxChOffset_valueChanged(double arg1) {
  ui->plot->changeChOffset(ui->comboBoxSelectedChannel->currentIndex(), arg1);
  scrollBarCursorValueChanged();
}

void MainWindow::on_comboBoxGraphStyle_currentIndexChanged(int index) {
  ui->plot->setChStyle(ui->comboBoxSelectedChannel->currentIndex(), index);
  ui->plot->updateVisuals();
}
void MainWindow::on_doubleSpinBoxChScale_valueChanged(double arg1) {
  ui->plot->changeChScale(ui->comboBoxSelectedChannel->currentIndex(), arg1 * (ui->checkBoxChInvert->isChecked() ? -1 : 1));
  scrollBarCursorValueChanged();
  updateChScale();
}

void MainWindow::on_pushButtonSelectedCSV_clicked() { exportCSV(false, ui->comboBoxSelectedChannel->currentIndex() - 1); }

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

void MainWindow::on_lineEditManualInput_returnPressed() {
  QByteArray bytes;
  bytes.append(ui->lineEditManualInput->text().toLocal8Bit());

  qDebug() << "Manual input: " << bytes;
  emit sendManualInput(bytes);
  ui->lineEditManualInput->clear();
  on_pushButtonScrollDown_clicked();
}

void MainWindow::on_pushButtonSendCommand_clicked() { on_lineEditCommand_returnPressed(); }

void MainWindow::on_pushButtonCSVXY_clicked() { exportCSV(false, XY_CHANNEL); }

void MainWindow::on_pushButtonPositive_clicked() { ui->dialVerticalCenter->setValue(ui->dialVerticalCenter->maximum()); }
