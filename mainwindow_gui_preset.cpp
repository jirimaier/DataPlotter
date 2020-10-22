#include "mainwindow.h"

void MainWindow::connectSignals() {
  connect(ui->pushButtonPause, &QPushButton::clicked, ui->plot, &MyPlot::pauseClicked);
  connect(ui->pushButtonSingleTriger, &QPushButton::clicked, ui->plot, &MyPlot::singleTrigerClicked);
  connect(ui->checkBoxCurXEn, &QCheckBox::toggled, ui->plot, &MyPlot::setCurXen);
  connect(ui->checkBoxCurYEn, &QCheckBox::toggled, ui->plot, &MyPlot::setCurYen);
  connect(ui->checkBoxVerticalValues, &QCheckBox::toggled, ui->plot, &MyPlot::setShowVerticalValues);
  connect(ui->checkBoxHorizontalValues, &QCheckBox::toggled, ui->plot, &MyPlot::setShowHorizontalValues);
  connect(ui->plot, &MyPlot::showPlotStatus, this, &MainWindow::showPlotStatus);
  connect(ui->plot, &MyPlot::updateDivs, this, &MainWindow::updateDivs);
  connect(ui->plot, &MyPlot::setCursorBounds, this, &MainWindow::setCursorBounds);
  connect(ui->doubleSpinBoxRangeHorizontal, SIGNAL(valueChanged(double)), ui->plot, SLOT(setRollingRange(double)));
  connect(ui->doubleSpinBoxRangeVerticalRange, SIGNAL(valueChanged(double)), ui->plot, SLOT(setVerticalRange(double)));
  connect(ui->doubleSpinBoxRangeHorizontal, SIGNAL(valueChanged(double)), ui->dialRollingRange, SLOT(updatePosition(double)));
  connect(ui->doubleSpinBoxRangeVerticalRange, SIGNAL(valueChanged(double)), ui->dialVerticalRange, SLOT(updatePosition(double)));
  connect(ui->doubleSpinBoxChScale, SIGNAL(valueChanged(double)), ui->dialChScale, SLOT(updatePosition(double)));
  connect(ui->verticalScrollBarVerticalCenter, &QScrollBar::valueChanged, ui->plot, &MyPlot::setVerticalCenter);
  connect(ui->horizontalScrollBarHorizontal, &QScrollBar::valueChanged, ui->plot, &MyPlot::setHorizontalPos);
  connect(ui->pushButtonPrintBuffer, &QPushButton::clicked, this, &MainWindow::requestBufferDebug);

  connect(&plotUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePlot);
  connect(&listUpdateTimer, &QTimer::timeout, this, &MainWindow::updateInfo);
  connect(&portsRefreshTimer, &QTimer::timeout, this, &MainWindow::comRefresh);
  connect(&graphResetTimer, &QTimer::timeout, this, &MainWindow::autoResetChannels);
}

void MainWindow::setAdaptiveSpinBoxes() {
// Adaptivní krok není v starším Qt (Win XP)
#if QT_VERSION >= 0x050C00
  ui->doubleSpinBoxBinaryDataMin->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxBinaryTimestep->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxBinarydataMax->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxChScale->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxRangeHorizontal->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxRangeVerticalRange->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
#endif
}

void MainWindow::startTimers() {
  portsRefreshTimer.setInterval(500);
  plotUpdateTimer.setInterval(10);
  listUpdateTimer.setInterval(200);
  plotUpdateTimer.start();
  listUpdateTimer.start();
  portsRefreshTimer.start();
}

void MainWindow::setGuiDefaults() {
  ui->tabs_right->setCurrentIndex(0);
  ui->tabs_Plot->setCurrentIndex(0);
  ui->checkBoxModeManual->setChecked(false);
  ui->labelBinSettings->setHidden(true);

  ui->labelBuildDate->setText("Build: " + QString(__DATE__) + " " + QString(__TIME__));
  ui->labelDataMode->setText(tr("Data mode: ") + ui->comboBoxDataMode->itemText(0));

  ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/run.png"));
}

void MainWindow::setGuiArrays() {
  cursors[0] = ui->horizontalScrollBarCursorX1;
  cursors[1] = ui->horizontalScrollBarCursorX2;
  cursors[2] = ui->verticalScrollBarCursorY1;
  cursors[3] = ui->verticalScrollBarCursorY2;
  mathEn[0] = ui->checkBoxMath1;
  mathEn[1] = ui->checkBoxMath2;
  mathEn[2] = ui->checkBoxMath3;
  mathEn[3] = ui->checkBoxMath4;
  mathFirst[0] = ui->spinBoxMath1First;
  mathFirst[1] = ui->spinBoxMath2First;
  mathFirst[2] = ui->spinBoxMath3First;
  mathFirst[3] = ui->spinBoxMath4First;
  mathSecond[0] = ui->spinBoxMath1Second;
  mathSecond[1] = ui->spinBoxMath2Second;
  mathSecond[2] = ui->spinBoxMath3Second;
  mathSecond[3] = ui->spinBoxMath4Second;
  mathOp[0] = ui->comboBoxMath1Op;
  mathOp[1] = ui->comboBoxMath2Op;
  mathOp[2] = ui->comboBoxMath3Op;
  mathOp[3] = ui->comboBoxMath4Op;
}
