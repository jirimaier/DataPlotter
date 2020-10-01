#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore>
#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init() {
  connectSignals();
  changeLanguage();

  ui->tabs_right->setCurrentIndex(0);
  ui->tabs_Plot->setCurrentIndex(0);
  ui->checkBoxModeManual->setChecked(false);
  ui->comboBoxPlotRangeType->setCurrentIndex(PLOT_RANGE_FIXED);
  on_pushButtonDataModeApply_clicked();
  ui->labelBuildDate->setText("Build: " + QString(__DATE__) + " " + QString(__TIME__));

  if (locale().toString(1.1).contains(','))
    ui->radioButtonCSVComma->setChecked(true);

  QPixmap pixmap(30, 30);
  pixmap.fill(defaultColors[0]);
  ui->pushButtonChannelColor->setIcon(pixmap);

  ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/run.png"));

  updateChScale();

  portsRefreshTimer.setInterval(500);
  plotUpdateTimer.setInterval(10);
  listUpdateTimer.setInterval(100);
  connect(&plotUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePlot);
  connect(&listUpdateTimer, &QTimer::timeout, this, &MainWindow::updateInfo);
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
  connect(ui->pushButtonPrintBuffer, &QPushButton::clicked, this, &MainWindow::requestBufferDebug);
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

void MainWindow::exportCSV(bool all, int ch) {
  QString fileName = QFileDialog::getSaveFileName(this, tr("Export Channel %1").arg(ch), QString(QCoreApplication::applicationDirPath()), tr("Comma separated values (*.csv)"));
  if (fileName.isEmpty())
    return;
  QFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Truncate)) {
    char decimal = ui->radioButtonCSVDot->isChecked() ? '.' : ',';
    char separator = ui->radioButtonCSVDot->isChecked() ? ',' : ';';
    if (all)
      file.write(ui->plot->exportAllCSV(separator, decimal, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVoffsets->isChecked()));
    else
      file.write(ui->plot->exportChannelCSV(separator, decimal, ch, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVoffsets->isChecked()));
    file.close();
  } else {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant write to file."));
    msgBox.setInformativeText(tr("This may be because file is opened in another program."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
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
  ui->labelBinSettings->setVisible(ui->checkBoxModeManual->isChecked() || mode == DATA_MODE_DATA_BINARY);
  ui->labelDataMode->setText(tr("Data mode: ") + ui->comboBoxDataMode->itemText(mode));
  ui->comboBoxDataMode->setCurrentIndex(mode);
}

void MainWindow::showProcessedCommand(QString message) { receivedListBuffer.append(message); }

void MainWindow::updateInfo() {
  if (receivedListBuffer.isEmpty())
    return;
  foreach (QString line, receivedListBuffer)
    ui->textEditSerialDebug->append(line);
  receivedListBuffer.clear();

  QString text = tr("Binary mode settings:");
  text.append(QString::number(binSettings.bits) + tr(" bits") + "\n");
  if (binSettings.bits != 64)
    text.append("Max (0x" + QString::number(((quint64)1 << binSettings.bits) - 1, 16).toUpper() + "): " + QString::number(binSettings.valueMax) + "\n");
  else
    text.append("Max (0xFFFFFFFFFFFFFFFF): " + QString::number(binSettings.valueMax) + "\n");
  text.append("Min (0x00): " + QString::number(binSettings.valueMin) + "\n");
  text.append(tr("Time step: ") + QString::number(binSettings.timeStep) + tr(" / sample") + "\n");
  if (binSettings.numCh == 1)
    text.append(tr("Channel ") + QString::number(binSettings.firstCh) + "\n");
  else
    text.append(tr("Channels ") + QString::number(binSettings.firstCh) + tr(" - ") + QString::number(binSettings.firstCh + binSettings.numCh - 1) + "\n");
  if (binSettings.continuous)
    text.append(tr("continous") + "\n");
  ui->labelBinSettings->setText(text.trimmed());
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
  // Zjistí, jestli nastala změna v portech.
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

  // Aktualizuje seznam portů
  if (change) {
    qDebug() << "Available ports changed";
    QString current = ui->comboBoxCom->currentText();
    ui->comboBoxCom->clear();
    portList = newPorts;
    int portWithStName = -1;

    // Nandá nové porty do comboboxu, pokusí se najít port jehož popis poukazuje na Nucleo
    for (int i = 0; i < portList.length(); i++) {
      QSerialPortInfo port = portList.at(i);
      ui->comboBoxCom->addItem(port.portName() + " - " + port.description());
      if (port.description().contains(PORT_NUCLEO_DESCRIPTION_IDENTIFIER))
        portWithStName = i;
    }

    // Znovu vypere původní port; pokud neexistuje, vybere port který je asi Nucleo, pokud žádný popisem neodpovídá, vybere ten první.
    ui->comboBoxCom->setCurrentIndex(ui->comboBoxCom->findText(current) == -1 ? MAX(portWithStName, 0) : ui->comboBoxCom->findText(current));
  }
}

void MainWindow::on_pushButtonConnect_clicked() { emit connectSerial(portList.at(ui->comboBoxCom->currentIndex()).portName(), ui->comboBoxBaud->currentText().toInt()); }

void MainWindow::on_tabs_right_currentChanged(int index) {
  if (index == 2)
    ui->lineEditCommand->setFocus();
}

void MainWindow::on_dialRollingRange_valueChanged(int value) { ui->doubleSpinBoxRangeHorizontal->setValue(logaritmicSettings[value]); }

void MainWindow::on_dialVerticalRange_valueChanged(int value) { ui->doubleSpinBoxRangeVerticalRange->setValue(logaritmicSettings[value]); }

void MainWindow::on_pushButtonClearChannels_clicked() {
  ui->plot->resetChannels();
  emit resetChannels();
}

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
  ui->labelCursordX->setText(tr("dX: ") + QString::number(fabs(x2 - x1)));
  ui->labelCursordY->setText(tr("dY: ") + QString::number(fabs(y2 - y1) / ui->plot->getChScale(ui->spinBoxChannelSelect->value())));
  ui->plot->updateCursors(x1, x2, y1, y2);
}

void MainWindow::on_pushButtonDisconnect_clicked() {
  emit disconnectSerial();
  ui->labelPortInfo->setText(tr("Disconnecting..."));
}

void MainWindow::on_pushButtonSendCommand_clicked() {
  QString text = ui->lineEditCommand->text() + lineEndings[ui->comboBoxLineEnding->currentIndex()];
  emit writeToSerial(text.toUtf8());
}

void MainWindow::addDataToPlot(int ch, QVector<double> *time, QVector<double> *value, bool continous, bool sorted) { ui->plot->newData(ch, time, value, continous, sorted); }

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonDisconnect->setEnabled(connected);
  ui->pushButtonConnect->setEnabled(!connected);
  ui->comboBoxCom->setEnabled(!connected);
  ui->comboBoxBaud->setEnabled(!connected);
  ui->labelPortInfo->setText(message);
  ui->pushButtonSendCommand->setEnabled(connected);
}

