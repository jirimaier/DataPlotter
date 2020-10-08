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
  connect(&plotUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePlot);
  connect(&listUpdateTimer, &QTimer::timeout, this, &MainWindow::updateInfo);
  connect(&portsRefreshTimer, &QTimer::timeout, this, &MainWindow::comRefresh);
  plotUpdateTimer.start();
  listUpdateTimer.start();
  portsRefreshTimer.start();
}

void MainWindow::setGuiDefaults() {
  ui->tabs_right->setCurrentIndex(0);
  ui->tabs_Plot->setCurrentIndex(0);
  ui->checkBoxModeManual->setChecked(false);
  ui->comboBoxPlotRangeType->setCurrentIndex(PlotRange::fixedRange);
  ui->comboBoxOutputLevel->setCurrentIndex(OutputLevel::low);
  ui->labelDataMode->setText(tr("Data mode: ") + ui->comboBoxDataMode->itemText(0));

  ui->comboBoxMath4Op->setCurrentIndex(MathOperations::xy);
  ui->labelBuildDate->setText("Build: " + QString(__DATE__) + " " + QString(__TIME__));

  QPixmap pixmap(30, 30);
  pixmap.fill(defaultColors[0]);
  ui->pushButtonChannelColor->setIcon(pixmap);

  ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/run.png"));
}
