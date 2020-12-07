#include "mainwindow.h"

void MainWindow::updateCursors() {
  int ch = ui->comboBoxCursor1Channel->currentIndex();
  QPair<long, long> range = ui->plot->getChVisibleSamples(ch);
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
  range = ui->plot->getChVisibleSamples(ch);
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
    ui->labelCurDeltaTime->setText(QString::number(time2 - time1, 'f', 3));
    ui->labelCurDeltaValue->setText(QString::number(value2 - value1, 'f', 3));
    ui->labelCurSlope->setText(QString::number((value2 - value1) / (time2 - time1), 'f', 3));
    ui->labelCurFreq->setText(QString::number(1 / (time2 - time1), 'f', 3));
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
    double valueOffseted = ui->plot->graph(selectedChannel)->data()->at(sample)->value;
    value = valueOffseted / ui->plot->getChScale(selectedChannel) - ui->plot->getChOffset(selectedChannel);
    timeStr = QString::number(time, 'f', 3).toLocal8Bit();
    valueStr = QString::number(value, 'f', 3).toLocal8Bit();
    ui->plot->setCursorVisible(cursor, true);
    ui->plot->setCursorVisible(cursor + 2, true);
    ui->plot->updateCursor(cursor, time);
    ui->plot->updateCursor(cursor + 2, valueOffseted);
  } else {
    QByteArray bits;
    value = 0.0;
    int group = selectedChannel - ANALOG_COUNT - MATH_COUNT + 1;
    time = ui->plot->graph(GlobalFunctions::getLogicChannelId(group, 1))->data()->at(sample)->key;
    int bitsUsed = ui->plot->getLogicBitsUsed(group);
    for (int bit = 0; bit < bitsUsed; bit++) {
      int chid = GlobalFunctions::getLogicChannelId(group, bit + 1);
      // Potřebuji zjistit, zda je 0 nebo 1. To nejde jen tak (hodnota obsahuje offset a scale)
      // Že je nula zjistím tak, že se hodnota rovná offsetu (scale nemá na nulu vliv).
      // Za každou jedničku přičtu 2^bit
      if (ui->plot->graph(chid)->data()->at(sample)->value != ui->plot->getChOffset(chid)) {
        value += (uint32_t)1 << (bit);
        bits.push_front('1');
      } else
        bits.push_front('0');
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
