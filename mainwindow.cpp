//  Copyright (C) 2020  Jiří Maier

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

void MainWindow::init(QTranslator *translator, const PlotData *plotData, const PlotMath *plotMath) {
  // Načte ikony které se mění za běhu
  iconXY = QIcon(":/images/icons/xyChannel.png");
  iconRun = QIcon(":/images/icons/run.png");
  iconPause = QIcon(":/images/icons/pause.png");
  iconHidden = QIcon(":/images/icons/hidden.png");
  iconVisible = QIcon(":/images/icons/visible.png");
  iconConnected = QIcon(":/images/icons/connected.png");
  iconNotConnected = QIcon(":/images/icons/disconnected.png");

  fillChannelSelect();  // Vytvoří seznam kanálů pro výběr

  QObject::connect(plotMath, &PlotMath::sendResult, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotMath, &PlotMath::sendResultXY, ui->plotxy, &MyXYPlot::newData);
  QObject::connect(plotData, &PlotData::clearXY, ui->plotxy, &MyXYPlot::clear);
  QObject::connect(plotData, &PlotData::addVectorToPlot, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotData, &PlotData::addPointToPlot, ui->plot, &MyMainPlot::newDataPoint);
  QObject::connect(plotData, &PlotData::clearLogic, ui->plot, &MyMainPlot::clearLogicGroup);

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
    ui->labelChScale->setText(QString::number(perDiv, 'f', 3) + tr(" / Div"));
  } else
    ui->labelChScale->setText(tr("---"));
}

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonConnect->setIcon(connected ? iconConnected : iconNotConnected);
  ui->labelPortInfo->setText(message);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
    ui->plotxy->clear();
    ui->myTerminal->resetTerminal();
    emit resetChannels();
  }
}

void MainWindow::serialFinishedWriting() {
  if (!ui->pushButtonMultiplInputs->isChecked()) ui->lineEditCommand->clear();
}

void MainWindow::updateDivs() {
  updateChScale();
  if (ui->labelHDiv->isEnabled())
    ui->labelHDiv->setText(QString::number(ui->plot->getHDiv()) + tr(" / Div"));
  else
    ui->labelHDiv->setText("---");
  ui->labelVDiv->setText(QString::number(ui->plot->getVDiv()) + tr(" / Div"));
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

  bool printAsHex = false;
  for (QByteArray::iterator it = messageBody.begin(); it != messageBody.end(); it++) {
    if (*it <= 31) {
      printAsHex = true;
      break;
    }
  }
  QString stringMessage;
  if (printAsHex) {
// Oddělení bajtů mezerami nefunguje v starším Qt (Win XP)
#if QT_VERSION >= 0x050900
    stringMessage = messageBody.toHex(' ');
#else
    stringMessage.clear();
    for (QByteArray::iterator it = messageBody.begin(); it != messageBody.end(); it++) stringMessage.append(QString::number((uint8_t)*it, 16).rightJustified(2, '0') + " ");
    stringMessage = stringMessage.trimmed();
#endif
    stringMessage = QString("<font color=navy>%1</font>").arg(stringMessage);
  } else {
    stringMessage = messageBody;
  }
  if (target == MessageTarget::serial1)
    ui->plainTextEditConsole->appendHtml(color + QString(messageHeader) + "</font color>: " + stringMessage);
  else
    ui->plainTextEditConsole_2->appendHtml(color + QString(messageHeader) + "</font color>: " + stringMessage);
}

void MainWindow::printDeviceMessage(QByteArray message, bool warning, bool ended) {
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
    MathOperations::enumetrator operation = (MathOperations::enumetrator)mathOp[number - 1]->currentIndex();
    QSharedPointer<QCPGraphDataContainer> in1 = ui->plot->graph(GlobalFunctions::getAnalogChId(mathFirst[number - 1]->value(), ChannelType::analog))->data();
    QSharedPointer<QCPGraphDataContainer> in2 = ui->plot->graph(GlobalFunctions::getAnalogChId(mathSecond[number - 1]->value(), ChannelType::analog))->data();
    emit resetMath(number, operation, in1, in2);
  }
}

void MainWindow::updateXYNow() {
  emit setXYFirst(ui->pushButtonXY->isChecked() ? ui->spinBoxXYFirst->value() : 0);
  emit setXYSecond(ui->pushButtonXY->isChecked() ? ui->spinBoxXYSecond->value() : 0);
  emit clearXY();
  if (ui->pushButtonXY->isChecked()) {
    QSharedPointer<QCPGraphDataContainer> in1 = ui->plot->graph(GlobalFunctions::getAnalogChId(ui->spinBoxXYFirst->value(), ChannelType::analog))->data();
    QSharedPointer<QCPGraphDataContainer> in2 = ui->plot->graph(GlobalFunctions::getAnalogChId(ui->spinBoxXYSecond->value(), ChannelType::analog))->data();
    emit resetXY(in1, in2);
  }
}
