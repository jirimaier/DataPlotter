//  Copyright (C) 2020  Jiří Maier

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

void MainWindow::on_pushButtonConnect_clicked() {
  if (ui->comboBoxCom->currentIndex() >= 0) {
    emit toggleSerialConnection(portList.at(ui->comboBoxCom->currentIndex()).portName(), ui->comboBoxBaud->currentText().toInt());
  }
}

void MainWindow::on_tabs_right_currentChanged(int index) {
  if (index == 2) ui->lineEditCommand->setFocus();
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
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChOffset(ui->comboBoxSelectedChannel->currentIndex(), arg1);
  else
    ui->plot->setLogicOffset(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, arg1);
}

void MainWindow::on_comboBoxGraphStyle_currentIndexChanged(int index) {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChStyle(ui->comboBoxSelectedChannel->currentIndex(), index);
  else
    ui->plot->setLogicStyle(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, index);
}
void MainWindow::on_doubleSpinBoxChScale_valueChanged(double arg1) {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChScale(ui->comboBoxSelectedChannel->currentIndex(), arg1);
  else
    ui->plot->setLogicScale(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, arg1);
  updateChScale();
}

void MainWindow::on_dialZoom_valueChanged(int value) {
  ui->plot->setZoomRange(value);
  ui->horizontalScrollBarHorizontal->setMinimum(value / 2);
  ui->horizontalScrollBarHorizontal->setMaximum(1000 - value / 2);
  ui->horizontalScrollBarHorizontal->setPageStep(value);
}

void MainWindow::on_radioButtonEn_toggled(bool checked) {
  if (checked) changeLanguage("en");
}

void MainWindow::on_radioButtonCz_toggled(bool checked) {
  if (checked) changeLanguage("cz");
}

void MainWindow::on_lineEditManualInput_returnPressed() {
  QByteArray bytes;
  bytes.append(ui->lineEditManualInput->text().toLocal8Bit());
  emit sendManualInput(bytes);
  ui->lineEditManualInput->clear();
  on_pushButtonScrollDown_clicked();
}

void MainWindow::on_pushButtonScrollDown_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
};

void MainWindow::on_lineEditCommand_returnPressed() {
  QString text = ui->lineEditCommand->text() + Global::lineEndings[ui->comboBoxLineEnding->currentIndex()];
  emit writeToSerial(text.toLocal8Bit());
}

void MainWindow::on_lineEditCommand_2_returnPressed() {
  QString text = ui->lineEditCommand_2->text() + Global::lineEndings[ui->comboBoxLineEnding->currentIndex()];
  emit writeToSerial(text.toLocal8Bit());
}

void MainWindow::on_lineEditCommand_3_returnPressed() {
  QString text = ui->lineEditCommand_3->text() + Global::lineEndings[ui->comboBoxLineEnding->currentIndex()];
  emit writeToSerial(text.toLocal8Bit());
}

void MainWindow::on_lineEditCommand_4_returnPressed() {
  QString text = ui->lineEditCommand_4->text() + Global::lineEndings[ui->comboBoxLineEnding->currentIndex()];
  emit writeToSerial(text.toLocal8Bit());
}

void MainWindow::on_comboBoxOutputLevel_currentIndexChanged(int index) {
  if (index >= 0) emit setSerialMessageLevel((OutputLevel::enumerator)index);
}

void MainWindow::on_pushButtonScrollDown_2_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole_2->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole_2->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
}

