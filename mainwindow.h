#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QMainWindow>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QTime>
#include <QTimer>
#include <QTranslator>
#include <QtCore>

#include "enums_defines_constants.h"
#include "plotdata.h"
#include "serialparser.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  void init();
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  QTimer plotUpdateTimer;
  QTimer listUpdateTimer;
  QTimer portsRefreshTimer;
  QList<QSerialPortInfo> portList;
  QStringList receivedListBuffer;
  void dataParser(QByteArray message);
  BinDataSettings_t binSettings;
  void connectSignals();
  void updateChScale();
  void changeLanguage(QString code = "en");
  void exportCSV(bool all = false, int ch = 1);
  int lastSelectedChannel = 1;
  int roundToStandardValue(double value);
  void printOutput(QString line);
  QByteArray getSettings();
  void setUp();
  void startTimers();
  void setGuiDefaults();
  double plotTop = 10, plotBottom = -10, plotRight = 10, plotLeft = 0;

private slots:
  void setAdaptiveSpinBoxes();
  void scrollBarCursorValueChanged();
  void updateInfo();
  void updatePlot();
  void updateDivs(double vertical, double horizontal);
  void comRefresh();

  void on_tabs_right_currentChanged(int index);
  void on_dialRollingRange_realValueChanged(double value) { ui->doubleSpinBoxRangeHorizontal->setValue(value); }
  void on_dialVerticalRange_realValueChanged(double value) { ui->doubleSpinBoxRangeVerticalRange->setValue(value); }
  void on_pushButtonClearChannels_clicked();
  void on_pushButtonChannelColor_clicked();
  void on_spinBoxChannelSelect_valueChanged(int arg1);
  void on_doubleSpinBoxChOffset_valueChanged(double arg1);
  void on_comboBoxGraphStyle_currentIndexChanged(int index);
  void on_pushButtonConnect_clicked();
  void on_pushButtonDisconnect_clicked();
  void on_pushButtonSendCommand_clicked();
  void on_doubleSpinBoxChScale_valueChanged(double arg1);
  void on_dialChScale_realValueChanged(double value) { ui->doubleSpinBoxChScale->setValue(value); }
  void on_pushButtonSelectedCSV_clicked();
  void on_dialZoom_valueChanged(int value);
  void on_comboBoxPlotRangeType_currentIndexChanged(int index) { ui->plot->setRangeType(index); }
  void on_radioButtonEn_toggled(bool checked);
  void on_radioButtonCz_toggled(bool checked);
  void on_lineEditCommand_returnPressed() { on_pushButtonSendCommand_clicked(); }
  void on_horizontalSliderLineTimeout_valueChanged(int value);
  void on_pushButtonPrintBuffer_clicked();
  void on_pushButtonDataModeApply_clicked();
  void on_checkBoxModeManual_toggled(bool checked);
  void on_comboBoxOutputLevel_currentIndexChanged(int index) { emit setOutputLevel(index); }
  void on_pushButtonAllCSV_clicked() { exportCSV(true); }
  void on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1) { ui->doubleSpinBoxChOffset->setSingleStep(pow(10, floor(log10(arg1)) - 2)); }
  void on_lineEditManualInput_returnPressed();
  void on_pushButtonLoadFile_clicked();
  void on_pushButtonDefaults_clicked();
  void on_checkBoxChInvert_toggled(bool checked) { ui->plot->changeChScale(ui->spinBoxChannelSelect->value(), ui->doubleSpinBoxChScale->value() * (checked ? -1 : 1)); }
  void on_pushButtonSaveSettings_clicked();
  void on_pushButtonReset_clicked();
  void on_pushButtonCursorToView_clicked();

public slots:
  void setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminfull, double xmaxfull, double yminfull, double ymaxfull);
  void changedDataMode(int mode);
  void changedBinSettings(BinDataSettings_t in_settings) { binSettings = in_settings; }
  void showProcessedCommand(QString message) { receivedListBuffer.append(message); }
  void printMessage(QByteArray data, bool urgent);
  void showPlotStatus(int type);
  void addDataToPlot(int ch, QVector<double> *time, QVector<double> *value, bool continous, bool sorted) { ui->plot->newData(ch, time, value, continous, sorted); }
  void serialConnectResult(bool connected, QString message);
  void printToTerminal(QByteArray data) { ui->myTerminal->printToTerminal(data); }
  void serialFinishedWriting() { ui->lineEditCommand->clear(); }
  void bufferDebug(QByteArray data);
  void useSettings(QByteArray settings);

signals:
  void allowModeChange(bool);
  void connectSerial(QString port, int baud);
  void disconnectSerial();
  void setBinParameters(BinDataSettings_t data);
  void setMode(int mode);
  void writeToSerial(QByteArray data);
  void changeLineTimeout(int);
  void requestBufferDebug();
  void requestNewDataForPlot(QVector<double> offsets, QVector<double> scales);
  void resetChannels();
  void setOutputLevel(int);
  void requestMath(int resultCh, int operation, QPair<QVector<double>, QVector<double>>, QPair<QVector<double>, QVector<double>>);
  void sendManaulInput(QByteArray data, int type);
  void parseError(QByteArray, int type = DataLineType::debugMessage);
};
#endif // MAINWINDOW_H
