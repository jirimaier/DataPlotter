#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init(QTranslator *translator, const PlotData *plotData, const PlotMath *plotMath) {
  fillChannelSelect();
  QObject::connect(plotMath, &PlotMath::sendResult, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotMath, &PlotMath::sendResultXY, ui->plotxy, &MyXYPlot::newData);
  QObject::connect(plotData, &PlotData::addVectorToPlot, ui->plot, &MyMainPlot::newDataVector);
  QObject::connect(plotData, &PlotData::addPointToPlot, ui->plot, &MyMainPlot::newDataPoint);
  QObject::connect(plotData, &PlotData::addLogicVectorToPlot, ui->plot, &MyMainPlot::newLogicDataVector);
  QObject::connect(plotData, &PlotData::clearLogic, ui->plot, &MyMainPlot::clearLogicGroup);

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
  if (ui->comboBoxSelectedChannel->currentIndex() < ANALOG_COUNT + MATH_COUNT) {
    double perDiv = ui->plot->getCHDiv(ui->comboBoxSelectedChannel->currentIndex());
    ui->labelChScale->setText(QString::number(perDiv) + tr(" / Div"));
  } else
    ui->labelChScale->setText(tr("---"));
}

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonConnect->setIcon(connected ? QPixmap(":/images/icons/connected.png") : QPixmap(":/images/icons/disconnected.png"));
  ui->labelPortInfo->setText(message);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
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
