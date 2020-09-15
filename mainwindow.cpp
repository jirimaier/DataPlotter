#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  init();
}

MainWindow::~MainWindow() {
  delete plotData;
  delete serial;
  delete settings;
  delete ui;
}

void MainWindow::init() {
  settings = new Settings();
  plotData = new PlotData(settings);
  serial = new SerialHandler(settings);
  ui->plot->init(settings, plotData);
  connectSignals();
  changeLanguage();

  ui->tabs_right->setCurrentIndex(0);
  ui->tabs_Plot->setCurrentIndex(0);
  ui->tabWidgetMainArea->setCurrentIndex(0);

  ui->labelBuildDate->setText("Build: " + QString(__DATE__) + " " + QString(__TIME__));
  on_sliderRefreshRate_valueChanged(ui->sliderRefreshRate->value());
  on_pushButtonComRefresh_clicked();

  QPixmap pixmap(30, 30);
  pixmap.fill(defaultColors[0]);
  ui->pushButtonChannelColor->setIcon(pixmap);
  ui->labelCursorChanelColor->setPixmap(pixmap);

  ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/run.png"));

  // updateChScale();
}

void MainWindow::connectSignals() {
  // GUI -> Plotting
  connect(ui->pushButtonPause, SIGNAL(clicked()), ui->plot, SLOT(pauseClicked()));
  connect(ui->pushButtonSingleTriger, SIGNAL(clicked()), ui->plot, SLOT(singleTrigerClicked()));
  connect(ui->checkBoxCurXEn, SIGNAL(toggled(bool)), ui->plot, SLOT(setCurXen(bool)));
  connect(ui->checkBoxCurYEn, SIGNAL(toggled(bool)), ui->plot, SLOT(setCurYen(bool)));
  connect(ui->checkBoxVerticalValues, SIGNAL(toggled(bool)), ui->plot, SLOT(setShowVerticalValues(bool)));
  connect(ui->checkBoxHorizontalValues, SIGNAL(toggled(bool)), ui->plot, SLOT(setShowHorizontalValues(bool)));

  // Plotting -> MainWindow
  connect(ui->plot, SIGNAL(showPlotStatus(int)), this, SLOT(showPlotStatus(int)));
  connect(ui->plot, SIGNAL(setHDivLimits(double)), this, SLOT(setHDivLimits(double)));
  connect(ui->plot, SIGNAL(setVDivLimits(double)), this, SLOT(setVDivLimits(double)));
  connect(ui->plot, SIGNAL(setCursorBounds(double, double, double, double, double, double, double, double)), this, SLOT(setCursorBounds(double, double, double, double, double, double, double, double)));

  // Serial -> MainWindow
  connect(serial, SIGNAL(serialErrorOccurredSignal()), this, SLOT(serialErrorOccurred()));
  connect(serial, SIGNAL(changedMode(int)), this, SLOT(setDataMode(int)));
  connect(serial, SIGNAL(showErrorMessage(QByteArray)), this, SLOT(showErrorMessage(QByteArray)));
  connect(serial, SIGNAL(printMessage(QByteArray, bool)), this, SLOT(printMessage(QByteArray, bool)));
  connect(serial, SIGNAL(newProcessedCommand(QPair<bool, QByteArray>)), this, SLOT(showProcessedCommand(QPair<bool, QByteArray>)));
  connect(serial, SIGNAL(changedBinSettings(Settings::binDataSettings_t)), this, SLOT(changeBinSettings(Settings::binDataSettings_t)));

  // Serial -> Plotting
  connect(serial, SIGNAL(newDataString(QByteArray)), plotData, SLOT(newDataString(QByteArray)));
  connect(serial, SIGNAL(newDataBin(QByteArray)), plotData, SLOT(newDataBin(QByteArray)));

  // Serial -> Terminal
  connect(serial, SIGNAL(printToTerminal(QByteArray)), ui->myTerminal, SLOT(printToTerminal(QByteArray)));
}