void MainWindow::on_pushButtonScrollDown_3_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole_3->verticalScrollBar();
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
  ui->dialChScale->blockSignals(true);
  ui->pushButtonHideCh->blockSignals(true);
  ui->comboBoxGraphStyle->blockSignals(true);
  ui->checkBoxChInverted->blockSignals(true);

  if (index >= ANALOG_COUNT + MATH_COUNT) {
    ui->checkBoxChInverted->setEnabled(false);
    int group = index - ANALOG_COUNT - MATH_COUNT;
    ui->comboBoxGraphStyle->setCurrentIndex(ui->plot->getLogicStyle(group));
    double offset = ui->plot->getLogicOffset(group);
    double scale = ui->plot->getLogicScale(group);
    ui->doubleSpinBoxChOffset->setValue(offset);
    ui->doubleSpinBoxChScale->setValue(scale);
    ui->dialChScale->updatePosition(scale);
    ui->pushButtonHideCh->setChecked(!ui->plot->isLogicVisible(group));
  } else {
    ui->checkBoxChInverted->setEnabled(true);
    ui->comboBoxGraphStyle->setCurrentIndex(ui->plot->getChStyle(index));
    double offset = ui->plot->getChOffset(index);
    double scale = ui->plot->getChScale(index);
    ui->doubleSpinBoxChOffset->setValue(offset);
    ui->doubleSpinBoxChScale->setValue(scale);
    ui->dialChScale->updatePosition(scale);
    ui->checkBoxChInverted->setChecked(ui->plot->isChInverted(index));
    ui->pushButtonHideCh->setChecked(!ui->plot->isChVisible(index));
  }
  updateChScale();

  // Ikona by se sama nezměnila, protože jsou zablokované signály
  if (ui->pushButtonHideCh->isChecked())
    ui->pushButtonHideCh->setIcon(iconHidden);
  else
    ui->pushButtonHideCh->setIcon(iconVisible);

  ui->doubleSpinBoxChOffset->blockSignals(false);
  ui->doubleSpinBoxChScale->blockSignals(false);
  ui->dialChScale->blockSignals(false);
  ui->pushButtonHideCh->blockSignals(false);
  ui->comboBoxGraphStyle->blockSignals(false);
  ui->checkBoxChInverted->blockSignals(false);
}

void MainWindow::on_checkBoxChInverted_toggled(bool checked) {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT) ui->plot->setChInvert(ui->comboBoxSelectedChannel->currentIndex(), checked);
}

void MainWindow::on_pushButtonResetChannels_clicked() {
  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
    ui->plot->setChOffset(i, 0);
    ui->plot->setChScale(i, 1);
    ui->plot->setChStyle(i, GraphStyle::line);
  }
  for (int i = 0; i < LOGIC_GROUPS; i++) {
    ui->plot->setLogicOffset(i, 0);
    ui->plot->setLogicScale(i, 1);
  }
  on_comboBoxSelectedChannel_currentIndexChanged(ui->comboBoxSelectedChannel->currentIndex());
}

void MainWindow::on_comboBoxHAxisType_currentIndexChanged(int index) {
  ui->labelHDiv->setEnabled(index <= 1);
  ui->plot->setShowHorizontalValues(index);
}

void MainWindow::on_pushButtonOpenHelp_clicked() {
  QString helpFile = QCoreApplication::applicationDirPath() + "/Manual.pdf";
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(helpFile))) {
    QMessageBox msgBox;
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
  if (!color.isValid()) return;
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChColor(ui->comboBoxSelectedChannel->currentIndex(), color);
  else
    ui->plot->setLogicColor(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, color);
  colorUpdateNeeded = true;
  updateUsedChannels();
}

void MainWindow::on_dialXYGrid_valueChanged(int value) {
  ui->plotxy->setGridHintX(value);
  ui->plotxy->setGridHintY(value);
}

void MainWindow::on_radioButtonRollingRange_toggled(bool checked) {
  ui->plotxy->setUseTimeRange(checked);
  updateXYNow();
}

