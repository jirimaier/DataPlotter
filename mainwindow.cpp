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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init(QTranslator* translator, const PlotData* plotData, const PlotMath* plotMath, const SerialReader* serialReader) {
  // Načte ikony které se mění za běhu
  iconRun = QIcon(":/images/icons/run.png");
  iconPause = QIcon(":/images/icons/pause.png");
  iconHidden = QIcon(":/images/icons/hidden.png");
  iconVisible = QIcon(":/images/icons/visible.png");
  iconConnected = QIcon(":/images/icons/connected.png");
  iconNotConnected = QIcon(":/images/icons/disconnected.png");
  iconCross = QIcon(":/images/icons/cross.png");

  fillChannelSelect(); // Vytvoří seznam kanálů pro výběr

  QObject::connect(plotMath, &PlotMath::sendResult, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotData, &PlotData::addVectorToPlot, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotData, &PlotData::addPointToPlot, ui->plot, &MyMainPlot::newDataPoint);
  QObject::connect(plotData, &PlotData::clearLogic, ui->plot, &MyMainPlot::clearLogicGroup);
  QObject::connect(ui->myTerminal, &MyTerminal::writeToSerial, serialReader, &SerialReader::write);

  this->translator = translator;
  setGuiArrays();
  initSetables();
  connectSignals();
  changeLanguage();
  setGuiDefaults();
  updateChScale();
  setAdaptiveSpinBoxes();
  setUp();
  startTimers();
}

void MainWindow::changeLanguage(QString code) {
  if (!translator->load(QString(":/translations/translations/translation_%1.qm").arg(code))) {
    qDebug() << "Can not load " << QString(":/translations/translations/translation_%1.qm").arg(code);
    return;
  }
  qApp->installTranslator(translator);
  ui->retranslateUi(this);
}

void MainWindow::showPlotStatus(PlotStatus::enumPlotStatus type) {
  if (type == PlotStatus::pause) {
    ui->pushButtonPause->setIcon(iconPause);
    ui->pushButtonPause->setToolTip(tr("Paused (click to resume)"));
  } else if (type == PlotStatus::run) {
    ui->pushButtonPause->setIcon(iconRun);
    ui->pushButtonPause->setToolTip(tr("Running (click to pause)"));
  }
}

void MainWindow::updateChScale() {
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT) {
    double perDiv = ui->plot->getCHDiv(ui->comboBoxSelectedChannel->currentIndex());
    ui->labelChScale->setText(floatToNiceString(perDiv, 3, true, false) + ui->plot->getYUnit() + tr(" / Div"));
  } else
    ui->labelChScale->setText("---");
}

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonConnect->setIcon(connected ? iconConnected : iconNotConnected);
  ui->labelPortInfo->setText(message);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
    ui->plotxy->clear();
    ui->plotFFT->clear(0);
    ui->plotFFT->clear(1);
    ui->myTerminal->resetTerminal();
    ui->plainTextEditConsole->clear();
    ui->plainTextEditConsole_3->clear();
    emit resetChannels();
  }
  if (connected && ui->checkBoxResetCmdEn->isChecked() && (!ui->lineEditResetCmd->text().isEmpty())) {
    // Poslat reset příkaz
    QByteArray data = ui->textEditTerminalDebug->toPlainText().toLocal8Bit();
    data.replace("\\n", "\n");
    data.replace("\\e", "\u001b");
    data.replace("\\r", "\r");
    data.replace("\\t", "\t");
    data.replace("\\b", "\b");
    data.replace("\\a", "\a");
    emit writeToSerial(ui->lineEditResetCmd->text().toLocal8Bit());
  }
  autoAutosetPending = ui->checkBoxAutoAutoSet->isChecked();
}

void MainWindow::serialFinishedWriting() {
  if (!ui->pushButtonMultiplInputs->isChecked())
    ui->lineEditCommand->clear();
}

void MainWindow::updateDivs() {
  updateChScale();
  if (ui->labelHDiv->isEnabled())
    ui->labelHDiv->setText(floatToNiceString(ui->plot->getHDiv(), 1, false, false) + tr("s/Div"));
  else
    ui->labelHDiv->setText("---");
  ui->labelVDiv->setText(floatToNiceString(ui->plot->getVDiv(), 1, false, false) + ui->plot->getYUnit() + tr("/Div"));
}

void MainWindow::on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1) {
  ui->doubleSpinBoxChOffset->setSingleStep(pow(10, floor(log10(arg1)) - 2));
  ui->doubleSpinBoxYCur1->setSingleStep(pow(10, floor(log10(arg1)) - 2));
  ui->doubleSpinBoxYCur2->setSingleStep(pow(10, floor(log10(arg1)) - 2));
}

