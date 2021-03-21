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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow),   serialSettingsDialog(new SerialSettingsDialog) {
  ui->setupUi(this);
  ui->doubleSpinBoxRangeVerticalRange->trimDecimalZeroes = true;
  ui->doubleSpinBoxRangeVerticalRange->emptyDefaultValue = 1;
  ui->doubleSpinBoxRangeHorizontal->trimDecimalZeroes = true;
  ui->doubleSpinBoxRangeHorizontal->emptyDefaultValue = 1;
}

MainWindow::~MainWindow() { delete ui; delete serialSettingsDialog; }

void MainWindow::setComboboxItemVisible(QComboBox& comboBox, int index, bool visible) {
  auto* model = qobject_cast<QStandardItemModel*>(comboBox.model());
  auto* item = model->item(index);
  item->setEnabled(visible);
  QListView* view = qobject_cast<QListView*>(comboBox.view());
  view->setRowHidden(index, !visible);
}

void MainWindow::setChStyleSelection(GraphType::enumGraphType type) {
  setComboboxItemVisible(*ui->comboBoxGraphStyle, GraphStyle::line, type == GraphType::analog || type == GraphType::math);
  setComboboxItemVisible(*ui->comboBoxGraphStyle, GraphStyle::point, type == GraphType::analog || type == GraphType::math);
  setComboboxItemVisible(*ui->comboBoxGraphStyle, GraphStyle::linePoint, type == GraphType::analog || type == GraphType::math);
  setComboboxItemVisible(*ui->comboBoxGraphStyle, GraphStyle::logic, type == GraphType::logic);
  setComboboxItemVisible(*ui->comboBoxGraphStyle, GraphStyle::logicFilled, type == GraphType::logic);
  setComboboxItemVisible(*ui->comboBoxGraphStyle, GraphStyle::logicpoints, type == GraphType::logic);
  setComboboxItemVisible(*ui->comboBoxGraphStyle, GraphStyle::logicSquare, type == GraphType::logic);
  setComboboxItemVisible(*ui->comboBoxGraphStyle, GraphStyle::logicSquareFilled, type == GraphType::logic);

}

