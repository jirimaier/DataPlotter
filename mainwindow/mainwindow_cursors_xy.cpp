//  Copyright (C) 2020-2024  Jiří Maier

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

using namespace Cursors;

void MainWindow::on_checkBoxXYCur1_toggled(bool checked) {
  ui->plotxy->setValueCursorVisible(Cursor1, checked);
  ui->plotxy->setTimeCursorVisible(Cursor1, checked);
  ui->plotxy->updateValueCursor(Cursor1, ui->doubleSpinBoxXYCurY1->value(), ui->doubleSpinBoxXYCurY1->text(), ui->plotxy->yAxis);
  ui->plotxy->updateTimeCursor(Cursor1, ui->doubleSpinBoxXYCurX1->value(), ui->doubleSpinBoxXYCurX1->text(), -1);
  updateXYCursorsCalculations();
}

void MainWindow::on_checkBoxXYCur2_toggled(bool checked) {
  ui->plotxy->setValueCursorVisible(Cursor2, checked);
  ui->plotxy->setTimeCursorVisible(Cursor2, checked);
  ui->plotxy->updateValueCursor(Cursor2, ui->doubleSpinBoxXYCurY2->value(), ui->doubleSpinBoxXYCurY2->text(), ui->plotxy->yAxis);
  ui->plotxy->updateTimeCursor(Cursor2, ui->doubleSpinBoxXYCurX2->value(), ui->doubleSpinBoxXYCurX2->text(), -1);
  updateXYCursorsCalculations();
}

void MainWindow::on_doubleSpinBoxXYCurY1_valueChanged(double arg1) {
  ui->plotxy->updateValueCursor(Cursor1, arg1, ui->doubleSpinBoxXYCurY1->text(), ui->plotxy->yAxis);
  updateXYCursorsCalculations();
}

void MainWindow::on_doubleSpinBoxXYCurX1_valueChanged(double arg1) {
  ui->plotxy->updateTimeCursor(Cursor1, arg1, ui->doubleSpinBoxXYCurX1->text(), -1);
  updateXYCursorsCalculations();
}

void MainWindow::on_doubleSpinBoxXYCurY2_valueChanged(double arg1) {
  ui->plotxy->updateValueCursor(Cursor2, arg1, ui->doubleSpinBoxXYCurY2->text(), ui->plotxy->yAxis);
  updateXYCursorsCalculations();
}

void MainWindow::on_doubleSpinBoxXYCurX2_valueChanged(double arg1) {
  ui->plotxy->updateTimeCursor(Cursor2, arg1, ui->doubleSpinBoxXYCurX2->text(), -1);
  updateXYCursorsCalculations();
}

void MainWindow::moveTimeCursorXY(Cursors::enumCursors cursor, double pos) {
  if (cursor == Cursor1)
    ui->doubleSpinBoxXYCurX1->setValue(pos);
  else
    ui->doubleSpinBoxXYCurX2->setValue(pos);
}

void MainWindow::moveValueCursorXY(Cursors::enumCursors cursor, double pos) {
  if (cursor == Cursor1)
    ui->doubleSpinBoxXYCurY1->setValue(pos);
  else
    ui->doubleSpinBoxXYCurY2->setValue(pos);
}

void MainWindow::setCursorPosXY(enumCursors cursor, double x, double y) {
  if (cursor == Cursor1) {
    ui->checkBoxXYCur1->setChecked(true);
    ui->doubleSpinBoxXYCurY1->setValue(y);
    ui->doubleSpinBoxXYCurX1->setValue(x);
  } else {
    ui->checkBoxXYCur2->setChecked(true);
    ui->doubleSpinBoxXYCurY2->setValue(y);
    ui->doubleSpinBoxXYCurX2->setValue(x);
  }
}

void MainWindow::updateXYCursorsCalculations() {
  if (ui->checkBoxXYCur1->isChecked() && ui->checkBoxXYCur2->isChecked()) {
    double dx = abs(ui->doubleSpinBoxXYCurX2->value() - ui->doubleSpinBoxXYCurX1->value());
    double dy = abs(ui->doubleSpinBoxXYCurY1->value() - ui->doubleSpinBoxXYCurY2->value());

    ui->labelXYDeltaX->setText(floatToNiceString(dx, 4, true, true, false, ui->plotxy->getYUnit()));
    ui->labelXYDeltaY->setText(floatToNiceString(dy, 4, true, true, false, ui->plotxy->getYUnit()));
    double phase = abs(asin(MIN(dx, dy) / MAX(dx, dy)));
    ui->labelXYPhaseDeg->setText(" " + QString::number(qRadiansToDegrees(phase), 'f', 1) + "°");
    ui->labelXYPhaseRad->setText(" " + QString::number(phase / M_PI, 'f', 3) + "\xCF\x80"); // Pi
  } else {
    ui->labelXYDeltaX->setText("---");
    ui->labelXYDeltaY->setText("---");
    ui->labelXYPhaseDeg->setText("---");
    ui->labelXYPhaseRad->setText("---");
  }
}


