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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init(QTranslator *translator, const PlotData *plotData, const PlotMath *plotMath, const SerialReader *serialReader) {
  // Načte ikony které se mění za běhu
  iconXY = QIcon(":/images/icons/xyChannel.png");
  iconRun = QIcon(":/images/icons/run.png");
  iconPause = QIcon(":/images/icons/pause.png");
  iconHidden = QIcon(":/images/icons/hidden.png");
  iconVisible = QIcon(":/images/icons/visible.png");
  iconConnected = QIcon(":/images/icons/connected.png");
  iconNotConnected = QIcon(":/images/icons/disconnected.png");
  iconCross = QIcon(":/images/icons/cross.png");
  iconFFT = QIcon(":/images/icons/fft.png");

  fillChannelSelect(); // Vytvoří seznam kanálů pro výběr

  QObject::connect(plotMath, &PlotMath::sendResult, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotData, &PlotData::clearXY, ui->plotxy, &MyXYPlot::clear);
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

void MainWindow::showPlotStatus(PlotStatus::enumerator type) {
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
    ui->labelChScale->setText(GlobalFunctions::floatToNiceString(perDiv, 3, true, false) + tr("V / Div"));
  } else
    ui->labelChScale->setText(tr("---"));
}

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonConnect->setIcon(connected ? iconConnected : iconNotConnected);
  ui->labelPortInfo->setText(message);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
    ui->plotxy->clear();
    ui->plotFFT->clear();
    ui->myTerminal->resetTerminal();
    emit resetChannels();
  }
}

void MainWindow::serialFinishedWriting() {
  if (!ui->pushButtonMultiplInputs->isChecked())
    ui->lineEditCommand->clear();
}

void MainWindow::updateDivs() {
  updateChScale();
  if (ui->labelHDiv->isEnabled())
    ui->labelHDiv->setText(GlobalFunctions::floatToNiceString(ui->plot->getHDiv(), 3, false, false, true) + tr("s/Div"));
  else
    ui->labelHDiv->setText("---");
  ui->labelVDiv->setText(GlobalFunctions::floatToNiceString(ui->plot->getVDiv(), 3, false, false, true) + tr("V/Div"));
}

void MainWindow::on_pushButtonCenter_clicked() {
  if (ui->dialVerticalCenter->value() != 0)
    ui->dialVerticalCenter->setValue(0);
  else
    ui->plot->setVerticalCenter(0);
}

void MainWindow::printMessage(QString messageHeader, QByteArray messageBody, int type, MessageTarget::enumerator target) {
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
    ui->plainTextEditConsole->appendHtml(color + QString(messageHeader) + "</font color>: " + stringMessage);
  else
    ui->plainTextEditConsole_2->appendHtml(color + QString(messageHeader) + "</font color>: " + stringMessage);
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
  QScrollBar *scroll = ui->plainTextEditConsole->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
  pendingDeviceMessage = !ended;
}

void MainWindow::updateMathNow(int number) {
  emit setMathFirst(number, mathEn[number - 1]->isChecked() ? mathFirst[number - 1]->value() : 0);
  emit setMathSecond(number, mathEn[number - 1]->isChecked() ? mathSecond[number - 1]->value() : 0);
  emit clearMath(number);
  ui->plot->clearCh(GlobalFunctions::getAnalogChId(number, ChannelType::math));
  if (mathEn[number - 1]->isChecked()) {
    MathOperations::enumerator operation = (MathOperations::enumerator)mathOp[number - 1]->currentIndex();
    QSharedPointer<QCPGraphDataContainer> in1 = ui->plot->graph(GlobalFunctions::getAnalogChId(mathFirst[number - 1]->value(), ChannelType::analog))->data();
    QSharedPointer<QCPGraphDataContainer> in2 = ui->plot->graph(GlobalFunctions::getAnalogChId(mathSecond[number - 1]->value(), ChannelType::analog))->data();
    emit resetMath(number, operation, in1, in2);
  }
}

void MainWindow::on_pushButtonChangeFFTColor_clicked() {
  QColor color = QColorDialog::getColor(ui->plotFFT->graph(0)->pen().color());
  if (color.isValid()) {
    ui->plotFFT->graph(0)->setPen(QColor(color));
    if (ui->comboBoxFFTStyle->currentIndex() == GraphStyle::filled)
      ui->plotFFT->graph(0)->setBrush(color);
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
  if (index != FFTType::spectrum)
    ui->plotFFT->setYUnit("dB");
  else
    ui->plotFFT->setYUnit("");
}

void MainWindow::on_lineEditVUnit_textEdited(const QString &arg1) {
  ui->plot->setYUnit(arg1);
  ui->plotxy->setYUnit(arg1);
  ui->plotxy->setXUnit(arg1);
}

void MainWindow::on_checkBoxOpenGL_toggled(bool checked) { ui->plot->setOpenGl(checked); }

void MainWindow::on_checkBoxMouseControls_toggled(bool checked) {
  ui->plot->enableMouseCursorControll(checked);
  ui->plotxy->enableMouseCursorControll(checked);
  ui->plotFFT->enableMouseCursorControll(checked);
}
