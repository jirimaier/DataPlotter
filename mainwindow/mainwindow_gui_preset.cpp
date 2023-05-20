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
#include "ui_developeroptions.h"
#include "ui_freqtimeplotdialog.h"

void MainWindow::connectSignals() {
  connect(ui->pushButtonPause, &QPushButton::clicked, ui->plot,
          &MyMainPlot::togglePause);
  connect(ui->checkBoxVerticalValues, &QCheckBox::toggled, ui->plot,
          &MyPlot::setShowVerticalValues);
  connect(ui->plot, &MyMainPlot::showPlotStatus, this,
          &MainWindow::showPlotStatus);
  connect(ui->plot, &MyPlot::gridChanged, this, &MainWindow::updateDivs);
  connect(ui->plot, &MyPlot::moveTimeCursor, this,
          &MainWindow::timeCursorMovedByMouse);
  connect(ui->plot, &MyPlot::moveValueCursor, this,
          &MainWindow::valueCursorMovedByMouse);
  connect(ui->plot, &MyPlot::setCursorPos, this, &MainWindow::cursorSetByMouse);
  connect(ui->plot, &MyMainPlot::offsetChangedByMouse, this,
          &MainWindow::offsetChangedByMouse);
  connect(ui->plotxy, &MyXYPlot::moveTimeCursorXY, this,
          &MainWindow::moveTimeCursorXY);
  connect(ui->plotFFT, &MyPlot::moveTimeCursor, this,
          &MainWindow::timeCursorMovedByMouse);
  connect(ui->plotxy, &MyPlot::moveValueCursor, this,
          &MainWindow::moveValueCursorXY);
  connect(ui->plotFFT, &MyPlot::moveValueCursor, this,
          &MainWindow::valueCursorMovedByMouse);
  connect(ui->plotxy, &MyXYPlot::setCursorPosXY, this,
          &MainWindow::setCursorPosXY);
  connect(ui->plotFFT, &MyPlot::setCursorPos, this,
          &MainWindow::cursorSetByMouse);
  connect(ui->lineEditHtitle, &QLineEdit::textChanged, ui->plot,
          &MyPlot::setXTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plot,
          &MyPlot::setYTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plotxy,
          &MyPlot::setXTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plotxy,
          &MyPlot::setYTitle);
  connect(ui->horizontalSliderVGrid, &QDial::valueChanged, ui->plot,
          &MyPlot::setGridHintY);
  connect(ui->horizontalSliderHGrid, &QDial::valueChanged, ui->plot,
          &MyPlot::setGridHintX);
  connect(ui->spinBoxShiftStep, SIGNAL(valueChanged(int)), ui->plot,
          SLOT(setShiftStep(int)));
  connect(ui->plot, &MyMainPlot::requestCursorUpdate, this,
          &MainWindow::updateCursors);
  connect(ui->pushButtonFFTHoldMax1, &QPushButton::toggled, ui->plotFFT,
          &MyFFTPlot::setHoldMax1);
  connect(ui->pushButtonFFTHoldMax2, &QPushButton::toggled, ui->plotFFT,
          &MyFFTPlot::setHoldMax2);

  connect(&ansiTerminalModel, &AnsiTerminalModel::sendMessage, this,
          &MainWindow::printMessage);
  connect(&portsRefreshTimer, &QTimer::timeout, this, &MainWindow::comRefresh);
  connect(&activeChRefreshTimer, &QTimer::timeout, this,
          &MainWindow::updateUsedChannels);
  connect(&cursorRangeUpdateTimer, &QTimer::timeout, this,
          &MainWindow::updateCursorRange);
  connect(&measureRefreshTimer1, &QTimer::timeout, this,
          &MainWindow::updateMeasurements1);
  connect(&measureRefreshTimer2, &QTimer::timeout, this,
          &MainWindow::updateMeasurements2);
  connect(&fftTimer1, &QTimer::timeout, this, &::MainWindow::updateFFT1);
  connect(&fftTimer2, &QTimer::timeout, this, &::MainWindow::updateFFT2);
  connect(&xyTimer, &QTimer::timeout, this, &::MainWindow::updateXY);
  connect(&serialMonitorTimer, &QTimer::timeout, this,
          &MainWindow::updateSerialMonitor);
  connect(&serialMonitorTimer, &QTimer::timeout, this,
          &MainWindow::updateConsole);
  connect(&interpolationTimer, &QTimer::timeout, this,
          &MainWindow::updateInterpolation);
  connect(&triggerLineTimer, &QTimer::timeout, this,
          &MainWindow::turnOffTriggerLine);

  connect(ui->horizontalSliderTimeCur1, &myCursorSlider::realValueChanged, this,
          &MainWindow::horizontalSliderTimeCur1_realValueChanged);
  connect(ui->horizontalSliderTimeCur2, &myCursorSlider::realValueChanged, this,
          &MainWindow::horizontalSliderTimeCur2_realValueChanged);

  connect(ui->plotFFT, &MyFFTPlot::newPeakValues,
          freqTimePlotDialog->getUi()->plotPeak, &MyPeakPlot::newData);

  connect(qmlTerminalInterface, &QmlTerminalInterface::dataSentToParser, this,
          &MainWindow::sendManualInput);

  connect(developerOptions, &DeveloperOptions::colorExceptionListChanged,
          &ansiTerminalModel, &AnsiTerminalModel::setColorExceptionList);
  connect(developerOptions, &DeveloperOptions::loadQmlFile, this,
          &MainWindow::loadQmlFile);
  connect(developerOptions, &DeveloperOptions::terminalDevToggled,
          &ansiTerminalModel, &AnsiTerminalModel::setShowGrid);
  connect(developerOptions, &DeveloperOptions::printToTerminal, this,
          &MainWindow::printToTerminal);
  // connect(developerOptions->getUi()->pushButtonLoadFile,
  // &QPushButton::clicked, this, &MainWindow::opushButtonLoadFile_clicked);
  // connect(developerOptions->getUi()->pushButtonDefaults,
  // &QPushButton::clicked, this, &MainWindow::pushButtonDefaults_clicked);
  // connect(developerOptions->getUi()->pushButtonSaveSettings,
  // &QPushButton::clicked, this, &MainWindow::pushButtonSaveSettings_clicked);
  connect(developerOptions->getUi()->pushButtonClearAll, &QPushButton::clicked,
          this, &MainWindow::pushButtonClearAll_clicked);
  connect(developerOptions->getUi()->checkBoxTriggerLineEn,
          &QCheckBox::stateChanged, this,
          &MainWindow::checkBoxTriggerLineEn_stateChanged);
  connect(developerOptions->getUi()->pushButtonClearGraph,
          &QPushButton::clicked, this,
          &MainWindow::pushButtonClearGraph_clicked);
  connect(developerOptions->getUi()->checkBoxEchoReply, &QCheckBox::toggled,
          this, &MainWindow::checkBoxEchoReply_toggled);
  connect(developerOptions->getUi()->checkBoxMouseControls, &QCheckBox::toggled,
          this, &MainWindow::checkBoxMouseControls_toggled_new);
  connect(freqTimePlotDialog, &FreqTimePlotDialog::requestedCSVExport, this,
          &MainWindow::exportCSV);
  connect(developerOptions, &DeveloperOptions::sendManualInput, this,
          &MainWindow::sendManualInput);
  connect(developerOptions, &DeveloperOptions::requestManualBufferClear, this,
          &MainWindow::requestManualBufferClear);
  connect(developerOptions, &DeveloperOptions::requestManualBufferShow, this,
          &MainWindow::requestManualBufferShow);
  connect(developerOptions, &DeveloperOptions::requestSerialBufferClear, this,
          &MainWindow::requestSerialBufferClear);
  connect(developerOptions, &DeveloperOptions::requestSerialBufferShow, this,
          &MainWindow::requestSerialBufferShow);
  connect(&ansiTerminalModel, &AnsiTerminalModel::gridClickedSignal,
          developerOptions, &DeveloperOptions::addTerminalCursorPosCommand);

  connect(ui->plot, &MyMainPlot::vRangeChanged, this,
          &MainWindow::mainPlotVRangeChanged);
  connect(ui->plot, &MyMainPlot::vRangeMaxChanged, this,
          &MainWindow::mainPlotVRangeMaxChanged);
  connect(ui->plot, &MyMainPlot::hRangeChanged, this,
          &MainWindow::mainPlotHRangeChanged);
  connect(ui->plot, &MyMainPlot::hRangeMaxChanged, this,
          &MainWindow::mainPlotHRangeMaxChanged);

  connect(ui->plot, &MyMainPlot::lastDataTypeWasPointChanged, this,
          &MainWindow::lastDataTypeWasPointChanged);
}

