#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  init();
}

MainWindow::~MainWindow() {
  delete plotting;
  delete serial;
  delete ui;
}

void MainWindow::init() {
  plotting = new Plotting(ui->plot, ui->tableWidget, ui->horizontalScrollBarHorizontal);
  serial = new SerialHandler();
  connectSignals();
  changeLanguage();
  loadIcons();

  ui->tabs_right->setCurrentIndex(0);
  ui->tabs_Plot->setCurrentIndex(0);
  ui->tabWidgetMainArea->setCurrentIndex(0);
  on_sliderRefreshRate_valueChanged(ui->sliderRefreshRate->value());
  on_pushButtonComRefresh_clicked();

  QPixmap pixmap(30, 30);
  pixmap.fill(defaultColors[0]);
  ui->pushButtonChannelColor->setIcon(pixmap);
  ui->labelCursorChanelColor->setPixmap(pixmap);

  ui->labelPauseResume->setPixmap(resume);

  useSettings(defaultSettings);

  updateChScale();
}

void MainWindow::connectSignals() {
  // GUI -> Plotting
  connect(ui->doubleSpinBoxRangeVerticalRange, SIGNAL(valueChanged(double)), plotting, SLOT(setVerticalRange(double)));
  connect(ui->doubleSpinBoxRangeHorizontal, SIGNAL(valueChanged(double)), plotting, SLOT(setRollingLength(double)));
  connect(ui->doubleSpinBoxRangeHorizontalDiv, SIGNAL(valueChanged(double)), plotting, SLOT(setHorizontalDiv(double)));
  connect(ui->verticalScrollBarVerticalCenter, SIGNAL(valueChanged(int)), plotting, SLOT(setVerticalCenter(int)));
  connect(ui->pushButtonPause, SIGNAL(clicked()), plotting, SLOT(pauseClicked()));
  connect(ui->pushButtonSingleTriger, SIGNAL(clicked()), plotting, SLOT(singleTrigerClicked()));
  connect(ui->checkBoxVerticalValues, SIGNAL(toggled(bool)), plotting, SLOT(setShowVerticalValues(bool)));
  connect(ui->checkBoxHorizontalValues, SIGNAL(toggled(bool)), plotting, SLOT(setShowHorizontalValues(bool)));
  connect(ui->dialZoom, SIGNAL(valueChanged(int)), plotting, SLOT(setZoom(int)));
  connect(ui->checkBoxPlotOprnGL, SIGNAL(toggled(bool)), plotting, SLOT(setOpenGL(bool)));
  connect(ui->checkBoxCurXEn, SIGNAL(toggled(bool)), plotting, SLOT(setCurXen(bool)));
  connect(ui->checkBoxCurYEn, SIGNAL(toggled(bool)), plotting, SLOT(setCurYen(bool)));

  // Plotting -> MainWindow
  connect(plotting, SIGNAL(showPlotStatus(int)), this, SLOT(showPlotStatus(int)));
  connect(plotting, SIGNAL(setHDivLimits(double)), this, SLOT(setHDivLimits(double)));
  connect(plotting, SIGNAL(setVDivLimits(double)), this, SLOT(setVDivLimits(double)));
  connect(plotting, SIGNAL(setCursorBounds(double, double, double, double, double, double, double, double)), this, SLOT(setCursorBounds(double, double, double, double, double, double, double, double)));

  // Serial -> MainWindow
  connect(serial, SIGNAL(serialErrorOccurredSignal()), this, SLOT(serialErrorOccurred()));
  connect(serial, SIGNAL(changedMode(int)), this, SLOT(setDataMode(int)));
  connect(serial, SIGNAL(showErrorMessage(QByteArray)), this, SLOT(showErrorMessage(QByteArray)));
  connect(serial, SIGNAL(printMessage(QByteArray, bool)), this, SLOT(printMessage(QByteArray, bool)));
  connect(serial, SIGNAL(newProcessedCommand(QPair<bool, QByteArray>)), this, SLOT(showProcessedCommand(QPair<bool, QByteArray>)));
  connect(serial, SIGNAL(changedBitMode(int, double, double, double, int, int, bool)), this, SLOT(setBitMode(int, double, double, double, int, int, bool)));

  // GUI -> Serial
  connect(ui->listWidgetDataMode, SIGNAL(currentRowChanged(int)), serial, SLOT(changeMode(int)));
  connect(ui->spinBoxDataBinaryBits, SIGNAL(valueChanged(int)), serial, SLOT(setBits(int)));
  connect(ui->spinBoxBinaryDataNumCh, SIGNAL(valueChanged(int)), serial, SLOT(setNumCh(int)));
  connect(ui->spinBoxBinaryDataFirstCh, SIGNAL(valueChanged(int)), serial, SLOT(setFirstCh(int)));
  connect(ui->doubleSpinBoxBinaryDataMin, SIGNAL(valueChanged(double)), serial, SLOT(setValueMin(double)));
  connect(ui->doubleSpinBoxBinarydataMax, SIGNAL(valueChanged(double)), serial, SLOT(setValueMax(double)));
  connect(ui->doubleSpinBoxBinaryTimestep, SIGNAL(valueChanged(double)), serial, SLOT(setTimeStep(double)));
  connect(ui->checkBoxBinContinuous, SIGNAL(toggled(bool)), serial, SLOT(setContinuous(bool)));

  // Serial -> Plotting
  connect(serial, SIGNAL(newDataString(QByteArray)), plotting, SLOT(newDataString(QByteArray)));
  connect(serial, SIGNAL(newDataBin(QByteArray, int, double, double, double, int, int, bool)), plotting, SLOT(newDataBin(QByteArray, int, double, double, double, int, int, bool)));

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
  if (ui->radioButtonCz->isChecked()) {
    if (!translator.load("translation_cz", QCoreApplication::applicationDirPath() + "/languages")) {
      QMessageBox msgBox;
      msgBox.setText("Can't load translation file.");
      msgBox.setInformativeText("translation_cz.qm");
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.exec();
      return;
    }
  }
  if (ui->radioButtonEn->isChecked()) {
    if (!translator.load("translation_en", QCoreApplication::applicationDirPath() + "/languages")) {
      QMessageBox msgBox;
      msgBox.setText("Can't load translation file.");
      msgBox.setInformativeText("translation_en.qm");
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.exec();
      return;
    }
  }
  qApp->installTranslator(&translator);
  ui->retranslateUi(this);
}

