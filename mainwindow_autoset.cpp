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
  for (int i =  0; i < ANALOG_COUNT + MATH_COUNT; i++)
    if (ui->plot->isChUsed(i) && ui->plot->isChVisible(i))
      activeAnalogs.append(i);
  for (int i = 0; i < LOGIC_GROUPS; i++)
    if (ui->plot->getLogicBitsUsed(i) > 0)
      activeLogic.append(i);

  if (activeAnalogs.count() + activeLogic.count() > 1) {
    // multiple channels
    double recentOffset = 0;

    for (int i = activeLogic.size() - 1; i >= 0; i--) {
      int bitsUsed = ui->plot->getLogicBitsUsed(activeLogic.at(i));
      // Skupina logických kanálů má spodek na 0 a vršek je 3*počet kanálů (bitů)
      ui->plot->setLogicOffset(activeLogic.at(i), recentOffset);
      recentOffset += bitsUsed * 3 * ui->plot->getLogicScale(activeLogic.at(i));
    }
    QCPRange maxRange;
    maxRange.upper = 0;
    maxRange.lower = 0;
    for (int i = activeAnalogs.size() - 1; i >= 0; i--) {
      bool foundRange; // Zbytečné, ale ta funkce to potřebuje.
      QCPRange range = ui->plot->graph(activeAnalogs.at(i))->data()->valueRange(foundRange);
      range.lower = ceilToNiceValue(range.lower) * ui->plot->getChScale(activeAnalogs.at(i));
      range.upper = ceilToNiceValue(range.upper) * ui->plot->getChScale(activeAnalogs.at(i));
      if (ui->plot->isChInverted(activeAnalogs.at(i))) {
        // Pokud je kanál invertovaný, změní znaménko a prohodí
        range.upper *= -1;
        range.lower *= -1;
        range.normalize();
      }
      if (range.upper > maxRange.upper)
        maxRange.upper = range.upper;
      if (range.lower < maxRange.lower)
        maxRange.lower = range.lower;
    }
    for (int i = activeAnalogs.size() - 1; i >= 0; i--) {
      recentOffset -= maxRange.lower;
      ui->plot->setChOffset(activeAnalogs.at(i), recentOffset);
      recentOffset += maxRange.upper;
    }
    if (recentOffset != 0) {
      on_pushButtonPositive_clicked();
      ui->checkBoxVerticalValues->setChecked(false);
      ui->doubleSpinBoxRangeVerticalRange->setValue(recentOffset);
    }
  } else {
    // Only one channel
    on_pushButtonResetChannels_clicked();
    if (!activeAnalogs.isEmpty()) {
      bool foundrange;
      QCPRange range = ui->plot->graph(activeAnalogs.at(0))->data()->valueRange(foundrange);
      if (ui->plot->isChInverted(activeAnalogs.at(0))) {
        // Pokud je kanál invertovaný, změní znaménko a prohodí
        range.upper *= -1;
        range.lower *= -1;
        range.normalize();
      }
      if (range.lower >= 0) {
        on_pushButtonPositive_clicked();
        ui->doubleSpinBoxRangeVerticalRange->setValue(ceilToNiceValue(range.upper));
      } else if (range.upper <= 0) {
        on_pushButtonNegative_clicked();
        ui->doubleSpinBoxRangeVerticalRange->setValue(ceilToNiceValue(abs(range.lower)));
      } else {
        on_pushButtonCenter_clicked();
        ui->doubleSpinBoxRangeVerticalRange->setValue(2 * ceilToNiceValue(MAX(std::abs(range.upper), std::abs(range.lower))));
      }
      ui->checkBoxVerticalValues->setChecked(true);
      ui->comboBoxSelectedChannel->setCurrentIndex(activeAnalogs.at(0));
    } else if (!activeLogic.isEmpty()) {
      int bitsUsed = ui->plot->getLogicBitsUsed(activeLogic.at(0));
      // Skupina logických kanálů má spodek na 0 a vršek je 3*počet pod-kanálů (bitů)
      ui->doubleSpinBoxRangeVerticalRange->setValue(bitsUsed * 3 * ui->plot->getLogicScale(activeLogic.at(0)));
      ui->checkBoxVerticalValues->setChecked(false);
      on_pushButtonPositive_clicked();
      ui->comboBoxSelectedChannel->setCurrentIndex(LOGIC_GROUP_TO_CH_LIST_INDEX(activeLogic.at(0)));
    }
  }
  on_comboBoxSelectedChannel_currentIndexChanged(ui->comboBoxSelectedChannel->currentIndex());
  if (lastUpdateWasPoint) {
    ui->radioButtonRollingRange->setChecked(true);
    if (dataUpdates > 10)
      ui->doubleSpinBoxRangeHorizontal->setValue(10);
    else
      ui->doubleSpinBoxRangeHorizontal->setValue(100);
  } else {
    // Pevný režim
    ui->dialZoom->setValue(ui->dialZoom->maximum());// Zoom žádný
    ui->radioButtonFixedRange->setChecked(true);
  }
}
