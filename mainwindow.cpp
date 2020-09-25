#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init() {
  connectSignals();
  changeLanguage();

  ui->tabs_right->setCurrentIndex(0);
  ui->tabs_Plot->setCurrentIndex(0);
  ui->comboBoxPlotRangeType->setCurrentIndex(PLOT_RANGE_FIXED);
  ui->labelBuildDate->setText("Build: " + QString(__DATE__) + " " + QString(__TIME__));

  QPixmap pixmap(30, 30);
  pixmap.fill(defaultColors[0]);
  ui->pushButtonChannelColor->setIcon(pixmap);

  ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/run.png"));

  updateChScale();

  portsRefreshTimer.setInterval(500);
  plotUpdateTimer.setInterval(10);
  listUpdateTimer.setInterval(100);
  connect(&plotUpdateTimer, &QTimer::timeout, ui->plot, &MyPlot::update);
  connect(&listUpdateTimer, &QTimer::timeout, this, &MainWindow::updateReceivedList);
  connect(&portsRefreshTimer, &QTimer::timeout, this, &MainWindow::comRefresh);
  plotUpdateTimer.start();
  listUpdateTimer.start();
  portsRefreshTimer.start();
}

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
  connect(ui->verticalScrollBarVerticalCenter, &QScrollBar::valueChanged, ui->plot, &MyPlot::setVerticalCenter);
  connect(ui->horizontalScrollBarHorizontal, &QScrollBar::valueChanged, ui->plot, &MyPlot::setHorizontalPos);
}
void MainWindow::printMessage(QByteArray data, bool urgent) {
  QString message = QString("<font color=grey>%1: </font>").arg(QString(QTime::currentTime().toString("hh:mm:ss")));
  if (urgent)
    message.append(QString("<font color=red>%1</font>").arg(QString(data + "\n")));
  else
    message.append(QString("<font color=black>%1</font>").arg(QString(data + "\n")));
  ui->textEditMessages->append(message);
}

