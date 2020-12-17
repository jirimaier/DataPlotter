#include "mainwindow.h"

void MainWindow::updateCursors() {
  int ch = ui->comboBoxCursor1Channel->currentIndex();
  QPair<long, long> range = ui->plot->getChVisibleSamplesRange(ch < ANALOG_COUNT + MATH_COUNT ? ch : GlobalFunctions::getLogicChannelID(ch - ANALOG_COUNT - MATH_COUNT, 0));
  ui->horizontalSliderTimeCur1->updateRange(range.first, range.second);
  double time1, value1, time2, value2;
  QByteArray timeStr, valueStr;

  if (ui->checkBoxCur1Visible->isChecked()) {
    updateCursor(Cursors::X1, ch, ui->horizontalSliderTimeCur1->realValue, time1, value1, timeStr, valueStr);
    ui->labelCur1Sample->setText(QString::number(ui->horizontalSliderTimeCur1->realValue).toLocal8Bit());
    ui->labelCur1Time->setText(timeStr);
    ui->labelCur1Val->setText(valueStr);
  } else {
    ui->labelCur1Sample->setText("---");
    ui->labelCur1Time->setText("---");
    ui->labelCur1Val->setText("---");
  }

  ch = ui->comboBoxCursor2Channel->currentIndex();
  range = ui->plot->getChVisibleSamplesRange(ch);
  ui->horizontalSliderTimeCur2->updateRange(range.first, range.second);
  timeStr = "";
  valueStr = "";

  if (ui->checkBoxCur2Visible->isChecked()) {
    updateCursor(Cursors::X2, ch, ui->horizontalSliderTimeCur2->realValue, time2, value2, timeStr, valueStr);
    ui->labelCur2Sample->setText(QString::number(ui->horizontalSliderTimeCur2->realValue).toLocal8Bit());
    ui->labelCur2Time->setText(timeStr);
    ui->labelCur2Val->setText(valueStr);
  } else {
    ui->labelCur2Sample->setText("---");
    ui->labelCur2Time->setText("---");
    ui->labelCur2Val->setText("---");
  }

  if (ui->checkBoxCur1Visible->isChecked() && ui->checkBoxCur2Visible->isChecked()) {
    double dt = time2 - time1;
    double dy = value2 - value1;
    ui->labelCurDeltaTime->setText(QString::number(dt, 'f', abs(dt) < 1000 ? 3 : 0));
    ui->labelCurDeltaValue->setText(QString::number(dy, 'f', abs(dy) < 1000 ? 3 : 0));
    ui->labelCurSlope->setText(QString::number(dy / dt, 'f', abs(dy / dt) < 1000 ? 3 : 0));
    ui->labelCurFreq->setText(QString::number(1 / dt, 'f', abs(1 / dt) < 1000 ? 3 : 0));
  } else {
    ui->labelCurDeltaTime->setText("---");
    ui->labelCurDeltaValue->setText("---");
    ui->labelCurSlope->setText("---");
    ui->labelCurFreq->setText("---");
  }

  // updateCursor(ui->comboBoxCursor2Channel->currentIndex(), ui->horizontalSliderTimeCur1->realValue);
}

void MainWindow::updateCursor(Cursors::enumerator cursor, int selectedChannel, unsigned int sample, double &time, double &value, QByteArray &timeStr, QByteArray &valueStr) {
  if (selectedChannel < ANALOG_COUNT + MATH_COUNT) {
    // Analogový kanál
    time = ui->plot->graph(selectedChannel)->data()->at(sample)->key;
    value = ui->plot->graph(selectedChannel)->data()->at(sample)->value;
    if (ui->plot->isChInverted(selectedChannel))
      value *= (-1);
    double valueOffseted = value * ui->plot->getChScale(selectedChannel) + ui->plot->getChOffset(selectedChannel);
    timeStr = QString::number(time, 'f', 3).toLocal8Bit();
    valueStr = QString::number(value, 'f', 3).toLocal8Bit();
    ui->plot->setCursorVisible(cursor, true);
    ui->plot->setCursorVisible(cursor + 2, true);
    ui->plot->updateCursor(cursor, time);
    ui->plot->updateCursor(cursor + 2, valueOffseted);
  } else {
    QByteArray bits;
    value = 0.0;
    int group = selectedChannel - ANALOG_COUNT - MATH_COUNT;
    time = ui->plot->graph(GlobalFunctions::getLogicChannelID(group, 0))->data()->at(sample)->key;
    int bitsUsed = ui->plot->getLogicBitsUsed(group);
    for (int bit = 0; bit < bitsUsed; bit++) {
      int chid = GlobalFunctions::getLogicChannelID(group, bit);
      // Potřebuji zjistit, zda je 0 nebo 1. To nejde jen tak (hodnota obsahuje offset vůči prvnímu bitu)
      // Že je nula zjistím tak, že hodnota je dělitelná třemi (nulová úroveň každého jena násobku 3).
      // Za každou jedničku přičtu 2^bit
      if ((int)ui->plot->graph(chid)->data()->at(sample)->value % 3) {
        value += (uint32_t)1 << (bit);
        bits.push_front('1');
      } else {
        bits.push_front('0');
      }
      if (!((bit + 1) % 4))
        bits.push_front(' ');
    }
    if (bits.left(1) == " ")
      bits = bits.mid(1);
    timeStr = QString::number(time, 'f', 3).toLocal8Bit();
    valueStr = "0x" + QString::number((uint32_t)value, 16).toUpper().rightJustified(ceil(bitsUsed / 4.0), '0').toLocal8Bit();
    valueStr.append("\n" + bits);
    ui->plot->setCursorVisible(cursor, true);
    ui->plot->setCursorVisible(cursor + 2, false);
    ui->plot->updateCursor(cursor, time);
  }
}

void MainWindow::on_checkBoxCur1Visible_toggled(bool checked) {
  ui->plot->setCursorVisible(Cursors::X1, checked);
  ui->plot->setCursorVisible(Cursors::Y1, checked);
  updateCursors();
}

void MainWindow::on_checkBoxCur2Visible_toggled(bool checked) {
  ui->plot->setCursorVisible(Cursors::X2, checked);
  ui->plot->setCursorVisible(Cursors::Y2, checked);
  updateCursors();
}

void MainWindow::on_horizontalSliderTimeCur1_realValueChanged() {
  ui->checkBoxCur1Visible->setChecked(true);
  updateCursors();
}

void MainWindow::on_horizontalSliderTimeCur2_realValueChanged() {
  ui->checkBoxCur2Visible->setChecked(true);
  updateCursors();
}
