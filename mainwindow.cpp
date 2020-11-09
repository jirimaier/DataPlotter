#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init(QTranslator *translator) {
  this->translator = translator;
  setGuiArrays();
  initSetables();
  connectSignals();
  changeLanguage();
  setGuiDefaults();
  updateChScale();
  startTimers();
  setAdaptiveSpinBoxes();
  setUp();
}

void MainWindow::changeLanguage(QString code) {
  if (!translator->load(QString(":/translations/translation_%1.qm").arg(code))) {
    qDebug() << "Can not load " << QString(":/translations/translation_%1.qm").arg(code);
    return;
  }
  qApp->installTranslator(translator);
  ui->retranslateUi(this);
}

void MainWindow::showPlotStatus(PlotStatus::enumerator type) {
  if (type == PlotStatus::pause) {
    ui->pushButtonPause->setIcon(QPixmap(":/images/icons/pause.png"));
    ui->pushButtonPause->setToolTip(tr("Paused (click to resume)"));
  } else if (type == PlotStatus::run) {
    ui->pushButtonPause->setIcon(QPixmap(":/images/icons/run.png"));
    ui->pushButtonPause->setToolTip(tr("Running (click to pause)"));
  }
}

void MainWindow::updateChScale() {
  double perDiv = ui->plot->getCHDiv(ui->spinBoxChannelSelect->value());
  ui->labelChScale->setText(QString::number(perDiv) + tr(" / Div"));
}

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonConnect->setIcon(connected ? QPixmap(":/images/icons/connected.png") : QPixmap(":/images/icons/disconnected.png"));
  ui->labelPortInfo->setText(message);
  ui->pushButtonSendCommand->setEnabled(connected);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
    ui->myTerminal->clearTerminal();
    emit resetChannels();
  }
}

void MainWindow::updateDivs(double vertical, double horizontal) {
  ui->plot->setVerticalDiv(Global::logaritmicSettings[MAX(GlobalFunctions::roundToStandardValue(vertical) + ui->dialVerticalDiv->value(), 0)]);
  ui->plot->setHorizontalDiv(Global::logaritmicSettings[MAX(GlobalFunctions::roundToStandardValue(horizontal) + ui->dialhorizontalDiv->value(), 0)]);
  updateChScale();
  if (ui->labelHDiv->isEnabled())
    ui->labelHDiv->setText(QString::number(ui->plot->getHDiv()) + tr(" / Div"));
  else
    ui->labelHDiv->setText("---");
  ui->labelVDiv->setText(QString::number(ui->plot->getVDiv()) + tr(" / Div"));
}

void MainWindow::on_comboBoxHAxisType_currentIndexChanged(int index) {
  ui->labelHDiv->setEnabled(index <= 1);
  ui->plot->setShowHorizontalValues(index);
}

void MainWindow::on_pushButtonOpenHelp_clicked() {
  QString helpFile = QCoreApplication::applicationDirPath() + "/help.pdf";
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(helpFile))) {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant open file."));
    msgBox.setInformativeText(helpFile);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_pushButtonCenter_clicked() { ui->dialVerticalCenter->setValue(0); }

void MainWindow::printMessage(QByteArray messageHeader, QByteArray messageBody, int type, MessageTarget::enumerator target) {
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
    for (QByteArray::iterator it = messageBody.begin(); it != messageBody.end(); it++)
      stringMessage.append(QString::number((uint8_t)*it, 16).rightJustified(2, '0') + " ");
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

void MainWindow::printDeviceMessage(QByteArray messageBody, bool warning, bool ended) {
  if (!pendingDeviceMessage) {
    if (warning)
      ui->plainTextEditConsole->appendHtml("<font color=darkred>Device warning:</font color> ");
    else
      ui->plainTextEditConsole->appendHtml("<font color=darkgreen>Device message:</font color> ");
  }
  ui->plainTextEditConsole->moveCursor(QTextCursor::End);
  ui->plainTextEditConsole->insertPlainText(messageBody);
  ui->plainTextEditConsole->moveCursor(QTextCursor::End);
  pendingDeviceMessage = !ended;
}

void MainWindow::printSerialMonitor(QByteArray data) { ui->plainTextEditConsole_3->appendPlainText(data); }

void MainWindow::on_pushButtonScrollDown_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
};

void MainWindow::on_checkBoxPlotOpenGL_toggled(bool checked) {
  ui->plot->setOpenGl(checked);
  ui->plotxy->setOpenGl(checked);
}

void MainWindow::on_pushButtonSendCommand_2_clicked() { on_lineEditManualInput_returnPressed(); }

void MainWindow::on_lineEditCommand_returnPressed() {
  QString text = ui->lineEditCommand->text() + Global::lineEndings[ui->comboBoxLineEnding->currentIndex()];
  emit writeToSerial(text.toUtf8());
}

void MainWindow::on_pushButtonClearBuffer_clicked() { emit requestSerialBufferClear(); }

void MainWindow::on_pushButtonViewBuffer_clicked() { emit requestSerialBufferShow(); }

void MainWindow::on_pushButtonClearBuffer_2_clicked() { emit requestManualBufferClear(); }

void MainWindow::on_pushButtonViewBuffer_2_clicked() { emit requestManualBufferShow(); }

void MainWindow::on_comboBoxOutputLevel_currentIndexChanged(int index) {
  if (index >= 0)
    emit setSerialMessageLevel((OutputLevel::enumerator)index);
}

void MainWindow::on_toolButton_triggered(QAction *arg1) { qDebug() << arg1; }

void MainWindow::on_pushButtonScrollDown_2_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole_2->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole_2->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
}

void MainWindow::on_pushButtonScrollDown_3_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole_3->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole_3->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
}

void MainWindow::on_checkBoxSerialMonitor_toggled(bool checked) {
  ui->frameSerialMonitor->setVisible(checked);
  emit enableSerialMonitor(checked);
}
