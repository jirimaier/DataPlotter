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
  QTimer portsRefreshTimer, activeChRefreshTimer, cursorRangeUpdateTimer;
  QList<QSerialPortInfo> portList;
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
  void setGuiArrays();
  QPushButton *mathEn[4];
  QSpinBox *mathFirst[4], *mathSecond[4];
  QComboBox *mathOp[4];
  QMap<QString, QWidget *> setables;
  bool pendingDeviceMessage = false;
  void applyGuiElementSettings(QWidget *, QString value);
  QByteArray readGuiElementSettings(QWidget *target);
  void updateSelectedChannel(int arg1);
  void sendFileToParser(QByteArray text, bool removeLastNewline = false, bool addSemicolums = false);
  void fillChannelSelect();
  void updateUsedChannels();
  void updateChannelComboBox(QComboBox &combobox);
  bool colorUpdateNeeded = true;
  void updateMathNow(int number);
  void updateXYNow();
  QIcon iconXY, iconRun, iconPause, iconHidden, iconVisible, iconConnected, iconNotConnected;

 private slots:
  void updateCursors();
  void setAdaptiveSpinBoxes();
  void updateDivs();
  void comRefresh();
  void rangeTypeChanged();
  void updateCursor(Cursors::enumerator cursor, int selectedChannel, unsigned int sample, double &time, double &value, QByteArray &timeStr, QByteArray &valueStr);
  void updateCursorRange();

 private slots:  // Autoconnect slots
  void on_tabs_right_currentChanged(int index);
  void on_dialRollingRange_realValueChanged(double value) { ui->doubleSpinBoxRangeHorizontal->setValue(value); }
  void on_dialVerticalRange_realValueChanged(double value) { ui->doubleSpinBoxRangeVerticalRange->setValue(value); }
  void on_doubleSpinBoxChOffset_valueChanged(double arg1);
  void on_comboBoxGraphStyle_currentIndexChanged(int index);
  void on_pushButtonConnect_clicked();
  void on_pushButtonSendCommand_clicked() { on_lineEditCommand_returnPressed(); }
  void on_pushButtonSendCommand_2_clicked() { on_lineEditCommand_2_returnPressed(); }
  void on_pushButtonSendCommand_3_clicked() { on_lineEditCommand_3_returnPressed(); }
  void on_pushButtonSendCommand_4_clicked() { on_lineEditCommand_4_returnPressed(); }
  void on_doubleSpinBoxChScale_valueChanged(double arg1);
  void on_dialChScale_realValueChanged(double value) { ui->doubleSpinBoxChScale->setValue(value); }
  void on_pushButtonSelectedCSV_clicked() { exportCSV(false, ui->comboBoxSelectedChannel->currentIndex()); }
  void on_dialZoom_valueChanged(int value);
  void on_radioButtonEn_toggled(bool checked);
  void on_radioButtonCz_toggled(bool checked);
  void on_pushButtonAllCSV_clicked() { exportCSV(true); }
  void on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1) { ui->doubleSpinBoxChOffset->setSingleStep(pow(10, floor(log10(arg1)) - 2)); }
  void on_lineEditManualInput_returnPressed();
  void on_pushButtonLoadFile_clicked();
  void on_pushButtonDefaults_clicked();
  void on_pushButtonSaveSettings_clicked();
  void on_pushButtonReset_clicked();
  void on_pushButtonAutoset_clicked();
  void on_pushButtonCSVXY_clicked() { exportCSV(false, XY_CHANNEL); }
  void on_comboBoxHAxisType_currentIndexChanged(int index);
  void on_pushButtonOpenHelp_clicked();
  void on_pushButtonCenter_clicked();
  void on_pushButtonScrollDown_clicked();
  void on_pushButtonSendManual_clicked() { on_lineEditManualInput_returnPressed(); }
  void on_lineEditCommand_returnPressed();
  void on_lineEditCommand_2_returnPressed();
  void on_lineEditCommand_3_returnPressed();
  void on_lineEditCommand_4_returnPressed();
  void on_pushButtonClearBuffer_clicked() { emit requestSerialBufferClear(); }
  void on_pushButtonViewBuffer_clicked() { emit requestSerialBufferShow(); }
  void on_pushButtonClearBuffer_2_clicked() { emit requestManualBufferClear(); }
  void on_pushButtonViewBuffer_2_clicked() { emit requestManualBufferShow(); }
  void on_comboBoxOutputLevel_currentIndexChanged(int index);
  void on_comboBoxCom_currentIndexChanged(int) { emit disconnectSerial(); }
  void on_comboBoxBaud_currentIndexChanged(int) { emit disconnectSerial(); }
  void on_pushButtonScrollDown_2_clicked();
  void on_pushButtonScrollDown_3_clicked();
  void on_checkBoxSerialMonitor_toggled(bool checked);
  void on_comboBoxSelectedChannel_currentIndexChanged(int index);
  void on_pushButtonResetChannels_clicked();
  void on_pushButtonLog1_toggled(bool checked) { emit setChDigital(1, checked ? ui->spinBoxLog1source->value() : 0); }
  void on_pushButtonLog2_toggled(bool checked) { emit setChDigital(2, checked ? ui->spinBoxLog2source->value() : 0); }
  void on_pushButtonLog3_toggled(bool checked) { emit setChDigital(3, checked ? ui->spinBoxLog3source->value() : 0); }
  void on_spinBoxLog1bits_valueChanged(int arg1) { emit setLogicBits(1, arg1); }
  void on_spinBoxLog2bits_valueChanged(int arg1) { emit setLogicBits(2, arg1); }
  void on_spinBoxLog3bits_valueChanged(int arg1) { emit setLogicBits(3, arg1); }
  void on_spinBoxLog1source_valueChanged(int arg1) { emit setChDigital(1, ui->pushButtonLog1->isChecked() ? arg1 : 0); }
  void on_spinBoxLog2source_valueChanged(int arg1) { emit setChDigital(2, ui->pushButtonLog2->isChecked() ? arg1 : 0); }
  void on_spinBoxLog3source_valueChanged(int arg1) { emit setChDigital(3, ui->pushButtonLog3->isChecked() ? arg1 : 0); }
  void on_pushButtonPlotImage_clicked();
  void on_pushButtonXYImage_clicked();
  void on_checkBoxCur1Visible_toggled(bool checked);
  void on_checkBoxCur2Visible_toggled(bool checked);
  void on_pushButtonChangeChColor_clicked();
  void on_pushButtonClearAll_clicked() { ui->plot->resetChannels(); }
  void on_checkBoxChInverted_toggled(bool checked);
  void on_pushButtonHideCh_toggled(bool checked);
  void on_pushButtonMath1_toggled(bool) { updateMathNow(1); }
  void on_pushButtonMath2_toggled(bool) { updateMathNow(2); }
  void on_pushButtonMath3_toggled(bool) { updateMathNow(3); }
  void on_comboBoxMath1Op_currentIndexChanged(int) { updateMathNow(1); }
  void on_comboBoxMath2Op_currentIndexChanged(int) { updateMathNow(2); }
  void on_comboBoxMath3Op_currentIndexChanged(int) { updateMathNow(3); }
  void on_spinBoxMath1First_valueChanged(int) { updateMathNow(1); };
  void on_spinBoxMath2First_valueChanged(int) { updateMathNow(2); };
  void on_spinBoxMath3First_valueChanged(int) { updateMathNow(3); };
  void on_spinBoxMath1Second_valueChanged(int) { updateMathNow(1); };
  void on_spinBoxMath2Second_valueChanged(int) { updateMathNow(2); };
  void on_spinBoxMath3Second_valueChanged(int) { updateMathNow(3); };
  void on_spinBoxXYFirst_valueChanged(int) { updateXYNow(); }
  void on_spinBoxXYSecond_valueChanged(int) { updateXYNow(); }
  void on_dial_valueChanged(int value);
  void on_radioButtonRollingRange_toggled(bool checked);
  void on_pushButtonXY_toggled(bool checked);
  void on_comboBoxCursor1Channel_currentIndexChanged(int index);
  void on_comboBoxCursor2Channel_currentIndexChanged(int index);
  void on_labelLicense_linkActivated(const QString &link) { QDesktopServices::openUrl(link); }
  void on_pushButtonPositive_clicked();
  void on_spinBoxCur1Sample_valueChanged(int arg1);
  void on_spinBoxCur2Sample_valueChanged(int arg1);

 public slots:
  void printMessage(QString messageHeader, QByteArray messageBody, int type, MessageTarget::enumerator target);
  void showPlotStatus(PlotStatus::enumerator type);
  void serialConnectResult(bool connected, QString message);
  void printToTerminal(QByteArray data) { ui->myTerminal->printToTerminal(data); }
  void serialFinishedWriting();
  void useSettings(QByteArray settings, MessageTarget::enumerator source);
  void printDeviceMessage(QByteArray message, bool warning, bool ended);
  void printSerialMonitor(QByteArray data) { ui->plainTextEditConsole_3->appendPlainText(data); }

 signals:
  void setChDigital(int chid, int target);
  void setLogicBits(int target, int bits);
  void requestSerialBufferClear();
  void requestSerialBufferShow();
  void requestManualBufferClear();
  void requestManualBufferShow();
  void toggleSerialConnection(QString port, int baud);
  void writeToSerial(QByteArray data);
  void resetChannels();
  void disconnectSerial();
  void sendManualInput(QByteArray data);
  void parseError(QByteArray, int type = DataLineType::debugMessage);
  void setSerialMessageLevel(OutputLevel::enumerator level);
  void setManualMessageLevel(OutputLevel::enumerator level);
  void enableSerialMonitor(bool enabled);
  void setMathFirst(int math, int ch);
  void setMathSecond(int math, int ch);
  void setXYFirst(int ch);
  void setXYSecond(int ch);
  void clearMath(int math);
  void clearXY();
  void resetMath(int mathNumber, MathOperations::enumetrator mode, QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2, bool shouldIgnorePause = false);
  void resetXY(QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2, bool shouldIgnorePause = false);
};
#endif  // MAINWINDOW_H
