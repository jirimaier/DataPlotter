#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init(QTranslator *translator, const PlotData *plotData, const PlotMath *plotMath) {
  fillChannelSelect();
  QObject::connect(plotMath, &PlotMath::sendResult, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotMath, &PlotMath::sendResultXY, ui->plotxy, &MyXYPlot::newData);
  QObject::connect(plotData, &PlotData::clearXY, ui->plotxy, &MyXYPlot::clear);
  QObject::connect(plotData, &PlotData::addVectorToPlot, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotData, &PlotData::addPointToPlot, ui->plot, &MyMainPlot::newDataPoint);
  QObject::connect(plotData, &PlotData::clearLogic, ui->plot, &MyMainPlot::clearLogicGroup);
  QObject::connect(plotData, &PlotData::clearAnalog, ui->plot, &MyMainPlot::clearCh);

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
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT) {
    double perDiv = ui->plot->getCHDiv(ui->comboBoxSelectedChannel->currentIndex());
    ui->labelChScale->setText(QString::number(perDiv, 'f', 3) + tr(" / Div"));
  } else
    ui->labelChScale->setText(tr("---"));
}

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonConnect->setIcon(connected ? QPixmap(":/images/icons/connected.png") : QPixmap(":/images/icons/disconnected.png"));
  ui->labelPortInfo->setText(message);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
    ui->plotxy->clear();
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
    ui->labelHDiv->setText(QString::number(ui->plot->getHDiv()) + tr(" / Div"));
  else
    ui->labelHDiv->setText("---");
  ui->labelVDiv->setText(QString::number(ui->plot->getVDiv()) + tr(" / Div"));
}

void MainWindow::on_pushButtonCenter_clicked() { ui->dialVerticalCenter->setValue(0); }

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
  QScrollBar *scroll = ui->plainTextEditConsole->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
  pendingDeviceMessage = !ended;
}

void MainWindow::on_labelLicense_linkActivated() {
  QString licenseFile;
  if (ui->radioButtonEn->isChecked())
    licenseFile = QCoreApplication::applicationDirPath() + "/license_en.txt";
  else
    licenseFile = QCoreApplication::applicationDirPath() + "/license_cz.txt";
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(licenseFile))) {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant open file."));
    msgBox.setInformativeText(licenseFile);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_pushButtonHideCh_toggled(bool checked) {
  if (checked)
    ui->pushButtonHideCh->setIcon(QPixmap(":/images/icons/hidden.png"));
  else
    ui->pushButtonHideCh->setIcon(QPixmap(":/images/icons/visible.png"));

  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT)
    ui->plot->setChVisible(ui->comboBoxSelectedChannel->currentIndex(), !checked);
  else
    ui->plot->setLogicVisibility(ui->comboBoxSelectedChannel->currentIndex() - ANALOG_COUNT - MATH_COUNT, !checked);
}

void MainWindow::on_checkBoxMath1_toggled(bool checked) {
  emit setMathFirst(1, checked ? ui->spinBoxMath1First->value() : 0);
  emit setMathSecond(1, checked ? ui->spinBoxMath1Second->value() : 0);
  MathOperations::enumetrator operation = (MathOperations::enumetrator)ui->comboBoxMath1Op->currentIndex();
  emit setMathMode(1, operation);
}

void MainWindow::on_checkBoxMath2_toggled(bool checked) {
  emit setMathFirst(2, checked ? ui->spinBoxMath2First->value() : 0);
  emit setMathSecond(2, checked ? ui->spinBoxMath2Second->value() : 0);
  MathOperations::enumetrator operation = (MathOperations::enumetrator)ui->comboBoxMath2Op->currentIndex();
  emit setMathMode(2, operation);
}

void MainWindow::on_checkBoxMath3_toggled(bool checked) {
  emit setMathFirst(3, checked ? ui->spinBoxMath3First->value() : 0);
  emit setMathSecond(3, checked ? ui->spinBoxMath3Second->value() : 0);
  MathOperations::enumetrator operation = (MathOperations::enumetrator)ui->comboBoxMath3Op->currentIndex();
  emit setMathMode(3, operation);
}

void MainWindow::on_checkBoxMath4_toggled(bool checked) {
  emit setMathFirst(4, checked ? ui->spinBoxMath4First->value() : 0);
  emit setMathSecond(4, checked ? ui->spinBoxMath4Second->value() : 0);
  MathOperations::enumetrator operation = (MathOperations::enumetrator)ui->comboBoxMath4Op->currentIndex();
  emit setMathMode(4, operation);
}

void MainWindow::on_checkBoxXY_toggled(bool checked) {
  emit setXYFirst(checked ? ui->spinBoxXYFirst->value() : 0);
  emit setXYSecond(checked ? ui->spinBoxXYSecond->value() : 0);
}

void MainWindow::on_dial_valueChanged(int value) {
  ui->plotxy->setGridHintX(value);
  ui->plotxy->setGridHintY(value);
}