void MainWindow::printMessage(QByteArray data, bool urgent) {
  QString message = QString("<font color=grey>%1: </font>").arg(QString(QTime::currentTime().toString("hh:mm:ss")));
  if (urgent)
    message.append(QString("<font color=red>%1</font>").arg(QString(data + "\n")));
  else
    message.append(QString("<font color=black>%1</font>").arg(QString(data + "\n")));
  ui->textEditMessages->append(message);
}

void MainWindow::changeLanguage() {
  if (ui->radioButtonCz->isChecked())
    if (!translator.load(":/translations/translation_cz.qm"))
      return;
  if (ui->radioButtonEn->isChecked())
    if (!translator.load(":/translations/translation_en.qm"))
      return;
  qApp->installTranslator(&translator);
  ui->retranslateUi(this);
}

void MainWindow::showPlotStatus(int type) {
  if (type == PLOT_STATUS_PAUSE) {
    ui->pushButtonPause->setText(tr("resume"));
    ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/pause.png"));
  }
  if (type == PLOT_STATUS_RUN) {
    ui->pushButtonPause->setText(tr("pause"));
    ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/run.png"));
  }
  if (type == PLOT_STATUS_SINGLETRIGER) {
    ui->pushButtonPause->setText(tr("normal"));
    ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/single.png"));
  }
  ui->pushButtonSingleTriger->setEnabled(type != PLOT_STATUS_SINGLETRIGER);
}

void MainWindow::setHDivLimits(double hRange) {
  int i = roundToStandardValue(hRange);
  ui->dialhorizontalDiv->setMinimum(i - 5);
  ui->dialhorizontalDiv->setMaximum(i - 1);
}

void MainWindow::setVDivLimits(double vRange) {
  int i = roundToStandardValue(vRange);
  ui->dialVerticalDiv->setMinimum(i - 5);
  ui->dialVerticalDiv->setMaximum(i - 1);
}

void MainWindow::serialErrorOccurred() {
  ui->lineEditPortInfo->setText(tr("Error"));
  ui->pushButtonDisconnect->setEnabled(false);
  ui->pushButtonConnect->setEnabled(true);
}

void MainWindow::setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminfull, double xmaxfull, double yminfull, double ymaxfull) {
  if (xmaxfull < xmax)
    xmaxfull = xmax;
  if (xminfull > xmin)
    xminfull = xmin;
  if (ymaxfull < ymax)
    ymaxfull = ymax;
  if (yminfull > ymin)
    yminfull = ymin;
  ui->verticalScrollBarCursorY1->setMinimum(yminfull * 1000);
  ui->verticalScrollBarCursorY1->setMaximum(ymaxfull * 1000);
  ui->verticalScrollBarCursorY2->setMinimum(yminfull * 1000);
  ui->verticalScrollBarCursorY2->setMaximum(ymaxfull * 1000);
  ui->horizontalScrollBarCursorX1->setMinimum(xminfull * 1000);
  ui->horizontalScrollBarCursorX1->setMaximum(xmaxfull * 1000);
  ui->horizontalScrollBarCursorX2->setMinimum(xminfull * 1000);
  ui->horizontalScrollBarCursorX2->setMaximum(xmaxfull * 1000);
  ui->verticalScrollBarCursorY1->setSingleStep((ymax - ymin) * 2);
  ui->verticalScrollBarCursorY2->setSingleStep((ymax - ymin) * 2);
  ui->verticalScrollBarCursorY1->setPageStep((ymax - ymin) * 2);
  ui->verticalScrollBarCursorY2->setPageStep((ymax - ymin) * 2);
  int stepsize = (xmax - xmin) * 2;
  ui->horizontalScrollBarCursorX1->setSingleStep(stepsize);
  ui->horizontalScrollBarCursorX2->setSingleStep(stepsize);
  ui->horizontalScrollBarCursorX1->setPageStep(stepsize);
  ui->horizontalScrollBarCursorX2->setPageStep(stepsize);
}

