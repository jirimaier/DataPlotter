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

void MainWindow::printMessage(QByteArray data, bool urgent) {
  QString message = QString("<font color=grey>%1: </font>").arg(QString(QTime::currentTime().toString("hh:mm:ss")));
  if (urgent)
    message.append(QString("<font color=red>%1</font>").arg(QString(data + "\n")));
  else
    message.append(QString("<font color=black>%1</font>").arg(QString(data + "\n")));
  ui->textEditMessages->append(message);
}

void MainWindow::changeLanguage(QString code) {
  if (!translator->load(QString(":/translations/translation_%1.qm").arg(code))) {
    qDebug() << "Can not load " << QString(":/translations/translation_%1.qm").arg(code);
    return;
  }
  qApp->installTranslator(translator);
  ui->retranslateUi(this);
}

void MainWindow::showPlotStatus(int type) {
  if (type == PlotStatus::pause) {
    ui->pushButtonPause->setText(tr("Resume"));
    ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/pause.png"));
  } else if (type == PlotStatus::run) {
    ui->pushButtonPause->setText(tr("Pause"));
    ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/run.png"));
  } else if (type == PlotStatus::single) {
    ui->pushButtonPause->setText(tr("Normal"));
    ui->labelPauseResume->setPixmap(QPixmap(":/images/icons/single.png"));
  }
  ui->pushButtonSingleTriger->setEnabled(type != PlotStatus::single);
}

void MainWindow::changedDataMode(int mode) {
  dataMode = mode;
  ui->labelBinSettings->setVisible(mode == DataMode::binData);
  ui->labelDataMode->setText(tr("Data mode: ") + ui->comboBoxDataMode->itemText(mode));
  ui->comboBoxDataMode->setCurrentIndex(mode);
}

void MainWindow::updateChScale() {
  double perDiv = ui->plot->getCHDiv(ui->spinBoxChannelSelect->value());
  ui->labelChScale->setText(QString::number(perDiv) + tr(" / Div"));
}

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonDisconnect->setEnabled(connected);
  ui->pushButtonConnect->setEnabled(!connected);
  ui->comboBoxCom->setEnabled(!connected);
  ui->comboBoxBaud->setEnabled(!connected);
  ui->labelPortInfo->setText(message);
  ui->pushButtonSendCommand->setEnabled(connected);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
    ui->myTerminal->clearTerminal();
    emit resetChannels();
  }
}

void MainWindow::bufferDebug(QByteArray data) {
  QString stringData = QString(data).simplified();
  if (stringData.length() > 0) {
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

void MainWindow::updateDivs(double vertical, double horizontal) {
  ui->plot->setVerticalDiv(Global::logaritmicSettings[MAX(GlobalFunctions::roundToStandardValue(vertical) + ui->dialVerticalDiv->value(), 0)]);
  ui->plot->setHorizontalDiv(Global::logaritmicSettings[MAX(GlobalFunctions::roundToStandardValue(horizontal) + ui->dialhorizontalDiv->value(), 0)]);
  updateChScale();
  ui->labelHDiv->setText(QString::number(ui->plot->getHDiv()) + tr(" / Div"));
  ui->labelVDiv->setText(QString::number(ui->plot->getVDiv()) + tr(" / Div"));
}

void MainWindow::on_comboBoxHAxisType_currentIndexChanged(int index) {
  ui->labelHDiv->setVisible(index <= 1);
  ui->plot->setShowHorizontalValues(index);
}
