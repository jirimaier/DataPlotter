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

void MainWindow::connectSignals() {
  connect(ui->pushButtonPause, &QPushButton::clicked, ui->plot, &MyMainPlot::togglePause);
  connect(ui->checkBoxVerticalValues, &QCheckBox::toggled, ui->plot, &MyPlot::setShowVerticalValues);
  connect(ui->plot, &MyMainPlot::showPlotStatus, this, &MainWindow::showPlotStatus);
  connect(ui->plot, &MyPlot::gridChanged, this, &MainWindow::updateDivs);
  connect(ui->plot, &MyPlot::moveTimeCursor, this, &MainWindow::timeCursorMovedByMouse);
  connect(ui->plot, &MyPlot::moveValueCursor, this, &MainWindow::valueCursorMovedByMouse);
  connect(ui->plot, &MyPlot::setCursorPos, this, &MainWindow::cursorSetByMouse);
  connect(ui->plot, &MyMainPlot::offsetChangedByMouse, this, &MainWindow::offsetChangedByMouse);
  connect(ui->plotxy, &MyXYPlot::moveTimeCursorXY, this, &MainWindow::moveTimeCursorXY);
  connect(ui->plotFFT, &MyPlot::moveTimeCursor, this, &MainWindow::timeCursorMovedByMouse);
  connect(ui->plotxy, &MyPlot::moveValueCursor, this, &MainWindow::moveValueCursorXY);
  connect(ui->plotFFT, &MyPlot::moveValueCursor, this, &MainWindow::valueCursorMovedByMouse);
  connect(ui->plotxy, &MyXYPlot::setCursorPosXY, this, &MainWindow::setCursorPosXY);
  connect(ui->plotFFT, &MyPlot::setCursorPos, this, &MainWindow::cursorSetByMouse);
  connect(ui->doubleSpinBoxRangeHorizontal, SIGNAL(valueChanged(double)), ui->plot, SLOT(setRollingRange(double)));
  connect(ui->doubleSpinBoxRangeVerticalRange, SIGNAL(valueChanged(double)), ui->plot, SLOT(setVerticalRange(double)));
  connect(ui->doubleSpinBoxRangeHorizontal, SIGNAL(valueChanged(double)), ui->dialRollingRange, SLOT(updatePosition(double)));
  connect(ui->doubleSpinBoxRangeVerticalRange, SIGNAL(valueChanged(double)), ui->dialVerticalRange, SLOT(updatePosition(double)));
  //connect(ui->doubleSpinBoxChScale, SIGNAL(valueChanged(double)), ui->dialChScale, SLOT(updatePosition(double)));
  connect(ui->sliderVerticalCenter, &QSlider::valueChanged, ui->plot, &MyMainPlot::setVerticalCenter);
  connect(ui->horizontalScrollBarHorizontal, &QScrollBar::valueChanged, ui->plot, &MyMainPlot::setHorizontalPos);
  connect(ui->lineEditHtitle, &QLineEdit::textChanged, ui->plot, &MyPlot::setXTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plot, &MyPlot::setYTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plotxy, &MyPlot::setXTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plotxy, &MyPlot::setYTitle);
  connect(ui->myTerminal, &MyTerminal::sendMessage, this, &MainWindow::printMessage);
  connect(ui->horizontalSliderVGrid, &QDial::valueChanged, ui->plot, &MyPlot::setGridHintY);
  connect(ui->horizontalSliderHGrid, &QDial::valueChanged, ui->plot, &MyPlot::setGridHintX);
  connect(ui->spinBoxShiftStep, SIGNAL(valueChanged(int)), ui->plot, SLOT(setShiftStep(int)));
  connect(ui->plot, &MyMainPlot::requestCursorUpdate, this, &MainWindow::updateCursors);

  connect(&portsRefreshTimer, &QTimer::timeout, this, &MainWindow::comRefresh);
  connect(&activeChRefreshTimer, &QTimer::timeout, this, &MainWindow::updateUsedChannels);
  connect(&cursorRangeUpdateTimer, &QTimer::timeout, this, &MainWindow::updateCursorRange);
  connect(&measureRefreshTimer1, &QTimer::timeout, this, &MainWindow::updateMeasurements1);
  connect(&measureRefreshTimer2, &QTimer::timeout, this, &MainWindow::updateMeasurements2);
  connect(&fftTimer1, &QTimer::timeout, this, &::MainWindow::updateFFT1);
  connect(&fftTimer2, &QTimer::timeout, this, &::MainWindow::updateFFT2);
  connect(&xyTimer, &QTimer::timeout, this, &::MainWindow::updateXY);
  connect(&serialMonitorTimer, &QTimer::timeout, this, &MainWindow::updateSerialMonitor);
  connect(&dataRateTimer, &QTimer::timeout, this, &MainWindow::updateDataRate);
  connect(&interpolationTimer, &QTimer::timeout, this, &MainWindow::updateInterpolation);
}

