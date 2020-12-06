#include "mainwindow.h"

void MainWindow::updateCursors() {
  int chid = ui->comboBoxCursor1Channel->currentIndex();
  if (chid < 0)
    return;
  QPair<long, long> range = ui->plot->getChVisibleSamples(chid);
  ui->horizontalSliderTimeCur1->updateRange(range.first, range.second);
  chid = ui->comboBoxCursor2Channel->currentIndex();
  if (chid < 0)
    return;
  range = ui->plot->getChVisibleSamples(chid);
  ui->horizontalSliderTimeCur2->updateRange(range.first, range.second);

  auto result1 = QPair<double, double>(0, 0);
  auto result2 = QPair<double, double>(0, 0);

  int chid1 = ui->comboBoxCursor1Channel->currentIndex();
  if (ui->plot->isChUsed(chid1)) {
    result1 = ui->plot->setTimeCursor(Cursors::X1, chid1, ui->horizontalSliderTimeCur1->realValue);
    ui->labelCur1Sample->setText(QString::number(ui->horizontalSliderTimeCur1->realValue));
    ui->labelCur1Time->setText(QString::number(result1.first, 'f', 3));
    ui->labelCur1Val->setText(QString::number(result1.second, 'f', 3));
  }

  int chid2 = ui->comboBoxCursor2Channel->currentIndex();
  if (ui->plot->isChUsed(chid1)) {
    result2 = ui->plot->setTimeCursor(Cursors::X2, chid2, ui->horizontalSliderTimeCur2->realValue);
    ui->labelCur2Sample->setText(QString::number(ui->horizontalSliderTimeCur2->realValue));
    ui->labelCur2Time->setText(QString::number(result2.first, 'f', 3));
    ui->labelCur2Val->setText(QString::number(result2.second, 'f', 3));
  }

  ui->labelCurDeltaTime->setText(QString::number(abs(result2.first - result1.first), 'f', 3));
  ui->labelCurDeltaValue->setText(QString::number(abs(result2.second - result1.second), 'f', 3));
  ui->labelCurFreq->setText(QString::number(1.0 / abs(result2.first - result1.first), 'f', 3));
  ui->labelCurSlope->setText(QString::number((result2.second - result1.second) / (result2.first - result1.first), 'f', 3));
}

void MainWindow::on_checkBoxCur1Visible_toggled(bool checked) {
  ui->plot->setCursorVisible(Cursors::X1, checked);
  ui->plot->setCursorVisible(Cursors::Y1, checked);
}

void MainWindow::on_checkBoxCur2Visible_toggled(bool checked) {
  ui->plot->setCursorVisible(Cursors::X2, checked);
  ui->plot->setCursorVisible(Cursors::Y2, checked);
}

void MainWindow::on_horizontalSliderTimeCur1_realValueChanged(int value) {
  ui->checkBoxCur1Visible->setChecked(true);
  updateCursors();
}

void MainWindow::on_horizontalSliderTimeCur2_realValueChanged(int value) {
  ui->checkBoxCur2Visible->setChecked(true);
  updateCursors();
}