void MainWindow::printToTerminal(QByteArray data) { ui->myTerminal->printToTerminal(data); }

void MainWindow::serialFinishedWriting() { ui->lineEditCommand->clear(); }

void MainWindow::bufferDebug(QByteArray data) {
  QString stringData = QString(data);
  if (stringData.length() == data.length()) {
    ui->textEditSerialDebug->append(QString("<font color=red>%1</font color>").arg(tr("Buffer content (Text): ")));
    ui->textEditSerialDebug->append(stringData.simplified());
  }
  ui->textEditSerialDebug->append(QString("<font color=red>%1</font color> %2 %3").arg(tr("Buffer content (Hex):")).arg(data.length()).arg(tr("bytes")));

  // Oddělení bajtů mezerami nefunguje v starším Qt (Win XP)
#if QT_VERSION >= 0x050900
  ui->textEditSerialDebug->append(QString("<font color=navy>%1</font color>").arg(QString(data.toHex(' '))));
#else
  QString data2;
  foreach (byte b, data)
    data2.append(QString::number(b, 16) + " ");
  data2 = data2.trimmed();
  ui->textEditSerialDebug->append(QString("<font color=red>%1</font color>").arg(data2.trimmed()));
#endif
  ui->textEditSerialDebug->append("");
}

void MainWindow::on_doubleSpinBoxChScale_valueChanged(double arg1) {
  ui->plot->changeChScale(ui->spinBoxChannelSelect->value(), arg1);
  scrollBarCursor_valueChanged();
  ui->checkBoxChInvert->setChecked(arg1 < 0);
  updateChScale();
}

void MainWindow::on_dialChScale_valueChanged(int value) {
  if (isStandardValue(fabs(ui->doubleSpinBoxChScale->value())))
    ui->doubleSpinBoxChScale->setValue(logaritmicSettings[value] * (ui->checkBoxChInvert->isChecked() ? -1 : 1));
  else {
    ui->dialChScale->setValue(roundToStandardValue(fabs(ui->doubleSpinBoxChScale->value())));
    ui->doubleSpinBoxChScale->setValue(logaritmicSettings[ui->dialChScale->value()] * (ui->checkBoxChInvert->isChecked() ? -1 : 1));
  }
}

void MainWindow::on_pushButtonSelectedCSV_clicked() { exportCSV(false, ui->spinBoxChannelSelect->value() - 1); }