void MainWindow::setAdaptiveSpinBoxes() {
// Adaptivní krok není v starším Qt (Win XP)
#if QT_VERSION >= 0x050C00
  ui->doubleSpinBoxChScale->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxRangeHorizontal->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxRangeVerticalRange->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

  ui->doubleSpinBoxXYCurX1->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxXYCurX2->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxXYCurY1->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxXYCurY2->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
#endif
}

void MainWindow::startTimers() {
  portsRefreshTimer.start(500);
  activeChRefreshTimer.start(500);
  cursorRangeUpdateTimer.start(100);
  measureRefreshTimer1.start(200);
  measureRefreshTimer2.start(200);
  fftTimer1.start(100);
  fftTimer2.start(100);
  xyTimer.start(100);
  serialMonitorTimer.start(500);
  dataRateTimer.start(1000);
  interpolationTimer.start(50);
}

void MainWindow::setGuiDefaults() {
  ui->tabs_right->setCurrentIndex(0);
  ui->tabsControll->setCurrentIndex(0);
  ui->comboBoxOutputLevel->setCurrentIndex((int)OutputLevel::info);
  ui->radioButtonFixedRange->setChecked(true);
  ui->plotxy->setHidden(true);
  ui->plotFFT->setHidden(true);
  ui->frameTermanalDebug->setVisible(ui->pushButtonTerminalDebug->isChecked());
  ui->labelBuildDate->setText(tr("Build: ") + QString(__DATE__) + " " + QString(__TIME__));
  ui->pushButtonPause->setIcon(iconRun);
  ui->pushButtonMultiplInputs->setChecked(false);

  ui->plot->setXUnit("s");
  ui->plotxy->tUnit = "s";
  ui->plotFFT->setXUnit("Hz");
  ui->plot->setYUnit("V");
  ui->plotxy->setYUnit("V");
  ui->plotxy->setYUnit("V");

  on_comboBoxFFTType_currentIndexChanged(ui->comboBoxFFTType->currentIndex());

  ui->checkBoxYCur1->setCheckState(Qt::CheckState::PartiallyChecked);
  ui->checkBoxYCur2->setCheckState(Qt::CheckState::PartiallyChecked);
  ui->spinBoxFFTSegments1->setVisible(ui->comboBoxFFTType->currentIndex() == FFTType::pwelch);
  ui->spinBoxFFTSegments2->setVisible(ui->comboBoxFFTType->currentIndex() == FFTType::pwelch);
  ui->checkBoxFFTCh1->setChecked(true);
  ui->comboBoxFFTCh1->setCurrentIndex(0);
  ui->comboBoxFFTCh2->setCurrentIndex(1);
  ui->comboBoxXYx->setCurrentIndex(0);
  ui->comboBoxXYy->setCurrentIndex(1);

  ui->comboBoxMeasure1->setCurrentIndex(0);
  ui->comboBoxMeasure2->setCurrentIndex(1);

  ui->comboBoxFFTType->setCurrentIndex(1);

  ui->plot->setGridHintX(ui->horizontalSliderHGrid->value());
  ui->plot->setGridHintY(ui->horizontalSliderVGrid->value());
  ui->plotxy->setGridHintX(ui->horizontalSliderXYGrid->value());
  ui->plotxy->setGridHintY(ui->horizontalSliderXYGrid->value());
  ui->plotFFT->setGridHintX(ui->horizontalSliderGridFFTH->value());
  ui->plotFFT->setGridHintY(ui->horizontalSliderGridFFTV->value());

  on_doubleSpinBoxRangeVerticalRange_valueChanged(ui->doubleSpinBoxRangeVerticalRange->value());
}

