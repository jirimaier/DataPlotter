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
        for (int i = 0; i < activeAnalogs.size(); i++) {
            QCPRange range;
            if (channelExpectedRanges[activeAnalogs.at(i)].unknown) {
                bool foundRange;
                range = ui->plot->graph(activeAnalogs.at(i))->data()->valueRange(foundRange);
                if (foundRange) {
                    range.lower = ceilToNiceValue(range.lower) * ui->plot->getChScale(activeAnalogs.at(i));
                    range.upper = ceilToNiceValue(range.upper) * ui->plot->getChScale(activeAnalogs.at(i));
                } else
                    range = QCPRange(0, 0);
            } else
                range = QCPRange(channelExpectedRanges[activeAnalogs.at(i)].minimum, channelExpectedRanges[activeAnalogs.at(i)].maximum * 1.1);
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
            recentOffset = ceilToMultipleOf(recentOffset, ui->plot->getVDiv());
            ui->plot->setChOffset(activeAnalogs.at(i), recentOffset);
            recentOffset += maxRange.upper;
        }
        if (recentOffset != 0) {
            ui->checkBoxVerticalValues->setChecked(false);
            ui->doubleSpinBoxRangeVerticalRange->setValue(ceilToMultipleOf(recentOffset, ui->plot->getVDiv()));
            on_pushButtonSetPositive_clicked();
        }
    } else {
        // Only one channel
        on_pushButtonResetChannels_clicked();
        if (!activeAnalogs.isEmpty()) {
            QCPRange range;
            if (channelExpectedRanges[activeAnalogs.first()].unknown) {
                bool foundRange;
                range = ui->plot->graph(activeAnalogs.first())->data()->valueRange(foundRange);
                if (foundRange) {
                    range.lower = ceilToNiceValue(range.lower) * ui->plot->getChScale(activeAnalogs.first());
                    range.upper = ceilToNiceValue(range.upper) * ui->plot->getChScale(activeAnalogs.first());
                } else
                    range = QCPRange(0, 0);
            } else
                range = QCPRange(channelExpectedRanges[activeAnalogs.first()].minimum, channelExpectedRanges[activeAnalogs.first()].maximum);
            if (ui->plot->isChInverted(activeAnalogs.first())) {
                // Pokud je kanál invertovaný, změní znaménko a prohodí
                range.upper *= -1;
                range.lower *= -1;
                range.normalize();
            }
            if (range.lower >= 0) {
                ui->doubleSpinBoxRangeVerticalRange->setValue(range.upper);
                on_pushButtonSetPositive_clicked();
            } else if (range.upper <= 0) {
                ui->doubleSpinBoxRangeVerticalRange->setValue(abs(range.lower));
                on_pushButtonSetNegative_clicked();
            } else {
                ui->doubleSpinBoxRangeVerticalRange->setValue(2 * MAX(std::abs(range.upper), std::abs(range.lower)));
                on_pushButtonSetCenter_clicked();
            }
            ui->checkBoxVerticalValues->setChecked(true);
            ui->comboBoxSelectedChannel->setCurrentIndex(activeAnalogs.first());
        } else if (!activeLogic.isEmpty()) {
            int bitsUsed = ui->plot->getLogicBitsUsed(activeLogic.first());
            // Skupina logických kanálů má spodek na 0 a vršek je 3*počet pod-kanálů (bitů)
            ui->doubleSpinBoxRangeVerticalRange->setValue(bitsUsed * 3 * ui->plot->getLogicScale(activeLogic.at(0)));
            ui->checkBoxVerticalValues->setChecked(false);
            on_pushButtonSetCenter_clicked();
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

void MainWindow::on_pushButtonResetChannels_clicked() {
    for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
        ui->plot->setChOffset(i, 0);
        ui->plot->setChScale(i, 1);
    }
    for (int i = 0; i < LOGIC_GROUPS; i++) {
        ui->plot->setLogicOffset(i, 0);
        ui->plot->setLogicScale(i, 1);
    }

    QCPRange valueRange(0, 5);
    bool nominmaxfoundyet = true;

    for (int i =  0; i < ANALOG_COUNT + MATH_COUNT; i++)
        if (ui->plot->isChUsed(i) && ui->plot->isChVisible(i)) {
            QCPRange chRange;
            if (channelExpectedRanges[i].unknown) {
                bool zbytrecnaPromena;
                chRange = ui->plot->graph(i)->getValueRange(zbytrecnaPromena, QCP::sdBoth, ui->plot->xAxis->range());
            } else {
                chRange.lower = channelExpectedRanges[i].minimum;
                chRange.upper = channelExpectedRanges[i].maximum;
            }
            if (ui->plot->isChInverted(i)) {
                // Pokud je kanál invertovaný, změní znaménko a prohodí
                chRange.upper *= -1;
                chRange.lower *= -1;
                chRange.normalize();
            }
            if (chRange.upper > valueRange.upper || nominmaxfoundyet)
                valueRange.upper = chRange.upper;
            if (chRange.lower < valueRange.lower || nominmaxfoundyet)
                valueRange.lower = chRange.lower;
            nominmaxfoundyet = false;
        }
    for (int i = 0; i < LOGIC_GROUPS; i++)
        if (ui->plot->getLogicBitsUsed(i) > 0) {
            QCPRange chRange(0, 3);
            chRange.upper = ui->plot->getLogicBitsUsed(i) * 3;
            if (chRange.upper > valueRange.upper || nominmaxfoundyet)
                valueRange.upper = chRange.upper;
            if (chRange.lower < valueRange.lower || nominmaxfoundyet)
                valueRange.lower = chRange.lower;
            nominmaxfoundyet = false;
        }

    if (valueRange.lower >= 0) {
        ui->doubleSpinBoxRangeVerticalRange->setValue(valueRange.upper);
        on_pushButtonSetPositive_clicked();
    } else if (valueRange.upper <= 0) {
        ui->doubleSpinBoxRangeVerticalRange->setValue(abs(valueRange.lower));
        on_pushButtonSetNegative_clicked();
    } else {
        ui->doubleSpinBoxRangeVerticalRange->setValue(2 * MAX(std::abs(valueRange.upper), std::abs(valueRange.lower)));
        on_pushButtonSetCenter_clicked();
    }
    ui->checkBoxVerticalValues->setChecked(true);

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
