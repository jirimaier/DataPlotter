#include "mainwindow.h"

void MainWindow::connectSignals() {
  connect(ui->pushButtonPause, &QPushButton::clicked, ui->plot, &MyMainPlot::togglePause);
  connect(ui->radioButtonCurMain, &QRadioButton::toggled, ui->plot, &MyPlot::setCursorsAccess);
  connect(ui->radioButtonCurXY, &QRadioButton::toggled, ui->plotxy, &MyXYPlot::setCursorsAccess);
  connect(ui->checkBoxVerticalValues, &QCheckBox::toggled, ui->plot, &MyPlot::setShowVerticalValues);
  connect(ui->plot, &MyMainPlot::showPlotStatus, this, &MainWindow::showPlotStatus);
  connect(ui->plot, &MyMainPlot::updateDivs, this, &MainWindow::updateDivs);
  connect(ui->plot, &MyMainPlot::setCursorBounds, this, &MainWindow::setCursorBounds);
  connect(ui->plotxy, &MyXYPlot::setCursorBounds, this, &MainWindow::setCursorBounds);
  connect(ui->doubleSpinBoxRangeHorizontal, SIGNAL(valueChanged(double)), ui->plot, SLOT(setRollingRange(double)));
  connect(ui->doubleSpinBoxRangeVerticalRange, SIGNAL(valueChanged(double)), ui->plot, SLOT(setVerticalRange(double)));
  connect(ui->doubleSpinBoxRangeHorizontal, SIGNAL(valueChanged(double)), ui->dialRollingRange, SLOT(updatePosition(double)));
  connect(ui->doubleSpinBoxRangeVerticalRange, SIGNAL(valueChanged(double)), ui->dialVerticalRange, SLOT(updatePosition(double)));
  connect(ui->doubleSpinBoxChScale, SIGNAL(valueChanged(double)), ui->dialChScale, SLOT(updatePosition(double)));
  connect(ui->dialVerticalCenter, &QDial::valueChanged, ui->plot, &MyMainPlot::setVerticalCenter);
  connect(ui->horizontalScrollBarHorizontal, &QScrollBar::valueChanged, ui->plot, &MyMainPlot::setHorizontalPos);
  connect(ui->checkBoxXYAutoSize, &QCheckBox::toggled, ui->plotxy, &MyXYPlot::setAutoSize);
  connect(ui->lineEditHtitle, &QLineEdit::textChanged, ui->plot, &MyPlot::setXTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plot, &MyPlot::setYTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plotxy, &MyPlot::setXTitle);
  connect(ui->lineEditVtitle, &QLineEdit::textChanged, ui->plotxy, &MyPlot::setYTitle);
  connect(ui->myTerminal, &MyTerminal::sendMessage, this, &MainWindow::printMessage);

  connect(&plotUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePlot);
  connect(&portsRefreshTimer, &QTimer::timeout, this, &MainWindow::comRefresh);
}

void MainWindow::setAdaptiveSpinBoxes() {
// Adaptivní krok není v starším Qt (Win XP)
#if QT_VERSION >= 0x050C00
  ui->doubleSpinBoxChScale->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxRangeHorizontal->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
  ui->doubleSpinBoxRangeVerticalRange->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
#endif
}

void MainWindow::startTimers() {
  portsRefreshTimer.setInterval(500);
  plotUpdateTimer.setInterval(10);

  plotUpdateTimer.start();
  portsRefreshTimer.start();
}

void MainWindow::setGuiDefaults() {
  ui->tabs_right->setCurrentIndex(0);
  ui->tabsControll->setCurrentIndex(0);
  ui->comboBoxOutputLevel->setCurrentIndex((int)OutputLevel::info);
  ui->radioButtonFixedRange->setChecked(true);
  ui->plotxy->setHidden(true);
  ui->plotfft->setHidden(true);
  ui->labelBuildDate->setText("Build: " + QString(__DATE__) + " " + QString(__TIME__));
  ui->pushButtonPause->setIcon(QPixmap(":/images/icons/run.png"));
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
