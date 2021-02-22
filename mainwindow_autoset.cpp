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

void MainWindow::on_pushButtonAutoset_clicked() {
  QVector<int> activeAnalogs, activeLogic;
  for (int i = ANALOG_COUNT + MATH_COUNT - 1; i >= 0; i--)
    if (ui->plot->isChUsed(i) && ui->plot->isChVisible(i))
      activeAnalogs.append(i);
  for (int i = LOGIC_GROUPS - 1; i >= 0; i--)
    if (ui->plot->getLogicBitsUsed(i) > 0)
      activeLogic.append(i);

  if (activeAnalogs.count() + activeLogic.count() > 1) {
    double recentOffset = 0;

    for (int i = LOGIC_GROUPS - 1; i >= 0; i--) {
      int bitsUsed = ui->plot->getLogicBitsUsed(i);
      if (bitsUsed > 0 && ui->plot->isLogicVisible(i)) {
        // Skupina logických kanálů má spodek na 0 a vršek je 3*počet kanálů (bitů)
        ui->plot->setLogicOffset(i, recentOffset);
        recentOffset += bitsUsed * 3 * ui->plot->getLogicScale(i);
      }
    }
    for (int i = ANALOG_COUNT + MATH_COUNT - 1; i >= 0; i--) {
      if (ui->plot->isChUsed(i) && ui->plot->isChVisible(i)) {
        bool foundRange; // Zbytečné, ale ta funkce to potřebuje.
        QCPRange range = ui->plot->graph(i)->data()->valueRange(foundRange);
        range.lower = ceilToNiceValue(range.lower) * ui->plot->getChScale(i);
        range.upper = ceilToNiceValue(range.upper) * ui->plot->getChScale(i);
        recentOffset -= range.lower;
        ui->plot->setChOffset(i, recentOffset);
        recentOffset += range.upper;
      }
    }
    if (recentOffset != 0) {
      on_pushButtonPositive_clicked();
      ui->checkBoxVerticalValues->setChecked(false);
      ui->doubleSpinBoxRangeVerticalRange->setValue(recentOffset);
    }
  } else {
    if (!activeAnalogs.isEmpty()) {
      on_pushButtonResetChannels_clicked();
      bool foundrange;
      QCPRange range = ui->plot->graph(activeAnalogs.at(0))->data()->valueRange(foundrange);
      if (range.lower >= 0) {
        on_pushButtonPositive_clicked();
        ui->doubleSpinBoxRangeVerticalRange->setValue(ceilToNiceValue(range.upper));
      } else {
        on_pushButtonCenter_clicked();
        ui->doubleSpinBoxRangeVerticalRange->setValue(2 * ceilToNiceValue(MAX(std::abs(range.upper), std::abs(range.lower))));
      }
      ui->checkBoxVerticalValues->setChecked(true);
    }
  }
  on_comboBoxSelectedChannel_currentIndexChanged(ui->comboBoxSelectedChannel->currentIndex());
  if (lastUpdateWasPoint) {
    ui->radioButtonRollingRange->setChecked(true);
    if (dataUpdates > 10)
      ui->doubleSpinBoxRangeHorizontal->setValue(10);
    else
      ui->doubleSpinBoxRangeHorizontal->setValue(100);
    if (lastPointTimeWasHMS)
      ui->comboBoxHAxisType->setCurrentIndex(HAxisType::HMS);
    else
      ui->comboBoxHAxisType->setCurrentIndex(HAxisType::normal);
  } else {
    // Pevný režim
    ui->dialZoom->setValue(ui->dialZoom->maximum());// Zoom žádný
    ui->radioButtonFixedRange->setChecked(true);
    ui->comboBoxHAxisType->setCurrentIndex(HAxisType::normal);
  }
}