void MainWindow::setGuiArrays() {
  mathEn[0] = ui->pushButtonMath1;
  mathEn[1] = ui->pushButtonMath2;
  mathEn[2] = ui->pushButtonMath3;

  mathFirst[0] = ui->spinBoxMath1First;
  mathFirst[1] = ui->spinBoxMath2First;
  mathFirst[2] = ui->spinBoxMath3First;

  mathSecond[0] = ui->spinBoxMath1Second;
  mathSecond[1] = ui->spinBoxMath2Second;
  mathSecond[2] = ui->spinBoxMath3Second;

  mathOp[0] = ui->comboBoxMath1Op;
  mathOp[1] = ui->comboBoxMath2Op;
  mathOp[2] = ui->comboBoxMath3Op;
}

void MainWindow::fillChannelSelect() {
  ui->comboBoxSelectedChannel->blockSignals(true);
  ui->comboBoxCursor1Channel->blockSignals(true);
  ui->comboBoxCursor2Channel->blockSignals(true);
  ui->comboBoxMeasure1->blockSignals(true);
  ui->comboBoxMeasure2->blockSignals(true);
  ui->comboBoxFFTCh1->blockSignals(true);
  ui->comboBoxFFTCh2->blockSignals(true);
  ui->comboBoxXYx->blockSignals(true);
  ui->comboBoxXYy->blockSignals(true);

  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
    ui->comboBoxSelectedChannel->addItem(getChName(i));
    ui->comboBoxCursor1Channel->addItem(getChName(i));
    ui->comboBoxCursor2Channel->addItem(getChName(i));
    ui->comboBoxMeasure1->addItem(getChName(i));
    ui->comboBoxMeasure2->addItem(getChName(i));
    ui->comboBoxFFTCh1->addItem(getChName(i));
    ui->comboBoxFFTCh2->addItem(getChName(i));
    ui->comboBoxXYx->addItem(getChName(i));
    ui->comboBoxXYy->addItem(getChName(i));
  }
  for (int i = 1; i <= LOGIC_GROUPS - 1; i++) {
    ui->comboBoxSelectedChannel->addItem(tr("Logic %1").arg(i));
    ui->comboBoxCursor1Channel->addItem(tr("Logic %1").arg(i));
    ui->comboBoxCursor2Channel->addItem(tr("Logic %1").arg(i));
  }

  // Poslední logický kanál (pro přímý zápis) je bez čísla
  ui->comboBoxSelectedChannel->addItem(tr("Logic"));
  ui->comboBoxCursor1Channel->addItem(tr("Logic"));
  ui->comboBoxCursor2Channel->addItem(tr("Logic"));

  ui->comboBoxCursor1Channel->addItem("FFT 1");
  ui->comboBoxCursor1Channel->addItem("FFT 2");
  ui->comboBoxCursor2Channel->addItem("FFT 1");
  ui->comboBoxCursor2Channel->addItem("FFT 2");

  ui->comboBoxMeasure1->addItem(iconCross, "Off");
  ui->comboBoxMeasure2->addItem(iconCross, "Off");
  ui->comboBoxMeasure1->setCurrentIndex(ui->comboBoxMeasure1->count() - 1);
  ui->comboBoxMeasure2->setCurrentIndex(ui->comboBoxMeasure2->count() - 1);

  // Skryje FFT kanály z nabýdky.
  setComboboxItemVisible(*ui->comboBoxCursor1Channel, FFTID(0), false);
  setComboboxItemVisible(*ui->comboBoxCursor2Channel, FFTID(0), false);
  setComboboxItemVisible(*ui->comboBoxCursor1Channel, FFTID(1), false);
  setComboboxItemVisible(*ui->comboBoxCursor2Channel, FFTID(1), false);

  ui->comboBoxSelectedChannel->blockSignals(false);
  ui->comboBoxCursor1Channel->blockSignals(false);
  ui->comboBoxCursor2Channel->blockSignals(false);
  ui->comboBoxMeasure1->blockSignals(false);
  ui->comboBoxMeasure2->blockSignals(false);
  ui->comboBoxFFTCh1->blockSignals(false);
  ui->comboBoxFFTCh2->blockSignals(false);
  ui->comboBoxXYx->blockSignals(false);
  ui->comboBoxXYy->blockSignals(false);
}