void MainWindow::setDataMode(int mode) {
  if (ui->checkBoxPreventModeChange->isChecked())
    return;
  ui->listWidgetDataMode->setCurrentRow(mode);
  settings->dataMode = mode;
}

void MainWindow::changeBinSettings(Settings::binDataSettings_t in_settings) {
  if (ui->checkBoxBinarySettingsOverride->isChecked())
    return;
  this->settings->binDataSettings = in_settings;
  if (!ui->checkBoxBinarySettingsOverride->isChecked()) {
    ui->spinBoxDataBinaryBits->setValue(settings->binDataSettings.bits);
    ui->doubleSpinBoxBinaryDataMin->setValue(settings->binDataSettings.valueMin);
    ui->doubleSpinBoxBinarydataMax->setValue(settings->binDataSettings.valueMax);
    ui->doubleSpinBoxBinaryTimestep->setValue(settings->binDataSettings.timeStep);
    ui->spinBoxBinaryDataNumCh->setValue(settings->binDataSettings.numCh);
    ui->spinBoxBinaryDataFirstCh->setValue(settings->binDataSettings.firstCh);
    ui->checkBoxBinContinuous->setChecked(settings->binDataSettings.continuous);
  }
}

void MainWindow::showErrorMessage(QByteArray message) {
  QMessageBox msgBox;
  msgBox.setText(tr("Error:"));
  msgBox.setInformativeText(QString(message));
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.exec();
}

void MainWindow::showProcessedCommand(QPair<bool, QByteArray> message) {
  if (!ui->checkBoxShowCommands->isChecked())
    return;
  QString stringMessage;
  if (!message.first && settings->dataMode == DATA_MODE_DATA_BINARY) {
    stringMessage = message.second.toHex(' ');
    stringMessage = "<font color=navy>" + stringMessage + "</font>";
  } else {
    stringMessage = QString(message.second);
    stringMessage.replace(QChar('\r'), "<font color=navy>[CR]</font>");
    stringMessage.replace(QChar('\n'), "<font color=navy>[LF]</font>");
    stringMessage.replace(QChar('\t'), "<font color=navy>[TAB]</font>");
    stringMessage.replace(QChar(27), "<font color=navy>[ESC]</font>");
  }
  ui->textEditSerialDebug->append(QString("<font color=gray>%1</font><font color=black>%2</font>").arg(message.first ? "Cmd: " : "Data: ", stringMessage));
}

int MainWindow::roundToStandardValue(double value) {
  for (int i = 0; i < 28; i++)
    if (value <= logaritmicSettings[i])
      return i;
  return 28;
}

bool MainWindow::isStandardValue(double value) {
  for (int i = 0; i < 28; i++)
    if (value == logaritmicSettings[i])
      return true;
  return false;
}

void MainWindow::updateChScale() {
  double perDiv = ui->doubleSpinBoxRangeVerticalDiv->value() / settings->channelSettings.at(ui->spinBoxChannelSelect->value() - 1)->scale;
  ui->labelChScale->setText(QString::number(perDiv) + tr(" / Div"));
}