void MainWindow::on_dialZoom_valueChanged(int value) {
  ui->plot->setZoomRange(value);
  ui->horizontalScrollBarHorizontal->setMinimum(value / 2);
  ui->horizontalScrollBarHorizontal->setMaximum(1000 - value / 2);
  ui->horizontalScrollBarHorizontal->setPageStep(value);
}

void MainWindow::on_comboBoxPlotRangeType_currentIndexChanged(int index) { ui->plot->setRangeType(index); }

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
  ui->plot->setVerticalDiv(logaritmicSettings[MAX(roundToStandardValue(vertical) + ui->dialVerticalDiv->value(), 0)]);
  ui->plot->setHorizontalDiv(logaritmicSettings[MAX(roundToStandardValue(horizontal) + ui->dialhorizontalDiv->value(), 0)]);
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

void MainWindow::on_horizontalSliderLineTimeout_valueChanged(int value) {
  ui->labelLineTimeout->setText(QString::number(logaritmicSettings[value]) + " ms");
  emit changeLineTimeout(logaritmicSettings[value]);
}

void MainWindow::on_pushButtonPrintBuffer_clicked() {}

void MainWindow::on_pushButtonDataModeApply_clicked() {
  emit setMode(ui->comboBoxDataMode->currentIndex());
  BinDataSettings_t settings;
  settings.bits = ui->spinBoxDataBinaryBits->value();
  settings.continuous = ui->checkBoxBinContinuous->isChecked();
  settings.firstCh = ui->spinBoxBinaryDataFirstCh->value();
  settings.numCh = ui->spinBoxBinaryDataNumCh->value();
  settings.timeStep = ui->doubleSpinBoxBinaryTimestep->value();
  settings.valueMax = ui->doubleSpinBoxBinarydataMax->value();
  settings.valueMin = ui->doubleSpinBoxBinaryDataMin->value();
  emit setBinParameters(settings);
}

void MainWindow::on_checkBoxModeManual_toggled(bool checked) {
  emit allowModeChange(!checked);
  if (checked) {
    ui->spinBoxDataBinaryBits->setValue(binSettings.bits);
    ui->doubleSpinBoxBinaryDataMin->setValue(binSettings.valueMin);
    ui->doubleSpinBoxBinarydataMax->setValue(binSettings.valueMax);
    ui->doubleSpinBoxBinaryTimestep->setValue(binSettings.timeStep);
    ui->spinBoxBinaryDataNumCh->setValue(binSettings.numCh);
    ui->spinBoxBinaryDataFirstCh->setValue(binSettings.firstCh);
    ui->checkBoxBinContinuous->setChecked(binSettings.continuous);
  }
  if (checked)
    ui->labelBinSettings->setVisible(true);
}

void MainWindow::updatePlot() {
  ui->plot->update();
  if (ui->checkBoxMath1->isChecked()) {
    int firstch = ui->spinBoxMath1First->value();
    int secondch = ui->spinBoxMath1Second->value();
    int resultch = ui->spinBoxMath1Result->value();
    int operation = ui->comboBoxMath1Op->currentIndex();
    emit requestMath(resultch, operation, ui->plot->getDataVector(firstch - 1, false), ui->plot->getDataVector(secondch - 1, false));
  }
  if (ui->checkBoxMath2->isChecked()) {
    int firstch = ui->spinBoxMath2First->value();
    int secondch = ui->spinBoxMath2Second->value();
    int resultch = ui->spinBoxMath2Result->value();
    int operation = ui->comboBoxMath2Op->currentIndex();
    emit requestMath(resultch, operation, ui->plot->getDataVector(firstch - 1, false), ui->plot->getDataVector(secondch - 1, false));
  }
  if (ui->checkBoxMath3->isChecked()) {
    int firstch = ui->spinBoxMath3First->value();
    int secondch = ui->spinBoxMath3Second->value();
    int resultch = ui->spinBoxMath3Result->value();
    int operation = ui->comboBoxMath3Op->currentIndex();
    emit requestMath(resultch, operation, ui->plot->getDataVector(firstch - 1, false), ui->plot->getDataVector(secondch - 1, false));
  }
  if (ui->checkBoxMath4->isChecked()) {
    int firstch = ui->spinBoxMath4First->value();
    int secondch = ui->spinBoxMath4Second->value();
    int resultch = ui->spinBoxMath4Result->value();
    int operation = ui->comboBoxMath4Op->currentIndex();
    emit requestMath(resultch, operation, ui->plot->getDataVector(firstch - 1, false), ui->plot->getDataVector(secondch - 1, false));
  }
}

void MainWindow::on_comboBoxOutputLevel_currentIndexChanged(int index) { emit setOutputLevel(index); }

void MainWindow::on_pushButtonAllCSV_clicked() { exportCSV(true); }