void MainWindow::init(QTranslator* translator, const PlotData* plotData, const PlotMath* plotMath, const SerialReader* serialReader, const Averager* avg) {
  // Načte ikony které se mění za běhu
  iconRun = QIcon(":/images/icons/run.png");
  iconPause = QIcon(":/images/icons/pause.png");
  iconCross = QIcon(":/images/icons/cross.png");
  iconHidden = QIcon(":/images/icons/hidden.png");
  iconVisible = QIcon(":/images/icons/visible.png");
  iconConnected = QIcon(":/images/icons/connected.png");
  iconNotConnected = QIcon(":/images/icons/disconnected.png");
  iconAbsoluteCursor = QIcon(":/images/icons/rangeTab.png");


  fillChannelSelect(); // Vytvoří seznam kanálů pro výběr

  QObject::connect(plotMath, &PlotMath::sendResult, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotData, &PlotData::addVectorToPlot, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotData, &PlotData::addPointToPlot, ui->plot, &MyMainPlot::newDataPoint);
  QObject::connect(plotData, &PlotData::clearLogic, ui->plot, &MyMainPlot::clearLogicGroup);
  QObject::connect(ui->myTerminal, &MyTerminal::writeToSerial, serialReader, &SerialReader::write);

  QObject::connect(avg, &Averager::addVectorToPlot, ui->plot, &MyMainPlot::newDataVector);

  // Odpojit port když se změní pokročilá nastavení
  QObject::connect(serialSettingsDialog, &SerialSettingsDialog::settingChanged, serialReader, &SerialReader::end);

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
  serialSettingsDialog->retranslate();
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

void MainWindow::serialConnectResult(bool connected, QString message, QString details) {
  ui->pushButtonConnect->setIcon(connected ? iconConnected : iconNotConnected);
  ui->labelPortInfo->setText(message);
  ui->labelPortInfo->setToolTip(details);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
    ui->plotxy->clear();
    ui->plotFFT->clear(0);
    ui->plotFFT->clear(1);
    ui->myTerminal->resetTerminal();
    ui->plainTextEditConsole->clear();
    ui->plainTextEditConsole_3->clear();
    emit resetChannels();
    emit resetAverager();
    pendingDeviceMessage = false;
  }
  if (connected && ui->checkBoxResetCmdEn->isChecked() && (!ui->lineEditResetCmd->text().isEmpty())) {
    // Poslat reset příkaz
    QByteArray data = ui->textEditTerminalDebug->toPlainText().toLocal8Bit();
    data.replace("\\n", "\n");
    data.replace("\\r", "\r");
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

void MainWindow::interpolationResult(int chID, QSharedPointer<QCPGraphDataContainer> dataOriginal, QSharedPointer<QCPGraphDataContainer> dataInterpolated, bool dataIsFromInterpolationBuffer) {
  ui->plot->newInterpolatedVector(chID, dataOriginal, dataInterpolated, dataIsFromInterpolationBuffer);
  interpolationsRunning--;
  if (interpolationsRunning == 0)
    interpolationTimer.start();
}

void MainWindow::deviceError(QByteArray message, MessageTarget::enumMessageTarget source) {
  if (source == MessageTarget::manual) {
    QMessageBox msgBox(this);
    msgBox.setInformativeText(message);
    msgBox.setWindowTitle(tr("Device error"));
    msgBox.setText(tr("Error message sent from manual input"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
  } else {
    emit disconnectSerial();
    QMessageBox msgBox(this);
    msgBox.setInformativeText("Message: " + message);
    msgBox.setWindowTitle(tr("Device error"));
    msgBox.setText(tr("Device reported error (port disconnected)"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}


void MainWindow::on_pushButtonSerialSetting_clicked() {
  serialSettingsDialog->show();
}

void MainWindow::on_pushButtonSerialMoreInfo_clicked() {
  QString portinfo;

  auto portlist = QSerialPortInfo::availablePorts();
  auto port = portlist.at(ui->comboBoxCom->currentIndex());

  portinfo.append(tr("Description: %1\n").arg(port.description()));
  portinfo.append(tr("Manufacturer: %1\n").arg(port.manufacturer()));
  portinfo.append(tr("Serial number: %1\n").arg(port.serialNumber()));
  portinfo.append(tr("Location: %1\n").arg(port.systemLocation()));
  portinfo.append(tr("Vendor Identifier: %1\n").arg(port.vendorIdentifier()));
  portinfo.append(tr("Product Identifier: %1").arg(port.productIdentifier()));

  QMessageBox msgBox(this);
  msgBox.setWindowTitle(port.portName());
  msgBox.setText(portinfo);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();
}

void MainWindow::on_comboBoxBaud_editTextChanged(const QString& arg1) {
  for (int i = 0; i < arg1.length(); i++) {
    if (!arg1.at(i).isDigit()) {
      ui->comboBoxBaud->setEditText(arg1.left(i));
      return;
    }
  }
}

void MainWindow::on_pushButtonHideCur1_clicked() {
  ui->checkBoxCur1Visible->setCheckState(Qt::CheckState::Unchecked);
  ui->checkBoxYCur1->setCheckState(Qt::CheckState::Unchecked);
}

void MainWindow::on_pushButtonHideCur2_clicked() {
  ui->checkBoxCur2Visible->setCheckState(Qt::CheckState::Unchecked);
  ui->checkBoxYCur2->setCheckState(Qt::CheckState::Unchecked);
}

void MainWindow::on_pushButtonAvg_toggled(bool checked) {
  for (int chID = 0; chID < ANALOG_COUNT; chID++)
    emit setAverager(chID, checked);
  emit resetAverager();
}

void MainWindow::on_spinBoxAvg_valueChanged(int arg1) {
  for (int i = 0; i < ANALOG_COUNT; i++)
    emit setAveragerCount(i, arg1);
}