void MainWindow::showPlotStatus(int type) {
  if (type == PLOT_STATUS_PAUSE) {
    ui->pushButtonPause->setText(tr("resume"));
    ui->labelPauseResume->setPixmap(pause);
  }
  if (type == PLOT_STATUS_RUN) {
    ui->pushButtonPause->setText(tr("pause"));
    ui->labelPauseResume->setPixmap(resume);
  }
  if (type == PLOT_STATUS_SINGLETRIGER) {
    ui->pushButtonPause->setText(tr("normal"));
    ui->labelPauseResume->setPixmap(single);
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
  if (!ui->checkBoxPreventModeChange->isChecked())
    ui->listWidgetDataMode->setCurrentRow(mode);
}

void MainWindow::showErrorMessage(QByteArray message) {
  QMessageBox msgBox;
  msgBox.setText(tr("message from connected device:"));
  msgBox.setInformativeText(QString(message));
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.exec();
}

void MainWindow::showProcessedCommand(QPair<bool, QByteArray> message) {
  QString stringMessage;
  if (!message.first && serial->currentMode() == DATA_MODE_DATA_BINARY)
    stringMessage = message.second.toHex(' ');
  else {
    stringMessage = QString(message.second);
    message.second.replace(QChar('\r'), "<font color=navy>[CR]</font>");
    message.second.replace(QChar('\n'), "<font color=navy>[LF]</font>");
    message.second.replace(QChar('\t'), "<font color=navy>[TAB]</font>");
    message.second.replace(QChar(27), "<font color=navy>[ESC]</font>");
  }
  ui->textEditSerialDebug->append(QString("<font color=gray>%1</font><font color=black>%2</font>").arg(message.first ? "Cmd: " : "Data: ", stringMessage));
}

void MainWindow::loadIcons() {
  bool failed = false;
  failed |= !resume.load(QCoreApplication::applicationDirPath() + "/icons/run.png");
  failed |= !pause.load(QCoreApplication::applicationDirPath() + "/icons/pause.png");
  failed |= !single.load(QCoreApplication::applicationDirPath() + "/icons/single.png");
  if (failed) {
    QMessageBox msgBox;
    msgBox.setText(tr("Can not load icons."));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }
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
  double perDiv = ui->doubleSpinBoxRangeVerticalDiv->value() / plotting->channel(ui->spinBoxChannelSelect->value())->getScale();
  ui->labelChScale->setText(QString::number(perDiv) + tr(" / Div"));
}

void MainWindow::useSettings(QString settings) {
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
        on_spinBoxChannelSelect_valueChanged(ui->spinBoxChannelSelect->value());
        continue;
      }
    }
    if (it->left(it->indexOf(':')) == "choff") {
      QStringList value = it->mid(it->indexOf(':') + 1).split(',');
      if (value.length() == 2 && value.at(0).toInt() > 0 && value.at(0).toInt() <= 64) {
        plotting->channel(value.at(0).toInt())->changeOffset(value.at(1).toDouble());
        on_spinBoxChannelSelect_valueChanged(ui->spinBoxChannelSelect->value());
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
}

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
  plotting->setRefreshPeriod(round(1000 / refreshRates[value]));
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
  plotting->setRangeType(type);
}