void MainWindow::on_pushButtonCenter_clicked() {
  if (ui->sliderVerticalCenter->value() != 0)
    ui->sliderVerticalCenter->setValue(0);
  else
    ui->plot->setVerticalCenter(0);
}

void MainWindow::on_pushButtonClearAll_clicked() {
  ui->plot->resetChannels();
  // emit resetChannels();
}

void MainWindow::on_pushButtonTerminalInputCopy_clicked() {
  QByteArray bytes = ui->textEditTerminalDebug->toPlainText().replace('\n', "\\r\\n").toUtf8();
  for (unsigned char ch = 0;; ch++) {
    if (ch == 32)
      ch = 127;
    bytes.replace(ch, ("\\x" + QString::number(ch, 16)).toLocal8Bit() + "\"\"");
    if (ch == 255)
      break;
  }

  QGuiApplication::clipboard()->setText(bytes);
}

void MainWindow::printMessage(QString messageHeader, QByteArray messageBody, int type, MessageTarget::enumMessageTarget target) {
  QString color = "<font color=black>";
  switch (type) {
    case MessageLevel::warning:
      color = "<font color=orange>";
      break;
    case MessageLevel::error:
      color = "<font color=red>";
      break;
    case MessageLevel::info:
      color = "<font color=green>";
      break;
    default:
      color = "<font color=black>";
  }

  QString stringMessage;
  stringMessage = messageBody;
  if (target == MessageTarget::serial1)
    ui->plainTextEditConsole->appendHtml(color + QString(messageHeader) + "</font color>" + (stringMessage.isEmpty() ? "" : ": ") + stringMessage);
  else
    ui->plainTextEditConsole_2->appendHtml(color + QString(messageHeader) + "</font color>" + (stringMessage.isEmpty() ? "" : ": ") + stringMessage);
}

void MainWindow::printDeviceMessage(QByteArray message, bool warning, bool ended) {
  if (message.contains('\a')) {
    QApplication::beep(); // Bell character
    message.replace('\a', "");
  }

  if (!pendingDeviceMessage && !message.isEmpty()) {
    if (warning)
      ui->plainTextEditConsole->appendHtml(tr("<font color=darkred>Device warning:</font color> "));
    else
      ui->plainTextEditConsole->appendHtml(tr("<font color=darkgreen>Device message:</font color> "));
  }
  ui->plainTextEditConsole->moveCursor(QTextCursor::End);
  ui->plainTextEditConsole->insertPlainText(message);
  ui->plainTextEditConsole->moveCursor(QTextCursor::End);
  QScrollBar* scroll = ui->plainTextEditConsole->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
  pendingDeviceMessage = !ended;
}

void MainWindow::ch1WasUpdated(bool wasPoint, bool wasLogic, HAxisType::enumHAxisType recommandedTimeBase) {
  lastUpdateWasPoint = wasPoint;
  lastUpdateWasLogic = wasLogic;
  dataUpdates++;
  recommandedAxisType = recommandedTimeBase;
}

void MainWindow::updateMathNow(int number) {
  emit setMathFirst(number, mathEn[number - 1]->isChecked() ? mathFirst[number - 1]->value() : 0);
  emit setMathSecond(number, mathEn[number - 1]->isChecked() ? mathSecond[number - 1]->value() : 0);
  emit clearMath(number);
  ui->plot->clearCh(getAnalogChId(number, ChannelType::math));
  if (mathEn[number - 1]->isChecked()) {
    MathOperations::enumMathOperations operation = (MathOperations::enumMathOperations)mathOp[number - 1]->currentIndex();
    QSharedPointer<QCPGraphDataContainer> in1 = ui->plot->graph(getAnalogChId(mathFirst[number - 1]->value(), ChannelType::analog))->data();
    QSharedPointer<QCPGraphDataContainer> in2 = ui->plot->graph(getAnalogChId(mathSecond[number - 1]->value(), ChannelType::analog))->data();
    emit resetMath(number, operation, in1, in2);
  }
}

void MainWindow::on_pushButtonChangeXYColor_clicked() {
  QColor color = QColorDialog::getColor(ui->plotxy->graphXY->pen().color());
  if (color.isValid())
    ui->plotxy->graphXY->setPen(QColor(color));
}

void MainWindow::on_pushButtonTerminalDebugSend_clicked() {
  QByteArray data = ui->textEditTerminalDebug->toPlainText().toUtf8();
  data.replace("\n", "\r\n"); // Odřádkování v textovém poli

  data.replace("\\n", "\n");
  data.replace("\\e", "\u001b");
  data.replace("\\r", "\r");
  data.replace("\\t", "\t");
  data.replace("\\b", "\b");
  data.replace("\\a", "\a");

  ui->myTerminal->printToTerminal(data);
}