/*void MainWindow::useSettings(QString settings) {
  QStringList lines = settings.split("\n");
  for (QStringList::Iterator it = lines.begin(); it != lines.end(); it++) {
    *it = it->toLower();
    if (it->left(2) == "//")
      continue;
    if (it->left(it->indexOf(':')) == "vrange") {
      double value = it->mid(it->indexOf(':') + 1).toDouble();
      ui->dialVerticalRange->setValue(roundToStandardValue(value));
      ui->doubleSpinBoxRangeVerticalRange->setValue(value);
      plotting->setVerticalRange(value);
      continue;
    }
    if (it->left(it->indexOf(':')) == "vdiv") {
      double value = it->mid(it->indexOf(':') + 1).toDouble();
      ui->dialVerticalDiv->setValue(roundToStandardValue(value));
      ui->doubleSpinBoxRangeVerticalDiv->setValue(value);
      plotting->setVerticalDiv(value);
      continue;
    }
    if (it->left(it->indexOf(':')) == "rollrange") {
      double value = it->mid(it->indexOf(':') + 1).toDouble();
      ui->dialRollingRange->setValue(roundToStandardValue(value));
      ui->doubleSpinBoxRangeHorizontal->setValue(value);
      plotting->setRollingLength(value);
      continue;
    }
    if (it->left(it->indexOf(':')) == "hdiv") {
      double value = it->mid(it->indexOf(':') + 1).toDouble();
      ui->dialhorizontalDiv->setValue(roundToStandardValue(value));
      ui->doubleSpinBoxRangeHorizontalDiv->setValue(value);
      plotting->setHorizontalDiv(value);
      continue;
    }
    if (it->left(it->indexOf(':')) == "chclr") {
      QStringList value = it->mid(it->indexOf(':') + 1).split(',');
      if (value.length() == 4 && value.at(0).toInt() > 0 && value.at(0).toInt() <= 64) {
        plotting->channel(value.at(0).toInt())->changeColor(QColor::fromRgb(value.at(1).toInt(), value.at(2).toInt(), value.at(3).toInt()));
        on_spinBoxChannelSelect_valueChanged(ui->spinBoxChannelSelect->value()-1);
        continue;
      }
    }
    if (it->left(it->indexOf(':')) == "choff") {
      QStringList value = it->mid(it->indexOf(':') + 1).split(',');
      if (value.length() == 2 && value.at(0).toInt() > 0 && value.at(0).toInt() <= 64) {
        plotting->channel(value.at(0).toInt())->changeOffset(value.at(1).toDouble());
        on_spinBoxChannelSelect_valueChanged(ui->spinBoxChannelSelect->value()-1);
        continue;
      }
    }
    QMessageBox msgBox;
    msgBox.setText(tr("Unknown settings line."));
    msgBox.setInformativeText(*it);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }
}*/

void MainWindow::on_pushButtonComRefresh_clicked() {
  ui->comboBoxCom->clear();
  ui->comboBoxCom->addItems(serial->refresh());
}

void MainWindow::on_pushButtonConnect_clicked() {
  if (serial->connectSerial(ui->comboBoxCom->currentIndex(), ui->comboBoxBaud->currentText().toUInt())) {
    ui->lineEditPortInfo->setText(tr("Connected to ") + serial->currentPort() + tr(" at ") + QString::number(serial->currentBaud()) + tr(" bps"));
    ui->pushButtonDisconnect->setEnabled(true);
    ui->pushButtonConnect->setEnabled(false);
  } else {
    ui->lineEditPortInfo->setText(tr("Failed"));
    ui->pushButtonDisconnect->setEnabled(false);
    ui->pushButtonConnect->setEnabled(true);
  }
}

void MainWindow::on_sliderRefreshRate_valueChanged(int value) {
  ui->labelRefreshRate->setText(QString::number((int)refreshRates[value]) + " Hz");
  ui->plot->setRefreshPeriod(round(1000 / refreshRates[value]));
}

void MainWindow::on_tabs_right_currentChanged(int index) {
  if (index == 2)
    ui->lineEditCommand->setFocus();
}

void MainWindow::radioButtonRangeType_toggled(bool checked) {
  if (checked == false)
    return;
  int type = PLOT_RANGE_FIXED;
  if (ui->radioButtonRangeFree->isChecked())
    type = PLOT_RANGE_FREE;
  if (ui->radioButtonRangeRolling->isChecked())
    type = PLOT_RANGE_ROLLING;
  settings->plotRangeType = type;
  ui->plot->setRangeType(type);
}

void MainWindow::on_dialRollingRange_valueChanged(int value) { ui->doubleSpinBoxRangeHorizontal->setValue(logaritmicSettings[value]); }

void MainWindow::on_dialVerticalRange_valueChanged(int value) { ui->doubleSpinBoxRangeVerticalRange->setValue(logaritmicSettings[value]); }

