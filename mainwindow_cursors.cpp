#include "mainwindow.h"

void MainWindow::scrollBarCursorValueChanged() {
  for (int i = 0; i < 4; i++)
    cursorPos[i] = cursors[i]->value() / 1000.0;
  ui->labelCursorX1->setText("X1: " + QString::number(cursorPos[0], 'f', 3));
  ui->labelCursorX2->setText("X2: " + QString::number(cursorPos[1], 'f', 3));
  ui->labelCursorY1->setText("Y1: " + QString::number(cursorPos[2] - ui->plot->getChOffset(ui->spinBoxChannelSelect->value()) / ui->plot->getChScale(ui->spinBoxChannelSelect->value()), 'f', 3));
  ui->labelCursorY2->setText("Y2: " + QString::number(cursorPos[3] - ui->plot->getChOffset(ui->spinBoxChannelSelect->value()) / ui->plot->getChScale(ui->spinBoxChannelSelect->value()), 'f', 3));
  ui->labelCursordX->setText(tr("dX: ") + QString::number(fabs(cursorPos[1] - cursorPos[0])));
  ui->labelCursordY->setText(tr("dY: ") + QString::number(fabs(cursorPos[3] - cursorPos[2]) / ui->plot->getChScale(ui->spinBoxChannelSelect->value())));
  ui->plot->updateCursors(cursorPos);
}

void MainWindow::on_pushButtonCursorToView_clicked() {
  cursors[0]->setValue((plotFrame.xMinView + (plotFrame.xMaxView - plotFrame.xMinView) * (1.0 / 4.0)) * 1000);
  cursors[1]->setValue((plotFrame.xMinView + (plotFrame.xMaxView - plotFrame.xMinView) * (3.0 / 4.0)) * 1000);
  cursors[2]->setValue((plotFrame.yMinView + (plotFrame.yMaxView - plotFrame.yMinView) * (1.0 / 4.0)) * 1000);
  cursors[3]->setValue((plotFrame.yMinView + (plotFrame.yMaxView - plotFrame.yMinView) * (3.0 / 4.0)) * 1000);
}

void MainWindow::setCursorBounds(PlotFrame_t frame) {
  for (int i = 0; i < 4; i++) {
    int stepsize;
    if (i < 2) {
      cursors[i]->setMinimum(frame.xMinTotal * 1000);
      cursors[i]->setMaximum(frame.xMaxTotal * 1000);
      stepsize = (frame.xMaxView - frame.xMinView) * 2;

    } else {
      cursors[i]->setMinimum(frame.yMinTotal * 1000);
      cursors[i]->setMaximum(frame.yMaxTotal * 1000);
      stepsize = (frame.yMaxView - frame.yMinView) * 2;
    }
    cursors[i]->setSingleStep(stepsize);
    cursors[i]->setPageStep(stepsize);
  }
  this->plotFrame = frame;
}
