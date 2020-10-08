#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init() {
  connectSignals();
  changeLanguage();
  setGuiDefaults();
  updateChScale();
  startTimers();
  setAdaptiveSpinBoxes();
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
  ui->labelBinSettings->setVisible(ui->checkBoxModeManual->isChecked() || mode == DataMode::binData);
  ui->labelDataMode->setText(tr("Data mode: ") + ui->comboBoxDataMode->itemText(mode));
  ui->comboBoxDataMode->setCurrentIndex(mode);
}

void MainWindow::updateChScale() {
  double perDiv = ui->plot->getCHDiv(ui->spinBoxChannelSelect->value());
  ui->labelChScale->setText(QString::number(perDiv) + tr(" / Div"));
}

int MainWindow::roundToStandardValue(double value) {
  for (int i = 0; i < LOG_SET_SIZE; i++)
    if (value <= logaritmicSettings[i])
      return i;
  return 28;
}

void MainWindow::scrollBarCursorValueChanged() {
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

void MainWindow::serialConnectResult(bool connected, QString message) {
  ui->pushButtonDisconnect->setEnabled(connected);
  ui->pushButtonConnect->setEnabled(!connected);
  ui->comboBoxCom->setEnabled(!connected);
  ui->comboBoxBaud->setEnabled(!connected);
  ui->labelPortInfo->setText(message);
  ui->pushButtonSendCommand->setEnabled(connected);
  if (connected && ui->checkBoxClearOnReconnect->isChecked()) {
    ui->plot->resetChannels();
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
  ui->plot->setVerticalDiv(logaritmicSettings[MAX(roundToStandardValue(vertical) + ui->dialVerticalDiv->value(), 0)]);
  ui->plot->setHorizontalDiv(logaritmicSettings[MAX(roundToStandardValue(horizontal) + ui->dialhorizontalDiv->value(), 0)]);
  updateChScale();
  ui->labelHDiv->setText(QString::number(ui->plot->getHDiv()) + tr(" / Div"));
  ui->labelVDiv->setText(QString::number(ui->plot->getVDiv()) + tr(" / Div"));
}