void MainWindow::on_textEditTerminalDebug_cursorPositionChanged() {
  if (ui->textEditTerminalDebug->textCursor().selectedText().isEmpty())
    ui->textEditTerminalDebug->setTextColor(Qt::black);
}

void MainWindow::on_myTerminal_cellClicked(int row, int column) {
  if (ui->pushButtonTerminalDebug->isChecked())
    insertInTerminalDebug(QString("\\e[%1;%2H").arg(row + 1).arg(column + 1), Qt::red);
}

void MainWindow::on_comboBoxFFTType_currentIndexChanged(int index) {
  if (index != FFTType::spectrum) {
    ui->plotFFT->setYUnit("dB");
    if (IS_FFT(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setSuffix("dB");
    if (IS_FFT(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setSuffix("dB");
  } else {
    ui->plotFFT->setYUnit("");
    if (IS_FFT(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setSuffix("");
    if (IS_FFT(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setSuffix("");
  } ui->spinBoxFFTSegments1->setVisible(index == FFTType::pwelch);
  ui->spinBoxFFTSegments2->setVisible(index == FFTType::pwelch);
}

void MainWindow::on_lineEditVUnit_textChanged(const QString& arg1) {
  ui->plot->setYUnit(arg1);
  ui->plotxy->setYUnit(arg1);
  ui->plotxy->setXUnit(arg1);
  ui->doubleSpinBoxRangeVerticalRange->setSuffix(arg1);
  ui->doubleSpinBoxChOffset->setSuffix(arg1);
  if (IS_ANALOG_OR_MATH(ui->comboBoxCursor1Channel->currentIndex()))
    ui->doubleSpinBoxYCur1->setSuffix(arg1);
  if (IS_ANALOG_OR_MATH(ui->comboBoxCursor1Channel->currentIndex()))
    ui->doubleSpinBoxYCur2->setSuffix(arg1);
  ui->doubleSpinBoxXYCurX1->setSuffix(arg1);
  ui->doubleSpinBoxXYCurX2->setSuffix(arg1);
  ui->doubleSpinBoxXYCurY1->setSuffix(arg1);
  ui->doubleSpinBoxXYCurY2->setSuffix(arg1);
}

void MainWindow::on_checkBoxOpenGL_toggled(bool checked) {
  ui->plot->setOpenGl(checked);
  ui->plot->redraw();
}

void MainWindow::on_checkBoxMouseControls_toggled(bool checked) {
  ui->plot->enableMouseCursorControll(checked);
  ui->plotxy->enableMouseCursorControll(checked);
  ui->plotFFT->enableMouseCursorControll(checked);
}

void MainWindow::on_checkBoxFFTCh1_toggled(bool checked) {
  auto* model = qobject_cast<QStandardItemModel*>(ui->comboBoxCursor1Channel->model());
  auto* item = model->item(FFTID(0));
  item->setEnabled(checked && ui->pushButtonFFT->isChecked());
  QListView* view = qobject_cast<QListView*>(ui->comboBoxCursor1Channel->view());
  view->setRowHidden(FFTID(0), !(checked && ui->pushButtonFFT->isChecked()));

  model = qobject_cast<QStandardItemModel*>(ui->comboBoxCursor2Channel->model());
  item = model->item(FFTID(0));
  item->setEnabled(checked && ui->pushButtonFFT->isChecked());
  view = qobject_cast<QListView*>(ui->comboBoxCursor2Channel->view());
  view->setRowHidden(FFTID(0), !(checked && ui->pushButtonFFT->isChecked()));
}

void MainWindow::on_checkBoxFFTCh2_toggled(bool checked) {
  auto* model = qobject_cast<QStandardItemModel*>(ui->comboBoxCursor1Channel->model());
  auto* item = model->item(FFTID(1));
  item->setEnabled(checked && ui->pushButtonFFT->isChecked());
  QListView* view = qobject_cast<QListView*>(ui->comboBoxCursor1Channel->view());
  view->setRowHidden(FFTID(1), !(checked && ui->pushButtonFFT->isChecked()));

  model = qobject_cast<QStandardItemModel*>(ui->comboBoxCursor2Channel->model());
  item = model->item(FFTID(1));
  item->setEnabled(checked && ui->pushButtonFFT->isChecked());
  view = qobject_cast<QListView*>(ui->comboBoxCursor2Channel->view());
  view->setRowHidden(FFTID(1), !(checked && ui->pushButtonFFT->isChecked()));
}

void MainWindow::on_pushButtonProtocolGuide_clicked() {
  QString helpFile = QCoreApplication::applicationDirPath() + "/Protocol.pdf";
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(helpFile))) {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant open file."));
    msgBox.setInformativeText(helpFile);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}
