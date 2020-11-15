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
#include "plotmath.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  void init(QTranslator *translator, const PlotData *plotData, const PlotMath *plotMath);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  QTranslator *translator;
  QTimer plotUpdateTimer, portsRefreshTimer, activeChRefreshTimer, mathUpdateTimer;
  QList<QSerialPortInfo> portList;
  QListView *channelList;
  void connectSignals();
  void updateChScale();
  void changeLanguage(QString code = "en");
  void exportCSV(bool all = false, int ch = 1);
  int lastSelectedChannel = 1;
  void initSetables();
  QByteArray getSettings();
  void setUp();
  void startTimers();
  void setGuiDefaults();
  double cursorPos[4];
  PlotFrame_t plotFrame;
  void setGuiArrays();
  QScrollBar *cursors[4];
  QCheckBox *mathEn[4];
  QSpinBox *mathFirst[4], *mathSecond[4];
  QComboBox *mathOp[4];
  QMap<QString, QWidget *> setables;
  bool pendingDeviceMessage = false;
  void applyGuiElementSettings(QWidget *, QString value);
  QByteArray readGuiElementSettings(QWidget *target);
  void updateSelectedChannel(int arg1);
  void sendFileToParser(QFile &file, bool removeLastNewline = false, bool addSemicolums = false);
  void fillChannelSelect();
  void updateUsedChannels();

private slots:
  void setAdaptiveSpinBoxes();
  void scrollBarCursorValueChanged();
  void updateMath();
  void updateDivs(double vertical, double horizontal);
  void comRefresh();
  void cursorsSwitched() { on_pushButtonCursorsZero_clicked(); }
  void rangeTypeChanged();

  void on_tabs_right_currentChanged(int index);
  void on_dialRollingRange_realValueChanged(double value) { ui->doubleSpinBoxRangeHorizontal->setValue(value); }
  void on_dialVerticalRange_realValueChanged(double value) { ui->doubleSpinBoxRangeVerticalRange->setValue(value); }
  void on_pushButtonChannelColor_clicked();
  void on_doubleSpinBoxChOffset_valueChanged(double arg1);
  void on_comboBoxGraphStyle_currentIndexChanged(int index);
  void on_pushButtonConnect_clicked();
  void on_pushButtonSendCommand_clicked();
  void on_doubleSpinBoxChScale_valueChanged(double arg1);
  void on_dialChScale_realValueChanged(double value) { ui->doubleSpinBoxChScale->setValue(value); }
  void on_pushButtonSelectedCSV_clicked();
  void on_dialZoom_valueChanged(int value);
  void on_radioButtonEn_toggled(bool checked);
  void on_radioButtonCz_toggled(bool checked);
  void on_pushButtonAllCSV_clicked() { exportCSV(true); }
  void on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1) { ui->doubleSpinBoxChOffset->setSingleStep(pow(10, floor(log10(arg1)) - 2)); }
  void on_lineEditManualInput_returnPressed();
  void on_pushButtonLoadFile_clicked();
  void on_pushButtonDefaults_clicked();
  void on_checkBoxChInvert_toggled(bool checked) { ui->plot->changeChScale(ui->comboBoxSelectedChannel->currentIndex(), ui->doubleSpinBoxChScale->value() * (checked ? -1 : 1)); }
  void on_pushButtonSaveSettings_clicked();
  void on_pushButtonReset_clicked();
  void on_pushButtonCursorToView_clicked();
  void on_pushButtonAutoset_clicked();
  void on_pushButtonCursorsZero_clicked();
  void on_pushButtonCSVXY_clicked();
  void on_comboBoxHAxisType_currentIndexChanged(int index);
  void on_pushButtonOpenHelp_clicked();
  void on_pushButtonPositive_clicked();
  void on_pushButtonCenter_clicked();
  void on_pushButtonScrollDown_clicked();
  void on_checkBoxPlotOpenGL_toggled(bool checked);
  void on_pushButtonSendCommand_2_clicked();
  void on_lineEditCommand_returnPressed();
  void on_pushButtonClearBuffer_clicked();
  void on_pushButtonViewBuffer_clicked();
  void on_pushButtonClearBuffer_2_clicked();
  void on_pushButtonViewBuffer_2_clicked();
  void on_comboBoxOutputLevel_currentIndexChanged(int index);
  void on_comboBoxCom_currentIndexChanged(int) { emit disconnectSerial(); }
  void on_comboBoxBaud_currentIndexChanged(int) { emit disconnectSerial(); }
  void on_pushButtonScrollDown_2_clicked();
  void on_pushButtonScrollDown_3_clicked();
  void on_checkBoxSerialMonitor_toggled(bool checked);
  void on_comboBoxSelectedChannel_currentIndexChanged(int index);
  void on_pushButtonResetChannels_clicked();

public slots:
  void printMessage(QString messageHeader, QByteArray messageBody, int type, MessageTarget::enumerator target);
  void setCursorBounds(PlotFrame_t frame);
  void showPlotStatus(PlotStatus::enumerator type);
  void serialConnectResult(bool connected, QString message);
  void printToTerminal(QByteArray data) { ui->myTerminal->printToTerminal(data); }
  void serialFinishedWriting() { ui->lineEditCommand->clear(); }
  void useSettings(QByteArray settings, MessageTarget::enumerator source);
  void printDeviceMessage(QByteArray messageBody, bool warning, bool ended);
  void printSerialMonitor(QByteArray data);

signals:
  void requestSerialBufferClear();
  void requestSerialBufferShow();
  void requestManualBufferClear();
  void requestManualBufferShow();
  void toggleSerialConnection(QString port, int baud);
  void writeToSerial(QByteArray data);
  void resetChannels();
  void disconnectSerial();
  void requestMath(int resultCh, int operation, QPair<QVector<double>, QVector<double>>, QPair<QVector<double>, QVector<double>>);
  void requestXY(QPair<QVector<double>, QVector<double>>, QPair<QVector<double>, QVector<double>>);
  void sendManualInput(QByteArray data);
  void parseError(QByteArray, int type = DataLineType::debugMessage);
  void setSerialMessageLevel(OutputLevel::enumerator level);
  void setManualMessageLevel(OutputLevel::enumerator level);
  void enableSerialMonitor(bool enabled);
};
#endif // MAINWINDOW_H
