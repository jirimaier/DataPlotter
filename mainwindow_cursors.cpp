#include "mainwindow.h"

void MainWindow::scrollBarCursorValueChanged() {
  double x1 = ui->horizontalScrollBarCursorX1->value() / 1000.0;
  double x2 = ui->horizontalScrollBarCursorX2->value() / 1000.0;
  double y1 = ui->verticalScrollBarCursorY1->value() / 1000.0;
  double y2 = ui->verticalScrollBarCursorY2->value() / 1000.0;
  ui->labelCursorX1->setText("X1: " + QString::number(x1, 'f', 3));
  ui->labelCursorX2->setText("X2: " + QString::number(x2, 'f', 3));
  ui->labelCursorY1->setText("Y1: " + QString::number(y1 - ui->plot->getChOffset(ui->spinBoxChannelSelect->value()) / ui->plot->getChScale(ui->spinBoxChannelSelect->value()), 'f', 3));
  ui->labelCursorY2->setText("Y2: " + QString::number(y2 - ui->plot->getChOffset(ui->spinBoxChannelSelect->value()) / ui->plot->getChScale(ui->spinBoxChannelSelect->value()), 'f', 3));
  ui->labelCursordX->setText(tr("dX: ") + QString::number(fabs(x2 - x1)));
  ui->labelCursordY->setText(tr("dY: ") + QString::number(fabs(y2 - y1) / ui->plot->getChScale(ui->spinBoxChannelSelect->value())));
  ui->plot->updateCursors(x1, x2, y1, y2);
}

void MainWindow::on_pushButtonCursorToView_clicked() {
  ui->horizontalScrollBarCursorX1->setValue((plotLeft + (plotRight - plotLeft) * (1.0 / 4.0)) * 1000);
  ui->horizontalScrollBarCursorX2->setValue((plotLeft + (plotRight - plotLeft) * (3.0 / 4.0)) * 1000);
  ui->verticalScrollBarCursorY1->setValue((plotBottom + (plotTop - plotBottom) * (1.0 / 4.0)) * 1000);
  ui->verticalScrollBarCursorY2->setValue((plotBottom + (plotTop - plotBottom) * (3.0 / 4.0)) * 1000);
}