void MainWindow::changeLanguage(QString code) {
  QTranslator translator;
  if (!translator.load(QString(":/translations/translation_%1.qm").arg(code))) {
    qDebug() << "Can not load " << QString(":/translations/translation_%1.qm").arg(code);
    return;
  }
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

void MainWindow::changedDataMode(int mode) {
  if (ui->checkBoxPreventModeChange->isChecked())
    return;
  ui->listWidgetDataMode->setCurrentRow(mode);
  emit
}

void MainWindow::changeBinSettings(BinDataSettings_t settings) {
  if (ui->checkBoxBinarySettingsOverride->isChecked())
    return;
  if (!ui->checkBoxBinarySettingsOverride->isChecked()) {
    ui->spinBoxDataBinaryBits->setValue(settings.bits);
    ui->doubleSpinBoxBinaryDataMin->setValue(settings.valueMin);
    ui->doubleSpinBoxBinarydataMax->setValue(settings.valueMax);
    ui->doubleSpinBoxBinaryTimestep->setValue(settings.timeStep);
    ui->spinBoxBinaryDataNumCh->setValue(settings.numCh);
    ui->spinBoxBinaryDataFirstCh->setValue(settings.firstCh);
    ui->checkBoxBinContinuous->setChecked(settings.continuous);
  }
}

void MainWindow::showProcessedCommand(QByteArray message) {
  QString stringMessage = QString(message);
  if (stringMessage.length() == message.length()) {
    stringMessage.replace(QChar('\r'), "<font color=navy>[CR]</font>");
    stringMessage.replace(QChar('\n'), "<font color=navy>[LF]</font>");
    stringMessage.replace(QChar('\t'), "<font color=navy>[TAB]</font>");
    stringMessage.replace(QChar(27), "<font color=navy>[ESC]</font>");
  } else {
// Oddělení bajtů mezerami nefunguje v starším Qt (Win XP)
#if QT_VERSION >= 0x050900
    stringMessage = message.toHex(' ');
#else
    stringMessage = message.toHex();
#endif
    stringMessage = "<font color=navy>" + stringMessage + "</font>";
  }
  receivedListBuffer.append(stringMessage);
}

void MainWindow::updateReceivedList() {
  foreach (QString line, receivedListBuffer)
    ui->textEditSerialDebug->append(line);
  receivedListBuffer.clear();
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
  double perDiv = ui->plot->getCHDiv(ui->spinBoxChannelSelect->value());
  ui->labelChScale->setText(QString::number(perDiv) + tr(" / Div"));
}

void MainWindow::comRefresh() {
  QList<QSerialPortInfo> newPorts = QSerialPortInfo::availablePorts();
  bool change = false;
  if (newPorts.length() == portList.length()) {
    for (quint8 i = 0; i < newPorts.length(); i++)
      if (newPorts.at(i).portName() != portList.at(i).portName()) {
        change = true;
        break;
      }
  } else
    change = true;
  if (change) {
    qDebug() << "Available ports changed";
    QString currect = ui->comboBoxCom->currentText();
    ui->comboBoxCom->clear();
    portList = newPorts;
    foreach (QSerialPortInfo port, portList)
      ui->comboBoxCom->addItem(port.portName() + " - " + port.description());
    int i = ui->comboBoxCom->findText(currect);
    if (i == -1) {
      if (ui->pushButtonDisconnect->isEnabled()) {
        on_pushButtonDisconnect_clicked();
        ui->comboBoxBaud->setCurrentIndex(0);
      }
    } else
      ui->comboBoxCom->setCurrentIndex(i);
  }
}

void MainWindow::on_pushButtonConnect_clicked() { emit connectSerial(portList.at(ui->comboBoxCom->currentIndex()).portName(), ui->comboBoxBaud->currentText().toInt()); }

void MainWindow::on_tabs_right_currentChanged(int index) {
  if (index == 2)
    ui->lineEditCommand->setFocus();
}

void MainWindow::on_dialRollingRange_valueChanged(int value) { ui->doubleSpinBoxRangeHorizontal->setValue(logaritmicSettings[value]); }

void MainWindow::on_dialVerticalRange_valueChanged(int value) { ui->doubleSpinBoxRangeVerticalRange->setValue(logaritmicSettings[value]); }

void MainWindow::on_pushButtonClearChannels_clicked() { ui->plot->resetChannels(); }

void MainWindow::on_pushButtonChannelColor_clicked() {
  QColor color = QColorDialog::getColor(ui->plot->getChColor(ui->spinBoxChannelSelect->value()));
  if (!color.isValid())
    return;
  ui->plot->setChColor(ui->spinBoxChannelSelect->value(), color);
  QPixmap pixmap(30, 30);
  pixmap.fill(color);
  ui->pushButtonChannelColor->setIcon(pixmap);
}

void MainWindow::on_spinBoxChannelSelect_valueChanged(int arg1) {
  ui->comboBoxGraphStyle->setCurrentIndex(ui->plot->getChStyle(arg1));
  QPixmap pixmap(30, 30);
  pixmap.fill(ui->plot->getChColor(arg1));
  ui->pushButtonChannelColor->setIcon(pixmap);
  double offset = ui->plot->getChOffset(arg1);
  double scale = ui->plot->getChScale(arg1);
  ui->doubleSpinBoxChOffset->setValue(offset);
  if (offset < ui->doubleSpinBoxRangeVerticalRange->value() / 2)
    ui->dialOffset->setValue(offset / ui->doubleSpinBoxRangeVerticalRange->value() * 100 * 2);
  ui->doubleSpinBoxChScale->setValue(scale);
  if (isStandardValue(scale))
    ui->dialChScale->setValue(roundToStandardValue(scale));
  updateChScale();
}

void MainWindow::on_doubleSpinBoxChOffset_valueChanged(double arg1) {
  ui->plot->changeChOffset(ui->spinBoxChannelSelect->value(), arg1);
  scrollBarCursor_valueChanged();
}

void MainWindow::on_dialOffset_valueChanged(int value) { ui->doubleSpinBoxChOffset->setValue(ui->doubleSpinBoxRangeVerticalRange->value() / 2 * value * 0.01); }

void MainWindow::on_comboBoxGraphStyle_currentIndexChanged(int index) {
  ui->plot->setChStyle(ui->spinBoxChannelSelect->value(), index);
  ui->plot->updateVisuals();
}

void MainWindow::scrollBarCursor_valueChanged() {
  double x1 = ui->horizontalScrollBarCursorX1->value() / 1000.0;
  double x2 = ui->horizontalScrollBarCursorX2->value() / 1000.0;
  double y1 = ui->verticalScrollBarCursorY1->value() / 1000.0;
  double y2 = ui->verticalScrollBarCursorY2->value() / 1000.0;

  ui->labelCursorX1->setText("X1: " + QString::number(x1, 'f', 3));
  ui->labelCursorX2->setText("X2: " + QString::number(x2, 'f', 3));
  ui->labelCursorY1->setText("Y1: " + QString::number(y1 - ui->plot->getChOffset(ui->spinBoxChannelSelect->value()) / ui->plot->getChScale(ui->spinBoxChannelSelect->value()), 'f', 3));
  ui->labelCursorY2->setText("Y2: " + QString::number(y2 - ui->plot->getChOffset(ui->spinBoxChannelSelect->value()) / ui->plot->getChScale(ui->spinBoxChannelSelect->value()), 'f', 3));
  ui->labelCursordX->setText(tr("dX: ") + QString::number(abs(x2 - x1)));
  ui->labelCursordY->setText(tr("dY: ") + QString::number(abs(y2 - y1) / ui->plot->getChScale(ui->spinBoxChannelSelect->value())));
  ui->plot->updateCursors(x1, x2, y1, y2);
}

void MainWindow::on_pushButtonDisconnect_clicked() {
  emit disconnectSerial();
  ui->lineEditPortInfo->setText(tr("Disconnecting..."));
}

void MainWindow::on_pushButtonSendCommand_clicked() {
  QString text = ui->lineEditCommand->text() + lineEndings[ui->comboBoxLineEnding->currentIndex()];
  emit writeToSerial(text.toUtf8());
}

void MainWindow::addDataToPlot(QVector<Channel *> channels) { ui->plot->newData(channels); }

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonDisconnect->setEnabled(connected);
  ui->pushButtonConnect->setEnabled(!connected);
  ui->comboBoxCom->setEnabled(!connected);
  ui->comboBoxBaud->setEnabled(!connected);
  ui->lineEditPortInfo->setText(message);
}