void MainWindow::on_pushButtonXY_toggled(bool checked) {
  updateXYNow();

  auto *model = qobject_cast<QStandardItemModel *>(ui->comboBoxCursor1Channel->model());
  auto *item = model->item(XYID);
  item->setEnabled(checked);
  QListView *view = qobject_cast<QListView *>(ui->comboBoxCursor1Channel->view());
  view->setRowHidden(XYID, !checked);

  model = qobject_cast<QStandardItemModel *>(ui->comboBoxCursor2Channel->model());
  item = model->item(XYID);
  item->setEnabled(checked);
  view = qobject_cast<QListView *>(ui->comboBoxCursor2Channel->view());
  view->setRowHidden(XYID, !checked);
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

void MainWindow::on_pushButtonPositive_clicked() {
  if (ui->dialVerticalCenter->value() != ui->dialVerticalCenter->maximum())
    ui->dialVerticalCenter->setValue(ui->dialVerticalCenter->maximum());
  else
    ui->plot->setVerticalCenter(ui->dialVerticalCenter->maximum());
}

void MainWindow::on_pushButtonTerminalDebug_toggled(bool checked) {
  if (checked) {
    ui->pushButtonTerminalClickToSend->blockSignals(true);
    ui->pushButtonTerminalClickToSend->setChecked(false);
    ui->pushButtonTerminalClickToSend->blockSignals(false);
    ui->pushButtonTerminalSelect->blockSignals(true);
    ui->pushButtonTerminalSelect->setChecked(false);
    ui->pushButtonTerminalSelect->blockSignals(false);
    ui->pushButtonTerminalCopy->setEnabled(false);
    ui->frameTermanalDebug->setVisible(true);
    ui->myTerminal->setMode(TerminalMode::debug);
    ui->plainTextEditTerminalDebug->clear();
  } else {
    ui->myTerminal->setMode(TerminalMode::none);
    ui->frameTermanalDebug->setVisible(false);
  }
}

void MainWindow::on_pushButtonTerminalClickToSend_toggled(bool checked) {
  if (checked) {
    ui->pushButtonTerminalSelect->blockSignals(true);
    ui->pushButtonTerminalSelect->setChecked(false);
    ui->pushButtonTerminalSelect->blockSignals(false);
    ui->pushButtonTerminalDebug->blockSignals(true);
    ui->pushButtonTerminalDebug->setChecked(false);
    ui->frameTermanalDebug->setVisible(false);
    ui->pushButtonTerminalDebug->blockSignals(false);
    ui->pushButtonTerminalCopy->setEnabled(false);
    ui->myTerminal->setMode(TerminalMode::clicksend);
  } else
    ui->myTerminal->setMode(TerminalMode::none);
}

void MainWindow::on_pushButtonTerminalSelect_toggled(bool checked) {
  if (checked) {
    ui->pushButtonTerminalClickToSend->blockSignals(true);
    ui->pushButtonTerminalClickToSend->setChecked(false);
    ui->pushButtonTerminalClickToSend->blockSignals(false);
    ui->pushButtonTerminalDebug->blockSignals(true);
    ui->pushButtonTerminalDebug->setChecked(false);
    ui->frameTermanalDebug->setVisible(false);
    ui->pushButtonTerminalDebug->blockSignals(false);
    ui->pushButtonTerminalCopy->setEnabled(true);
    ui->myTerminal->setMode(TerminalMode::select);
  } else {
    ui->pushButtonTerminalCopy->setEnabled(false);
    ui->myTerminal->setMode(TerminalMode::none);
  }
}

void MainWindow::printTerminalDebug(QString text) {
  QTextCursor prevCursor = ui->plainTextEditTerminalDebug->textCursor();
  text.replace(" ", "&nbsp;");                    // Normální mezera se nezobrazí :-(
  text.replace("font&nbsp;color", "font color");  // A když už tam tu mezeru změním, tek to zase zničí tu mezeru v "font color" a nefunguje to.
  ui->plainTextEditTerminalDebug->moveCursor(QTextCursor::End);
  ui->plainTextEditTerminalDebug->textCursor().insertHtml(text);
  ui->plainTextEditTerminalDebug->setTextCursor(prevCursor);
}

void MainWindow::on_listWidgetTerminalCodeList_itemClicked(QListWidgetItem *item) {
  QString code = "";
  if (item->text().contains(" "))
    code = item->text().left(item->text().indexOf(" "));
  else
    code = item->text();

  if (code == "3?m") {
    QColor color = QColorDialog::getColor(Qt::white);
    if (!color.isValid()) return;
    QByteArray colorCode = ui->myTerminal->nearestColorCode(color);
    ui->myTerminal->printToTerminal(QString("\u001b[3" + colorCode + "m").toUtf8());
    return;
  }

  if (code == "4?m") {
    QColor color = QColorDialog::getColor(Qt::black);
    if (!color.isValid()) return;
    QByteArray colorCode = ui->myTerminal->nearestColorCode(color);
    ui->myTerminal->printToTerminal(QString("\u001b[4" + colorCode + "m").toUtf8());
    return;
  }

  ui->myTerminal->printToTerminal(QString("\u001b[" + code).toUtf8());
}