void MainWindow::setAdaptiveSpinBoxes() {
  // Adaptivní krok není v starším Qt (Win XP)
#if QT_VERSION >= 0x050C00
  ui->doubleSpinBoxRangeHorizontal->setStepType(
      QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxRangeVerticalRange->setStepType(
      QAbstractSpinBox::AdaptiveDecimalStepType);

  ui->doubleSpinBoxXYCurX1->setStepType(
      QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxXYCurX2->setStepType(
      QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxXYCurY1->setStepType(
      QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxXYCurY2->setStepType(
      QAbstractSpinBox::AdaptiveDecimalStepType);
#endif
}

void MainWindow::startTimers() {
  portsRefreshTimer.start(500);
  activeChRefreshTimer.start(500);
  cursorRangeUpdateTimer.start(100);
  measureRefreshTimer1.start(250);
  measureRefreshTimer2.start(250);
  fftTimer1.start(50);
  fftTimer2.start(50);
  xyTimer.start(50);
  serialMonitorTimer.start(500);
  interpolationTimer.start(50);
  consoleTimer.start(250);
}

void MainWindow::setGuiDefaults() {
  ui->pushButtonSerialMonitor->setChecked(false);
  ui->tabs_right->setCurrentIndex(0);
  ui->tabsControll->setCurrentIndex(0);
  ui->comboBoxOutputLevel->setCurrentIndex((int)OutputLevel::warning);
  ui->labelBuildDate->setText(tr("Build: ") + QString(__DATE__) + " " +
                              QString(__TIME__));
  ui->pushButtonPause->setIcon(iconRun);

  on_pushButtonModeFull_clicked();
  ui->frameRollingRange->setVisible(false);
  setPlotLayout("all");

  on_lineEditHUnit_textChanged(ui->lineEditHUnit->text());
  on_lineEditVUnit_textChanged(ui->lineEditVUnit->text());

  on_comboBoxFFTType_currentIndexChanged(ui->comboBoxFFTType->currentIndex());

  ui->spinBoxFFTSegments1->setVisible(ui->comboBoxFFTType->currentIndex() ==
                                      FFTType::pwelch);
  ui->spinBoxFFTSegments2->setVisible(ui->comboBoxFFTType->currentIndex() ==
                                      FFTType::pwelch);
  ui->checkBoxFFTCh1->setChecked(true);
  ui->comboBoxFFTCh1->setCurrentIndex(0);
  ui->comboBoxFFTCh2->setCurrentIndex(1);
  ui->comboBoxXYx->setCurrentIndex(0);
  ui->comboBoxXYy->setCurrentIndex(1);

  ui->comboBoxMeasure1->setCurrentIndex(0);
  ui->comboBoxMeasure2->setCurrentIndex(1);

  ui->comboBoxMathFirst1->setCurrentIndex(0);
  ui->comboBoxMathSecond1->setCurrentIndex(1);
  ui->comboBoxMathFirst2->setCurrentIndex(0);
  ui->comboBoxMathSecond2->setCurrentIndex(1);
  ui->comboBoxMathFirst3->setCurrentIndex(0);
  ui->comboBoxMathSecond3->setCurrentIndex(1);

  ui->comboBoxLogic1->setCurrentIndex(0);
  ui->comboBoxLogic2->setCurrentIndex(1);

  ui->comboBoxFFTType->setCurrentIndex(1);

  ui->comboBoxBaud->setCurrentIndex(1);

  ui->plot->setGridHintX(ui->horizontalSliderHGrid->value());
  ui->plot->setGridHintY(ui->horizontalSliderVGrid->value());
  ui->plotxy->setGridHintX(ui->horizontalSliderXYGrid->value());
  ui->plotxy->setGridHintY(ui->horizontalSliderXYGrid->value());
  ui->plotFFT->setGridHintX(ui->horizontalSliderGridFFTH->value());
  ui->plotFFT->setGridHintY(ui->horizontalSliderGridFFTV->value());

  on_doubleSpinBoxRangeVerticalRange_valueChanged(
      ui->doubleSpinBoxRangeVerticalRange->value());

  ui->doubleSpinBoxXCur1->setVisible(false);
  ui->doubleSpinBoxXCur2->setVisible(false);
  ui->doubleSpinBoxYCur1->setVisible(false);
  ui->doubleSpinBoxYCur2->setVisible(false);

  ui->radioButtonAverageAll->setChecked(true);

  triggerLineTimer.setSingleShot(true);
  triggerLineTimer.setInterval(2000);

  developerOptions->getUi()->checkBoxTriggerLineEn->setCheckState(
      Qt::PartiallyChecked);

  ui->comboBoxFIR->setCurrentIndex(0);
  on_comboBoxFIR_currentIndexChanged(0);  // Interpolátor načte filtr
}

void MainWindow::setGuiArrays() {
  mathEn[0] = ui->pushButtonMath1;
  mathEn[1] = ui->pushButtonMath2;
  mathEn[2] = ui->pushButtonMath3;

  mathFirst[0] = ui->comboBoxMathFirst1;
  mathFirst[1] = ui->comboBoxMathFirst2;
  mathFirst[2] = ui->comboBoxMathFirst3;

  mathSecond[0] = ui->comboBoxMathSecond1;
  mathSecond[1] = ui->comboBoxMathSecond2;
  mathSecond[2] = ui->comboBoxMathSecond3;

  mathOp[0] = ui->comboBoxMath1Op;
  mathOp[1] = ui->comboBoxMath2Op;
  mathOp[2] = ui->comboBoxMath3Op;

  mathScalarFirst[0] = ui->doubleSpinBoxMathScalar1_1;
  mathScalarFirst[1] = ui->doubleSpinBoxMathScalar1_2;
  mathScalarFirst[2] = ui->doubleSpinBoxMathScalar1_3;

  mathScalarSecond[0] = ui->doubleSpinBoxMathScalar2_1;
  mathScalarSecond[1] = ui->doubleSpinBoxMathScalar2_2;
  mathScalarSecond[2] = ui->doubleSpinBoxMathScalar2_3;
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
  ui->comboBoxMathFirst1->blockSignals(true);
  ui->comboBoxMathFirst2->blockSignals(true);
  ui->comboBoxMathFirst3->blockSignals(true);
  ui->comboBoxMathSecond1->blockSignals(true);
  ui->comboBoxMathSecond2->blockSignals(true);
  ui->comboBoxMathSecond3->blockSignals(true);
  ui->comboBoxLogic1->blockSignals(true);
  ui->comboBoxLogic2->blockSignals(true);
  ui->comboBoxAvgIndividualCh->blockSignals(true);
  developerOptions->getUi()->comboBoxChClear->blockSignals(true);

  for (int i = 0; i < ANALOG_COUNT; i++) {
    ui->comboBoxMathFirst1->addItem(getChName(i));
    ui->comboBoxMathFirst2->addItem(getChName(i));
    ui->comboBoxMathFirst3->addItem(getChName(i));
    ui->comboBoxMathSecond1->addItem(getChName(i));
    ui->comboBoxMathSecond2->addItem(getChName(i));
    ui->comboBoxMathSecond3->addItem(getChName(i));
    ui->comboBoxLogic1->addItem(getChName(i));
    ui->comboBoxLogic2->addItem(getChName(i));
    ui->comboBoxAvgIndividualCh->addItem(getChName(i));
  }

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
    developerOptions->getUi()->comboBoxChClear->addItem(getChName(i));
  }
  for (int i = 1; i <= LOGIC_GROUPS - 1; i++) {
    ui->comboBoxSelectedChannel->addItem(tr("Logic %1").arg(i));
    ui->comboBoxCursor1Channel->addItem(tr("Logic %1").arg(i));
    ui->comboBoxCursor2Channel->addItem(tr("Logic %1").arg(i));
    developerOptions->getUi()->comboBoxChClear->addItem(tr("Logic %1").arg(i));
  }

  // Poslední logický kanál (pro přímý zápis) je bez čísla
  ui->comboBoxSelectedChannel->addItem(tr("Logic"));
  ui->comboBoxCursor1Channel->addItem(tr("Logic"));
  ui->comboBoxCursor2Channel->addItem(tr("Logic"));
  developerOptions->getUi()->comboBoxChClear->addItem(tr("Logic"));

  ui->comboBoxCursor1Channel->addItem("FFT 1");
  ui->comboBoxCursor1Channel->addItem("FFT 2");
  ui->comboBoxCursor2Channel->addItem("FFT 1");
  ui->comboBoxCursor2Channel->addItem("FFT 2");

  ui->comboBoxMeasure1->addItem(iconCross, "Off");
  ui->comboBoxMeasure2->addItem(iconCross, "Off");
  ui->comboBoxMeasure1->setCurrentIndex(ui->comboBoxMeasure1->count() - 1);
  ui->comboBoxMeasure2->setCurrentIndex(ui->comboBoxMeasure2->count() - 1);

  ui->comboBoxCursor1Channel->addItem(iconAbsoluteCursor, tr("Absolute"));
  ui->comboBoxCursor2Channel->addItem(iconAbsoluteCursor, tr("Absolute"));
  ui->comboBoxCursor1Channel->setCurrentIndex(
      ui->comboBoxCursor1Channel->count() - 1);
  ui->comboBoxCursor2Channel->setCurrentIndex(
      ui->comboBoxCursor2Channel->count() - 1);

  // Skryje FFT kanály z nabýdky.
  setComboboxItemVisible(*ui->comboBoxCursor1Channel, FFT_INDEX(0), false);
  setComboboxItemVisible(*ui->comboBoxCursor2Channel, FFT_INDEX(0), false);
  setComboboxItemVisible(*ui->comboBoxCursor1Channel, FFT_INDEX(1), false);
  setComboboxItemVisible(*ui->comboBoxCursor2Channel, FFT_INDEX(1), false);

  ui->comboBoxMathFirst1->addItem(tr("Constant"));
  ui->comboBoxMathFirst2->addItem(tr("Constant"));
  ui->comboBoxMathFirst3->addItem(tr("Constant"));
  ui->comboBoxMathSecond1->addItem(tr("Constant"));
  ui->comboBoxMathSecond2->addItem(tr("Constant"));
  ui->comboBoxMathSecond3->addItem(tr("Constant"));

  ui->comboBoxSelectedChannel->blockSignals(false);
  ui->comboBoxCursor1Channel->blockSignals(false);
  ui->comboBoxCursor2Channel->blockSignals(false);
  ui->comboBoxMeasure1->blockSignals(false);
  ui->comboBoxMeasure2->blockSignals(false);
  ui->comboBoxFFTCh1->blockSignals(false);
  ui->comboBoxFFTCh2->blockSignals(false);
  ui->comboBoxXYx->blockSignals(false);
  ui->comboBoxXYy->blockSignals(false);
  ui->comboBoxMathFirst1->blockSignals(false);
  ui->comboBoxMathFirst2->blockSignals(false);
  ui->comboBoxMathFirst3->blockSignals(false);
  ui->comboBoxMathSecond1->blockSignals(false);
  ui->comboBoxMathSecond2->blockSignals(false);
  ui->comboBoxMathSecond3->blockSignals(false);
  ui->comboBoxLogic1->blockSignals(false);
  ui->comboBoxLogic2->blockSignals(false);
  ui->comboBoxAvgIndividualCh->blockSignals(false);
  developerOptions->getUi()->comboBoxChClear->blockSignals(false);
}