void MainWindow::printToTerminal(QByteArray data) { ui->myTerminal->printToTerminal(data); }

void MainWindow::serialFinishedWriting() { ui->lineEditCommand->clear(); }

void MainWindow::on_spinBoxDataBinaryBits_valueChanged(int arg1) {
  if (arg1 != 64)
    ui->doubleSpinBoxBinarydataMax->setPrefix("Max (0x" + QString::number(((quint64)1 << arg1) - 1, 16).toUpper() + "): ");
  else
    ui->doubleSpinBoxBinarydataMax->setPrefix("Max (0xFFFFFFFFFFFFFFFF): ");
  emit setBinBits(arg1);
}

void MainWindow::on_spinBoxBinaryDataNumCh_valueChanged(int arg1) {
  ui->spinBoxBinaryDataFirstCh->setMaximum(65 - arg1);
  emit setBinNCh(arg1);
}

void MainWindow::on_doubleSpinBoxChScale_valueChanged(double arg1) {
  ui->plot->changeChScale(ui->spinBoxChannelSelect->value(), arg1);
  scrollBarCursor_valueChanged();
  ui->checkBoxChInvert->setChecked(arg1 < 0);
  updateChScale();
}

void MainWindow::on_dialChScale_valueChanged(int value) {
  if (isStandardValue(abs(ui->doubleSpinBoxChScale->value())))
    ui->doubleSpinBoxChScale->setValue(logaritmicSettings[value] * (ui->checkBoxChInvert->isChecked() ? -1 : 1));
  else {
    ui->dialChScale->setValue(roundToStandardValue(abs(ui->doubleSpinBoxChScale->value())));
    ui->doubleSpinBoxChScale->setValue(logaritmicSettings[ui->dialChScale->value()] * (ui->checkBoxChInvert->isChecked() ? -1 : 1));
  }
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
  ui->plot->setZoomRange(value);
  ui->horizontalScrollBarHorizontal->setMinimum(value / 2);
  ui->horizontalScrollBarHorizontal->setMaximum(1000 - value / 2);
  ui->horizontalScrollBarHorizontal->setPageStep(value);
}

void MainWindow::on_comboBoxPlotRangeType_currentIndexChanged(int index) { ui->plot->setRangeType(index); }

void MainWindow::on_listWidgetDataMode_currentRowChanged(int currentRow) { emit setMode(currentRow); }

void MainWindow::on_radioButtonEn_toggled(bool checked) {
  if (checked)
    changeLanguage("en");
}

void MainWindow::on_radioButtonCz_toggled(bool checked) {
  if (checked)
    changeLanguage("cz");
}

void MainWindow::on_lineEditCommand_returnPressed() { on_pushButtonSendCommand_clicked(); }

void MainWindow::updateDivs(double vertical, double horizontal) {
  ui->plot->setVerticalDiv(logaritmicSettings[roundToStandardValue(vertical) + ui->dialVerticalDiv->value()]);
  ui->plot->setHorizontalDiv(logaritmicSettings[roundToStandardValue(horizontal) + ui->dialhorizontalDiv->value()]);
  updateChScale();
  ui->labelHDiv->setText(QString::number(ui->plot->getHDiv()) + tr(" / Div"));
  ui->labelVDiv->setText(QString::number(ui->plot->getVDiv()) + tr(" / Div"));
}

void MainWindow::on_checkBoxChInvert_toggled(bool checked) {
  if (checked == false && ui->doubleSpinBoxChScale->value() < 0)
    ui->doubleSpinBoxChScale->setValue(-ui->doubleSpinBoxChScale->value());
  else if (checked == true && ui->doubleSpinBoxChScale->value() > 0)
    ui->doubleSpinBoxChScale->setValue(-ui->doubleSpinBoxChScale->value());
}
