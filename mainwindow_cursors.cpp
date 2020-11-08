#include "mainwindow.h"

void MainWindow::scrollBarCursorValueChanged() {
  if (!ui->radioButtonCurOff->isChecked()) {
    for (int i = 0; i < 4; i++)
      cursorPos[i] = cursors[i]->value() / 1000.0;
    ui->labelCursorX1->setText("X1: " + QString::number(cursorPos[0], 'f', 3));
    ui->labelCursorX2->setText("X2: " + QString::number(cursorPos[1], 'f', 3));
    if (ui->radioButtonCurMain->isChecked()) {
      ui->labelCursorY1->setText("Y1: " + QString::number(cursorPos[2] - ui->plot->getChOffset(ui->spinBoxChannelSelect->value()) / ui->plot->getChScale(ui->spinBoxChannelSelect->value()), 'f', 3));
      ui->labelCursorY2->setText("Y2: " + QString::number(cursorPos[3] - ui->plot->getChOffset(ui->spinBoxChannelSelect->value()) / ui->plot->getChScale(ui->spinBoxChannelSelect->value()), 'f', 3));
      ui->labelCursordY->setText(tr("dY: ") + QString::number(fabs(cursorPos[3] - cursorPos[2]) / ui->plot->getChScale(ui->spinBoxChannelSelect->value())));
    } else {
      ui->labelCursorY1->setText("Y1: " + QString::number(cursorPos[2], 'f', 3));
      ui->labelCursorY2->setText("Y2: " + QString::number(cursorPos[3], 'f', 3));
      ui->labelCursordY->setText(tr("dY: ") + QString::number(fabs(cursorPos[3] - cursorPos[2])));
    }
    ui->labelCursordX->setText(tr("dX: ") + QString::number(fabs(cursorPos[1] - cursorPos[0])));
    ui->labelCursorFX->setText(tr("1/dX: ") + QString::number(1.0 / (fabs(cursorPos[1] - cursorPos[0])), 'f', 3));

    if (ui->radioButtonCurMain->isChecked())
      ui->plot->updateCursors(cursorPos);
    else if (ui->radioButtonCurXY->isChecked())
      ui->plotxy->updateCursors(cursorPos);
  }
}

void MainWindow::on_pushButtonCursorToView_clicked() {
  if (ui->radioButtonCurOff->isChecked())
    ui->radioButtonCurMain->setChecked(true);
  cursors[0]->setValue((plotFrame.xMinView + (plotFrame.xMaxView - plotFrame.xMinView) * (1.0 / 4.0)) * 1000);
  cursors[1]->setValue((plotFrame.xMinView + (plotFrame.xMaxView - plotFrame.xMinView) * (3.0 / 4.0)) * 1000);
  cursors[2]->setValue((plotFrame.yMinView + (plotFrame.yMaxView - plotFrame.yMinView) * (1.0 / 4.0)) * 1000);
  cursors[3]->setValue((plotFrame.yMinView + (plotFrame.yMaxView - plotFrame.yMinView) * (3.0 / 4.0)) * 1000);
}

void MainWindow::setCursorBounds(PlotFrame_t frame) {
  for (int i = 0; i < 4; i++) {
    int stepsize;
    if (i < 2) {
      cursors[i]->setMinimum(MIN(frame.xMinTotal, frame.xMinView) * 1000);
      cursors[i]->setMaximum(MAX(frame.xMaxTotal, frame.xMaxView) * 1000);
      stepsize = (frame.xMaxView - frame.xMinView) * 2;

    } else {
      cursors[i]->setMinimum(MIN(frame.yMinTotal, frame.yMinView) * 1000);
      cursors[i]->setMaximum(MAX(frame.yMaxTotal, frame.yMaxView) * 1000);
      stepsize = (frame.yMaxView - frame.yMinView) * 2;
    }
    cursors[i]->setSingleStep(stepsize);
    cursors[i]->setPageStep(stepsize);
  }
  this->plotFrame = frame;
}

void MainWindow::on_pushButtonCursorsZero_clicked() {
  if (ui->radioButtonCurOff->isChecked())
    ui->radioButtonCurMain->setChecked(true);
  cursors[0]->setValue(0);
  cursors[1]->setValue(0);
  cursors[2]->setValue(0);
  cursors[3]->setValue(0);
  scrollBarCursorValueChanged();
}