void MainWindow::on_dialRollingRange_valueChanged(int value) { ui->doubleSpinBoxRangeHorizontal->setValue(logaritmicSettings[value]); }

void MainWindow::on_dialVerticalRange_valueChanged(int value) { ui->doubleSpinBoxRangeVerticalRange->setValue(logaritmicSettings[value]); }

void MainWindow::on_pushButtonClearChannels_clicked() { plotting->clearChannels(); }

void MainWindow::on_pushButtonChannelColor_clicked() {
  QColor color = QColorDialog::getColor(plotting->channel(ui->spinBoxChannelSelect->value())->getColor());
  if (!color.isValid())
    return;
  plotting->channel(ui->spinBoxChannelSelect->value())->changeColor(color);
  on_spinBoxChannelSelect_valueChanged(ui->spinBoxChannelSelect->value());
}

void MainWindow::on_spinBoxChannelSelect_valueChanged(int arg1) {
  ui->comboBoxGraphStyle->setCurrentIndex(plotting->channel(arg1)->getStyle());
  QPixmap pixmap(30, 30);
  pixmap.fill(plotting->channel(arg1)->getColor());
  ui->pushButtonChannelColor->setIcon(pixmap);
  double offset = plotting->channel(arg1)->getOffset();
  double scale = plotting->channel(arg1)->getScale();
  ui->doubleSpinBoxChOffset->setValue(offset);
  if (offset < ui->doubleSpinBoxRangeVerticalRange->value() / 2)
    ui->dialOffset->setValue(offset / ui->doubleSpinBoxRangeVerticalRange->value() * 100 * 2);
  ui->doubleSpinBoxChScale->setValue(scale);
  if (isStandardValue(scale))
    ui->dialChScale->setValue(roundToStandardValue(scale));
  updateChScale();
}

void MainWindow::on_doubleSpinBoxChOffset_valueChanged(double arg1) {
  plotting->channel(ui->spinBoxChannelSelect->value())->changeOffset(arg1);
  if (ui->spinBoxCursorCh->value() == ui->spinBoxChannelSelect->value())
    scrollBarCursor_valueChanged();
}

