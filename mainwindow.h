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

#include "enumsDefinesConstants.h"
#include "plotdata.h"
#include "plotmath.h"
#include "serialreader.h"
#include "signalprocessing.h"
#include "interpolator.h"
#include "ui_mainwindow.h"
#include "myplot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  void init(QTranslator* translator, const PlotData* plotData, const PlotMath* plotMath, const SerialReader* serialReader);
  ~MainWindow();

 private:
  Ui::MainWindow* ui;
  QTranslator* translator;
  QTimer portsRefreshTimer, activeChRefreshTimer, xyTimer, cursorRangeUpdateTimer, measureRefreshTimer1, measureRefreshTimer2, fftTimer1, fftTimer2, serialMonitorTimer, interpolationTimer;
  QList<QSerialPortInfo> portList;

  void setComboboxItemVisible(QComboBox& comboBox, int index, bool visible);
  void setChStyleSelection(GraphType::enumGraphType type);

  int lastSelectedChannel = 1;

  QPushButton* mathEn[4];
  QSpinBox* mathFirst[4];
  QSpinBox* mathSecond[4];
  QComboBox* mathOp[4];

  QTimer dataRateTimer;
  int dataUpdates = 0;
  bool lastUpdateWasPoint = false;
  bool lastUpdateWasLogic = false;
  bool autoAutosetPending = false;
  HAxisType::enumHAxisType recommandedAxisType = HAxisType::normal;

  QMap<QString, QWidget*> setables;
  void initSetables();
  QByteArray getSettings();
  void applyGuiElementSettings(QWidget*, QString value);
  QByteArray readGuiElementSettings(QWidget* target);

  void connectSignals();
  void setUp();
  void startTimers();
  void setGuiDefaults();
  void setGuiArrays();

  void updateChScale();
  void changeLanguage(QString code = "en");

  void exportCSV(bool all = false, int ch = 1);

  void insertInTerminalDebug(QString text, QColor textColor);

  bool colorUpdateNeeded = true;
  void fillChannelSelect();
  void updateChannelComboBox(QComboBox& combobox, bool includeLogic, bool leaveAtLeastOne);
  void updateUsedChannels();
  void updateSelectedChannel(int arg1);

  bool pendingDeviceMessage = false;

  void updateMathNow(int number);
  void updateXY();

  QIcon iconRun, iconPause, iconHidden, iconVisible, iconConnected, iconNotConnected, iconCross;

  QByteArray serialMonitor;

  void setCursorsVisibility(Cursors::enumCursors cursor, int graph, bool timeCurVisible, int valueCurState);
  void updateXYCursorsCalculations();

  int interpolationsRunning = 0;

 private slots:
  void updateCursors();
  void setAdaptiveSpinBoxes();
  void updateDivs();
  void comRefresh();
  void rangeTypeChanged();
  void updateCursor(Cursors::enumCursors cursor, int selectedChannel, unsigned int sample, double& time, double& value, QByteArray& timeStr, QByteArray& valueStr, bool useValueCursor);
  void updateCursorRange();
  void updateMeasurements1();
  void updateMeasurements2();
  void updateFFT1();
  void updateFFT2();
  void updateInterpolation();
  void updateSerialMonitor();
  void updateDataRate();

 private slots: // Autoconnect slots
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
  //void on_dialChScale_realValueChanged(double value) { ui->doubleSpinBoxChScale->setValue(value); }
  void on_pushButtonSelectedCSV_clicked() { exportCSV(false, ui->comboBoxSelectedChannel->currentIndex()); }
  void on_dialZoom_valueChanged(int value);
  void on_radioButtonEn_toggled(bool checked);
  void on_radioButtonCz_toggled(bool checked);
  void on_pushButtonAllCSV_clicked() { exportCSV(true); }
  void on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1);
  void on_lineEditManualInput_returnPressed();
  void on_pushButtonLoadFile_clicked();
  void on_pushButtonDefaults_clicked();
  void on_pushButtonSaveSettings_clicked();
  void on_pushButtonReset_clicked();
  void on_pushButtonAutoset_clicked();
  void on_pushButtonCSVXY_clicked() { exportCSV(false, EXPORT_XY); }
  void on_pushButtonCSVFFT_clicked() { exportCSV(false, EXPORT_FFT); }
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
  void on_checkBoxSerialMonitor_toggled(bool checked);
  void on_comboBoxSelectedChannel_currentIndexChanged(int index);
  void on_pushButtonResetChannels_clicked();
  void on_pushButtonLog1_toggled(bool checked) { emit setChDigital(1, checked ? ui->spinBoxLog1source->value() : 0); }
  void on_pushButtonLog2_toggled(bool checked) { emit setChDigital(2, checked ? ui->spinBoxLog2source->value() : 0); }
  void on_spinBoxLog1bits_valueChanged(int arg1) { emit setLogicBits(1, arg1); }
  void on_spinBoxLog2bits_valueChanged(int arg1) { emit setLogicBits(2, arg1); }
  void on_spinBoxLog1source_valueChanged(int arg1) { emit setChDigital(1, ui->pushButtonLog1->isChecked() ? arg1 : 0); }
  void on_spinBoxLog2source_valueChanged(int arg1) { emit setChDigital(2, ui->pushButtonLog2->isChecked() ? arg1 : 0); }
  void on_pushButtonPlotImage_clicked();
  void on_pushButtonXYImage_clicked();
  void on_checkBoxCur1Visible_toggled(bool checked);
  void on_checkBoxCur2Visible_toggled(bool checked);
  void on_pushButtonChangeChColor_clicked();
  void on_pushButtonClearAll_clicked();
  void on_pushButtonInvert_toggled(bool checked);
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
  void on_horizontalSliderXYGrid_valueChanged(int value);
  void on_pushButtonXY_toggled(bool checked);
  void on_comboBoxCursor1Channel_currentIndexChanged(int index);
  void on_comboBoxCursor2Channel_currentIndexChanged(int index);
  void on_pushButtonPositive_clicked();
  void on_pushButtonNegative_clicked();
  void on_spinBoxCur1Sample_valueChanged(int arg1);
  void on_spinBoxCur2Sample_valueChanged(int arg1);
  void on_pushButtonTerminalDebug_toggled(bool checked);
  void on_pushButtonTerminalClickToSend_toggled(bool checked);
  void on_pushButtonTerminalSelect_toggled(bool checked);
  void on_pushButtonTerminalInputCopy_clicked();
  void on_listWidgetTerminalCodeList_itemClicked(QListWidgetItem* item);
  void on_pushButtonFFT_toggled(bool checked);
  void on_pushButtonFFTImage_clicked();
  void on_pushButtonChangeXYColor_clicked();
  void on_comboBoxXYy_currentIndexChanged(int) { updateXY(); }
  void on_comboBoxXYx_currentIndexChanged(int) { updateXY(); }
  void on_pushButtonTerminalDebugSend_clicked();
  void on_textEditTerminalDebug_cursorPositionChanged();
  void on_myTerminal_cellClicked(int row, int column);
  void on_comboBoxFFTType_currentIndexChanged(int index);
  void on_checkBoxOpenGL_toggled(bool checked);
  void on_checkBoxMouseControls_toggled(bool checked);
  void on_lineEditVUnit_textChanged(const QString& arg1);
  void on_pushButtonClearReceivedList_3_clicked() { serialMonitor.clear(); }
  void on_pushButtonScrollDown_3_clicked();
  void on_checkBoxYCur1_stateChanged(int arg1);
  void on_checkBoxYCur2_stateChanged(int arg1);
  void on_doubleSpinBoxYCur2_valueChanged(double arg1);
  void on_doubleSpinBoxYCur1_valueChanged(double arg1);
  void on_checkBoxFFTCh1_toggled(bool checked);
  void on_checkBoxFFTCh2_toggled(bool checked);
  void on_comboBoxFFTStyle1_currentIndexChanged(int index) {ui->plotFFT->setStyle(0, index);}
  void on_comboBoxFFTStyle2_currentIndexChanged(int index) {ui->plotFFT->setStyle(1, index);}
  void on_checkBoxXYCur1_toggled(bool checked);
  void on_checkBoxXYCur2_toggled(bool checked);
  void on_doubleSpinBoxXYCurY1_valueChanged(double arg1);
  void on_doubleSpinBoxXYCurY2_valueChanged(double arg1);
  void on_doubleSpinBoxXYCurX1_valueChanged(double arg1);
  void on_doubleSpinBoxXYCurX2_valueChanged(double arg1);
  void on_pushButtonProtocolGuide_clicked();
  void on_pushButtonDolarNewline_toggled(bool checked);
  void on_pushButtonInterpolate_toggled(bool checked);

 public slots:
  void printMessage(QString messageHeader, QByteArray messageBody, int type, MessageTarget::enumMessageTarget target);
  void showPlotStatus(PlotStatus::enumPlotStatus type);
  void serialConnectResult(bool connected, QString message);
  void printToTerminal(QByteArray data) { ui->myTerminal->printToTerminal(data); }
  void serialFinishedWriting();
  void useSettings(QByteArray settings, MessageTarget::enumMessageTarget source);
  void printDeviceMessage(QByteArray message, bool warning, bool ended);
  void printSerialMonitor(QByteArray data) { serialMonitor.append(data); }
  void signalMeasurementsResult1(float period, float freq, float amp, float min, float max, float vrms, float dc, float fs, float rise, float fall, int samples);
  void signalMeasurementsResult2(float period, float freq, float amp, float min, float max, float vrms, float dc, float fs, float rise, float fall, int samples);
  void fftResult1(QSharedPointer<QCPGraphDataContainer> data);
  void fftResult2(QSharedPointer<QCPGraphDataContainer> data);
  void xyResult(QSharedPointer<QCPCurveDataContainer> data);
  void timeCursorMovedByMouse(Cursors::enumCursors cursor, int sample);
  void valueCursorMovedByMouse(Cursors::enumCursors cursor, double value);
  void cursorSetByMouse(int chid, Cursors::enumCursors cursor, int sample);
  void offsetChangedByMouse(int chid);
  void ch1WasUpdated(bool wasPoint, bool wasLogic,  HAxisType::enumHAxisType recommandedTimeBase);
  void moveTimeCursorXY(Cursors::enumCursors cursor, double pos);
  void moveValueCursorXY(Cursors::enumCursors cursor, double pos);
  void setCursorPosXY(Cursors::enumCursors cursor, double x, double y);
  void interpolationResult(int chID, QSharedPointer<QCPGraphDataContainer> dataOriginal, QSharedPointer<QCPGraphDataContainer> dataInterpolated, bool dataIsFromInterpolationBuffer);

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
  void setSerialMessageLevel(OutputLevel::enumOutputLevel level);
  void setManualMessageLevel(OutputLevel::enumOutputLevel level);
  void enableSerialMonitor(bool enabled);
  void setMathFirst(int math, int ch);
  void setMathSecond(int math, int ch);
  void clearMath(int math);
  void resetMath(int mathNumber, MathOperations::enumMathOperations mode, QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2, bool shouldIgnorePause = false);
  void requestXY(QSharedPointer<QCPGraphDataContainer> in1, QSharedPointer<QCPGraphDataContainer> in2);
  void requstMeasurements1(QSharedPointer<QCPGraphDataContainer> data);
  void requstMeasurements2(QSharedPointer<QCPGraphDataContainer> data);
  void requestFFT1(QSharedPointer<QCPGraphDataContainer> data, FFTType::enumFFTType type, FFTWindow::enumFFTWindow window, bool removeDC, int pWelchtimeDivisions, bool twosided, bool zerocenter, int minNFFT);
  void requestFFT2(QSharedPointer<QCPGraphDataContainer> data, FFTType::enumFFTType type, FFTWindow::enumFFTWindow window, bool removeDC, int pWelchtimeDivisions, bool twosided, bool zerocenter, int minNFFT);
  void setInterpolation(int chID, bool enabled);
  void interpolate(int chID, const QSharedPointer<QCPGraphDataContainer> data, QCPRange visibleRange, bool dataIsFromInterpolationBuffer);
};
#endif // MAINWINDOW_H