void MainWindow::on_pushButtonClearChannels_clicked() {
  plotData->clearChannels();
  ui->plot->resetChannels();
}

void MainWindow::on_pushButtonChannelColor_clicked() {
  QColor color = QColorDialog::getColor(settings->channelSettings.at(ui->spinBoxChannelSelect->value() - 1)->color);
  if (!color.isValid())
    return;
  settings->channelSettings.at(ui->spinBoxChannelSelect->value() - 1)->color = color;
  on_spinBoxChannelSelect_valueChanged(ui->spinBoxChannelSelect->value());
  ui->plot->updateVisuals();
}

void MainWindow::on_spinBoxChannelSelect_valueChanged(int arg1) {
  ui->comboBoxGraphStyle->setCurrentIndex(settings->channelSettings.at(arg1 - 1)->style);
  QPixmap pixmap(30, 30);
  pixmap.fill(settings->channelSettings.at(arg1 - 1)->color);
  ui->pushButtonChannelColor->setIcon(pixmap);
  double offset = settings->channelSettings.at(arg1 - 1)->offset;
  double scale = settings->channelSettings.at(arg1 - 1)->scale;
  ui->doubleSpinBoxChOffset->setValue(offset);
  if (offset < ui->doubleSpinBoxRangeVerticalRange->value() / 2)
    ui->dialOffset->setValue(offset / ui->doubleSpinBoxRangeVerticalRange->value() * 100 * 2);
  ui->doubleSpinBoxChScale->setValue(scale);
  if (isStandardValue(scale))
    ui->dialChScale->setValue(roundToStandardValue(scale));
  updateChScale();
}

void MainWindow::on_doubleSpinBoxChOffset_valueChanged(double arg1) {
  settings->channelSettings.at(ui->spinBoxChannelSelect->value() - 1)->offset = arg1;
  if (ui->spinBoxCursorCh->value() == ui->spinBoxChannelSelect->value())
    scrollBarCursor_valueChanged();
}

void MainWindow::on_dialOffset_valueChanged(int value) { ui->doubleSpinBoxChOffset->setValue(ui->doubleSpinBoxRangeVerticalRange->value() / 2 * value * 0.01); }

void MainWindow::on_dialVerticalDiv_valueChanged(int value) { ui->doubleSpinBoxRangeVerticalDiv->setValue(logaritmicSettings[value]); }

void MainWindow::on_pushButtonVerticalZero_clicked() { ui->verticalScrollBarVerticalCenter->setValue(0); }

void MainWindow::on_dialhorizontalDiv_valueChanged(int value) { ui->doubleSpinBoxRangeHorizontalDiv->setValue(logaritmicSettings[value]); }

void MainWindow::on_comboBoxGraphStyle_currentIndexChanged(int index) {
  settings->channelSettings.at(ui->spinBoxChannelSelect->value() - 1)->style = index;
  ui->plot->updateVisuals();
}

void MainWindow::scrollBarCursor_valueChanged() {
  double x1 = ui->horizontalScrollBarCursorX1->value() / 1000.0;
  double x2 = ui->horizontalScrollBarCursorX2->value() / 1000.0;
  double y1 = ui->verticalScrollBarCursorY1->value() / 1000.0;
  double y2 = ui->verticalScrollBarCursorY2->value() / 1000.0;

  ui->labelCursorX1->setText("X1: " + QString::number(x1, 'f', 3));
  ui->labelCursorX2->setText("X2: " + QString::number(x2, 'f', 3));
  ui->labelCursorY1->setText("Y1: " + QString::number((y1 - settings->channelSettings.at(ui->spinBoxCursorCh->value() - 1)->offset) / settings->channelSettings.at(ui->spinBoxCursorCh->value() - 1)->scale, 'f', 3));
  ui->labelCursorY2->setText("Y2: " + QString::number((y2 - settings->channelSettings.at(ui->spinBoxCursorCh->value() - 1)->offset) / settings->channelSettings.at(ui->spinBoxCursorCh->value() - 1)->scale, 'f', 3));
  ui->labelCursordX->setText(tr("dX: ") + QString::number(abs(x2 - x1)));
  ui->labelCursordY->setText(tr("dY: ") + QString::number(abs(y2 - y1) / settings->channelSettings.at(ui->spinBoxCursorCh->value() - 1)->scale));
  ui->plot->updateCursors(x1, x2, y1, y2);
}