void MainWindow::on_dialOffset_valueChanged(int value) { ui->doubleSpinBoxChOffset->setValue(ui->doubleSpinBoxRangeVerticalRange->value() / 2 * value * 0.01); }

void MainWindow::on_dialVerticalDiv_valueChanged(int value) { ui->doubleSpinBoxRangeVerticalDiv->setValue(logaritmicSettings[value]); }

void MainWindow::on_pushButtonVerticalZero_clicked() { ui->verticalScrollBarVerticalCenter->setValue(0); }

void MainWindow::on_dialhorizontalDiv_valueChanged(int value) { ui->doubleSpinBoxRangeHorizontalDiv->setValue(logaritmicSettings[value]); }

void MainWindow::on_comboBoxGraphStyle_currentIndexChanged(int index) { plotting->channel(ui->spinBoxChannelSelect->value())->setStyle(index); }

void MainWindow::scrollBarCursor_valueChanged() {
  double x1 = ui->horizontalScrollBarCursorX1->value() / 1000.0;
  double x2 = ui->horizontalScrollBarCursorX2->value() / 1000.0;
  double y1 = ui->verticalScrollBarCursorY1->value() / 1000.0;
  double y2 = ui->verticalScrollBarCursorY2->value() / 1000.0;

  ui->labelCursorX1->setText("X1: " + QString::number(x1, 'f', 3));
  ui->labelCursorX2->setText("X2: " + QString::number(x2, 'f', 3));
  ui->labelCursorY1->setText("Y1: " + QString::number((y1 - plotting->channel(ui->spinBoxCursorCh->value())->getOffset()) / plotting->channel(ui->spinBoxCursorCh->value())->getScale(), 'f', 3));
  ui->labelCursorY2->setText("Y2: " + QString::number((y2 - plotting->channel(ui->spinBoxCursorCh->value())->getOffset()) / plotting->channel(ui->spinBoxCursorCh->value())->getScale(), 'f', 3));
  ui->labelCursordX->setText(tr("dX: ") + QString::number(abs(x2 - x1)));
  ui->labelCursordY->setText(tr("dY: ") + QString::number(abs(y2 - y1) / plotting->channel(ui->spinBoxCursorCh->value())->getScale()));
  plotting->updateCursors(x1, x2, y1, y2);
}

void MainWindow::on_spinBoxCursorCh_valueChanged(int arg1) {
  QPixmap pixmap(1, 1);
  pixmap.fill(plotting->channel(arg1)->getColor());
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
}

void MainWindow::on_spinBoxBinaryDataNumCh_valueChanged(int arg1) { ui->spinBoxBinaryDataFirstCh->setMaximum(65 - arg1); }

void MainWindow::setBitMode(int bits, double valMin, double valMax, double timeStep, int numCh, int firstCh, bool continuous) {
  if (!ui->checkBoxBinarySettingsOverride->isChecked()) {
    ui->spinBoxDataBinaryBits->setValue(bits);
    ui->doubleSpinBoxBinaryDataMin->setValue(valMin);
    ui->doubleSpinBoxBinarydataMax->setValue(valMax);
    ui->doubleSpinBoxBinaryTimestep->setValue(timeStep);
    ui->spinBoxBinaryDataNumCh->setValue(numCh);
    ui->spinBoxBinaryDataFirstCh->setValue(firstCh);
    ui->checkBoxBinContinuous->setChecked(continuous);
  }
}

void MainWindow::on_doubleSpinBoxChScale_valueChanged(double arg1) {
  plotting->channel(ui->spinBoxChannelSelect->value())->changeScale(arg1);
  if (ui->spinBoxCursorCh->value() == ui->spinBoxChannelSelect->value())
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
  plotting->setVerticalDiv(arg1);
  updateChScale();
}