void MainWindow::on_spinBoxCursorCh_valueChanged(int arg1) {
  QPixmap pixmap(1, 1);
  pixmap.fill(settings->channelSettings.at(arg1 - 1)->color);
  ui->labelCursorChanelColor->setPixmap(pixmap);
  scrollBarCursor_valueChanged();
}

void MainWindow::on_pushButtonDisconnect_clicked() {
  serial->disconnectSerial();
  ui->lineEditPortInfo->setText(tr("Not connected"));
  ui->pushButtonDisconnect->setEnabled(false);
  ui->pushButtonConnect->setEnabled(true);
}

void MainWindow::on_pushButtonSendCommand_clicked() {
  QByteArray data = ui->lineEditCommand->text().toUtf8();
  serial->write(data);
  ui->lineEditCommand->clear();
}

void MainWindow::on_spinBoxDataBinaryBits_valueChanged(int arg1) {
  ui->doubleSpinBoxBinarydataMax->setPrefix("0x" + QString::number(((quint64)1 << arg1) - 1, 16).toUpper() + " = ");
  ui->doubleSpinBoxBinarydataMax->setValue((1 << arg1) - 1);
  settings->binDataSettings.bits = arg1;
}

void MainWindow::on_spinBoxBinaryDataNumCh_valueChanged(int arg1) {
  ui->spinBoxBinaryDataFirstCh->setMaximum(65 - arg1);
  settings->binDataSettings.numCh = arg1;
}

void MainWindow::on_doubleSpinBoxChScale_valueChanged(double arg1) {
  settings->channelSettings.at(ui->spinBoxChannelSelect->value() - 1)->scale = arg1;
  if (ui->spinBoxCursorCh->value() == ui->spinBoxChannelSelect->value() - 1)
    scrollBarCursor_valueChanged();
  updateChScale();
}

void MainWindow::on_dialChScale_valueChanged(int value) {
  if (isStandardValue(ui->doubleSpinBoxChScale->value()))
    ui->doubleSpinBoxChScale->setValue(logaritmicSettings[value]);
  else {
    ui->dialChScale->setValue(roundToStandardValue(ui->doubleSpinBoxChScale->value()));
    ui->doubleSpinBoxChScale->setValue(logaritmicSettings[ui->dialChScale->value()]);
  }
}

void MainWindow::on_doubleSpinBoxRangeVerticalDiv_valueChanged(double arg1) {
  ui->plot->setVerticalDiv(arg1);
  updateChScale();
}

void MainWindow::on_pushButtonSelectedCSV_clicked() {
  int ch = ui->spinBoxChannelSelect->value() - 1;
  QString fileName = QFileDialog::getSaveFileName(this, tr("Export Channel %1").arg(ch), QString(QCoreApplication::applicationDirPath()), tr("Comma separated values (*.csv)"));
  if (fileName.isEmpty())
    return;
  QFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Truncate)) {
    // file.write(plotting->chToCSV(ch).toUtf8());
    file.close();
  } else {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant write to file."));
    msgBox.setInformativeText(tr("This may be because file is opened in another program."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_dialZoom_valueChanged(int value) {
  settings->plotSettings.zoom = value;
  ui->horizontalScrollBarHorizontal->setMinimum(value / 2);
  ui->horizontalScrollBarHorizontal->setMaximum(1000 - value / 2);
  ui->horizontalScrollBarHorizontal->setPageStep(value);
}

void MainWindow::on_doubleSpinBoxRangeHorizontalDiv_valueChanged(double arg1) { ui->plot->setHorizontalDiv(arg1); }
