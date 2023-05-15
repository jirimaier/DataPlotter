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
#include <QElapsedTimer>
#include <QMainWindow>
#include <QMessageBox>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSerialPortInfo>
#include <QTime>
#include <QTimer>
#include <QTranslator>
#include <QtCore>

#include "communication/filesender.h"
#include "communication/serialreader.h"
#include "communication/serialsettingsdialog.h"
#include "developeroptions.h"
#include "freqtimeplotdialog.h"
#include "global.h"
#include "manualinputdialog.h"
#include "math/averager.h"
#include "math/plotmath.h"
#include "qml/ansiterminalmodel.h"
#include "qml/messagemodel.h"
#include "qml/qmlterminalinterface.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  void init(QTranslator* translator,
            const PlotData* plotData,
            const PlotMath* plotMath,
            const SerialReader* serialReader,
            const Averager* avg);
  ~MainWindow();

 private:
  Ui::MainWindow* ui;
  QmlTerminalInterface* qmlTerminalInterface;
  SerialSettingsDialog* serialSettingsDialog;
  DeveloperOptions* developerOptions;
  FreqTimePlotDialog* freqTimePlotDialog;
  ManualInputDialog* manualInputDialog;
  QTranslator* translator;
  QTimer portsRefreshTimer, activeChRefreshTimer, xyTimer,
      cursorRangeUpdateTimer, measureRefreshTimer1, measureRefreshTimer2,
      fftTimer1, fftTimer2, serialMonitorTimer, consoleTimer,
      interpolationTimer, triggerLineTimer;
  QList<QSerialPortInfo> portList;
  FileSender fileSender;
  QPalette darkPalette, lightPalette;
  QPushButton* mathEn[3];
  QComboBox* mathFirst[3];
  QComboBox* mathSecond[3];
  QComboBox* mathOp[3];
  QDoubleSpinBox* mathScalarFirst[3];
  QDoubleSpinBox* mathScalarSecond[3];
  QIcon iconRun, iconPause, iconHidden, iconVisible, iconConnected,
      iconNotConnected, iconCross, iconAbsoluteCursor;
  QByteArray serialMonitor;
  QStringList consoleBuffer;
  QString configFilePath;
  int averagerCounts[ANALOG_COUNT];
  QStringList autoConnectPortNames;
  QString attemptReconnectPort;
  ChannelExpectedRange channelExpectedRanges[ANALOG_COUNT + MATH_COUNT];
  QFile recordingOfMeasurements1, recordingOfMeasurements2;
  QElapsedTimer uptime;
  QTemporaryFile currentQmlFile;
  QString pendingMessagePart;
  bool pendingMessageType;
  MessageModel messageModel;
  AnsiTerminalModel ansiTerminalModel;
  bool autoAutosetPending = false;
  bool colorUpdateNeeded = true;
  bool currentThemeDark = false;
  QMap<QString, QPair<QWidget*, bool>> setables;
  int interpolationsRunning = 0;
  bool lastUpdateWasLogic = false;
  bool lastUpdateWasPoint = false;
  int lastSelectedChannel = 1;
  bool pendingDeviceMessage = false;
  HAxisType::enumHAxisType recommandedAxisType = HAxisType::normal;
  bool recommendOpenGL = true;
  bool timeUseUnits = true;
  bool valuesUseUnits = true;
  bool freqUseUnits = true;
  int dataUpdates = 0;

 private:
  void setComboboxItemVisible(QComboBox& comboBox, int index, bool visible);
  void setChStyleSelection(GraphType::enumGraphType type);
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
  void exportCSV(int ch);
  void fillChannelSelect();
  void updateChannelComboBox(QComboBox& combobox, int numberOfExcludedAtEnd);
  void updateSelectedChannel(int arg1);
  void updateMathNow(int number);
  void updateXY();
  void setCursorsVisibility(Cursors::enumCursors cursor,
                            int graph,
                            int timeCurState,
                            int valueCurState);
  void updateXYCursorsCalculations();
  void updateCursorMeasurementsText();
  void initQmlTerminal();
  void resetQmlTerminal();
  void saveDefaultSettings();
  void closeEvent(QCloseEvent* event);

 private slots:
  void updateCursors();
  void setAdaptiveSpinBoxes();
  void updateDivs();
  void comRefresh();
  void plotLayoutChanged();
  void updateCursor(Cursors::enumCursors cursor,
                    int selectedChannel,
                    unsigned int sample,
                    double& time,
                    double& value,
                    QByteArray& timeStr,
                    QByteArray& valueStr,
                    bool useValueCursor);
  void updateCursorRange();
  void updateMeasurements1();
  void updateMeasurements2();
  void updateFFT1();
  void updateFFT2();
  void updateInterpolation();
  void updateSerialMonitor();
  void horizontalSliderTimeCur1_realValueChanged(int arg1);
  void horizontalSliderTimeCur2_realValueChanged(int arg1);
  void updateUsedChannels();
  void turnOffTriggerLine() { ui->plot->setTriggerLineVisible(false); }
  void updateConsole();

 private slots:
  void pushButtonLoadFile_clicked_new();
  void pushButtonDefaults_clicked_new();
  void pushButtonSaveSettings_clicked();
  void pushButtonClearAll_clicked();
  void checkBoxTriggerLineEn_stateChanged(int arg1);
  void pushButtonClearGraph_clicked();
  void checkBoxEchoReply_toggled(bool checked);
  void checkBoxMouseControls_toggled_new(bool checked);

 private slots:  // Autoconnect slots
  void on_dialRollingRange_realValueChanged(double value) {
    ui->doubleSpinBoxRangeHorizontal->setValue(value);
  }
  void on_dialVerticalRange_realValueChanged(double value) {
    ui->doubleSpinBoxRangeVerticalRange->setValue(value);
  }
  void on_doubleSpinBoxChOffset_valueChanged(double arg1);
  void on_comboBoxGraphStyle_currentIndexChanged(int index);
  void on_pushButtonConnect_clicked();
  void on_doubleSpinBoxChScale_valueChanged(double arg1);
  void on_pushButtonSelectedCSV_clicked() {
    exportCSV(ui->comboBoxSelectedChannel->currentIndex());
  }
  void on_radioButtonEn_toggled(bool checked);
  void on_radioButtonCz_toggled(bool checked);
  void on_pushButtonAllCSV_clicked() { exportCSV(EXPORT_ALL); }
  void on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1);
  void on_pushButtonReset_clicked();
  void on_pushButtonAutoset_clicked();
  void on_pushButtonCSVXY_clicked() { exportCSV(EXPORT_XY); }
  void on_pushButtonCSVFFT_clicked() { exportCSV(EXPORT_FFT); }
  void on_comboBoxHAxisType_currentIndexChanged(int index);
  void on_pushButtonOpenHelpCZ_clicked();
  void on_pushButtonScrollDown_clicked();
  void on_comboBoxOutputLevel_currentIndexChanged(int index);
  void on_comboBoxSelectedChannel_currentIndexChanged(int index);
  void on_pushButtonResetChannels_clicked();
  void on_pushButtonLog1_toggled(bool checked) {
    emit setChDigital(1, checked ? ui->comboBoxLogic1->currentIndex() + 1 : 0);
  }
  void on_pushButtonLog2_toggled(bool checked) {
    emit setChDigital(2, checked ? ui->comboBoxLogic2->currentIndex() + 1 : 0);
  }
  void on_spinBoxLog1bits_valueChanged(int arg1) { emit setLogicBits(1, arg1); }
  void on_spinBoxLog2bits_valueChanged(int arg1) { emit setLogicBits(2, arg1); }
  void on_comboBoxLogic1_currentIndexChanged(int index) {
    emit setChDigital(1, ui->pushButtonLog1->isChecked() ? index + 1 : 0);
  }
  void on_comboBoxLogic2_currentIndexChanged(int index) {
    emit setChDigital(2, ui->pushButtonLog2->isChecked() ? index + 1 : 0);
  }
  void on_pushButtonPlotImage_clicked();
  void on_pushButtonXYImage_clicked();
  void on_checkBoxCur1Visible_stateChanged(int arg1);
  void on_checkBoxCur2Visible_stateChanged(int arg1);
  void on_pushButtonChangeChColor_clicked();
  void on_pushButtonInvert_toggled(bool checked);
  void on_pushButtonHideCh_toggled(bool checked);
  void on_pushButtonMath1_toggled(bool) { updateMathNow(1); }
  void on_pushButtonMath2_toggled(bool) { updateMathNow(2); }
  void on_pushButtonMath3_toggled(bool) { updateMathNow(3); }
  void on_comboBoxMath1Op_currentIndexChanged(int) { updateMathNow(1); }
  void on_comboBoxMath2Op_currentIndexChanged(int) { updateMathNow(2); }
  void on_comboBoxMath3Op_currentIndexChanged(int) { updateMathNow(3); }
  void on_comboBoxMathFirst1_currentIndexChanged(int) { updateMathNow(1); }
  void on_comboBoxMathFirst2_currentIndexChanged(int) { updateMathNow(2); }
  void on_comboBoxMathFirst3_currentIndexChanged(int) { updateMathNow(3); }
  void on_comboBoxMathSecond1_currentIndexChanged(int) { updateMathNow(1); }
  void on_comboBoxMathSecond2_currentIndexChanged(int) { updateMathNow(2); }
  void on_comboBoxMathSecond3_currentIndexChanged(int) { updateMathNow(3); }
  void on_doubleSpinBoxMathScalar1_1_valueChanged(double) { updateMathNow(1); }
  void on_doubleSpinBoxMathScalar1_2_valueChanged(double) { updateMathNow(2); }
  void on_doubleSpinBoxMathScalar1_3_valueChanged(double) { updateMathNow(3); }
  void on_doubleSpinBoxMathScalar2_1_valueChanged(double) { updateMathNow(1); }
  void on_doubleSpinBoxMathScalar2_2_valueChanged(double) { updateMathNow(2); }
  void on_doubleSpinBoxMathScalar2_3_valueChanged(double) { updateMathNow(3); }
  void on_horizontalSliderXYGrid_valueChanged(int value);
  void on_pushButtonXY_toggled(bool checked);
  void on_comboBoxCursor1Channel_currentIndexChanged(int index);
  void on_comboBoxCursor2Channel_currentIndexChanged(int index);
  void on_pushButtonFFT_toggled(bool checked);
  void on_pushButtonFFTImage_clicked();
  void on_pushButtonChangeXYColor_clicked();
  void on_comboBoxXYy_currentIndexChanged(int) { updateXY(); }
  void on_comboBoxXYx_currentIndexChanged(int) { updateXY(); }
  void on_comboBoxFFTType_currentIndexChanged(int index);
  void on_checkBoxOpenGL_toggled(bool checked);
  void on_lineEditVUnit_textChanged(const QString& arg1);
  void on_pushButtonClearReceivedList_3_clicked() { serialMonitor.clear(); }
  void on_pushButtonScrollDown_3_clicked();
  void on_checkBoxYCur1_stateChanged(int arg1);
  void on_checkBoxYCur2_stateChanged(int arg1);
  void on_doubleSpinBoxYCur2_valueChanged(double arg1);
  void on_doubleSpinBoxYCur1_valueChanged(double arg1);
  void on_doubleSpinBoxXCur2_valueChanged(double arg1);
  void on_doubleSpinBoxXCur1_valueChanged(double arg1);
  void on_checkBoxFFTCh1_toggled(bool checked);
  void on_checkBoxFFTCh2_toggled(bool checked);
  void on_checkBoxXYCur1_toggled(bool checked);
  void on_checkBoxXYCur2_toggled(bool checked);
  void on_doubleSpinBoxXYCurY1_valueChanged(double arg1);
  void on_doubleSpinBoxXYCurY2_valueChanged(double arg1);
  void on_doubleSpinBoxXYCurX1_valueChanged(double arg1);
  void on_doubleSpinBoxXYCurX2_valueChanged(double arg1);
  void on_pushButtonDolarNewline_toggled(bool checked);
  void on_pushButtonInterpolate_toggled(bool checked);
  void on_pushButtonSerialSetting_clicked();
  void on_pushButtonSerialMoreInfo_clicked();
  void on_comboBoxBaud_editTextChanged(const QString& arg1);
  void on_pushButtonHideCur1_clicked();
  void on_pushButtonHideCur2_clicked();
  void on_pushButtonAvg_toggled(bool checked);
  void on_spinBoxAvg_valueChanged(int arg1);
  void on_radioButtonAverageIndividual_toggled(bool checked);
  void on_comboBoxAvgIndividualCh_currentIndexChanged(int arg1);
  void on_lineEditHUnit_textChanged(const QString& arg1);
  void on_pushButtonProtocolGuideCZ_clicked();
  void on_pushButtonProtocolGuideEN_clicked();
  void on_pushButtonIntroVideoCZ_clicked();
  void on_labelLogo_clicked();
  void on_comboBoxFIR_currentIndexChanged(int index);
  void on_comboBoxBaud_currentTextChanged(const QString& arg1);
  void on_pushButtonRecordMeasurements1_clicked();
  void on_pushButtonRecordMeasurements2_clicked();
  void on_radioButtonDark_toggled(bool checked);
  void on_tabs_right_currentChanged(int index);
  void on_pushButtonRangeFit_clicked();
  void on_listWidgetCom_currentItemChanged(QListWidgetItem* current,
                                           QListWidgetItem* previous);
  void on_pushButtonDevOptions_clicked() { developerOptions->show(); }
  void on_pushButtonSetPositive_clicked();
  void on_pushButtonSetCenter_clicked();
  void on_pushButtonSetNegative_clicked();
  void on_pushButtonFFT_FS_toggled(bool checked);
  void on_pushButtonXY_FS_toggled(bool checked);
  void on_pushButtonFvsT_clicked();
  void on_pushButtonShowManualInput_clicked() { manualInputDialog->show(); }
  void on_pushButtonSerialMonitor_toggled(bool checked);
  void on_comboBoxXYStyle_currentIndexChanged(int index);
  void on_comboBoxFFTStyle1_currentIndexChanged(int index);
  void on_comboBoxFFTStyle2_currentIndexChanged(int index);
  void on_pushButtonModeRolling_toggled(bool checked);
  void on_pushButtonModeFull_toggled(bool checked);
  void on_doubleSpinBoxRangeHorizontal_valueChanged(double arg1);

  void on_doubleSpinBoxViewCenter_valueChanged(double arg1);

  void on_horizontalScrollBarHorizontal_sliderMoved(int position);
  
  void on_dialZoom_valueChanged(int position);

 public slots:
  void printMessage(QString messageHeader,
                    QByteArray messageBody,
                    int type,
                    MessageTarget::enumMessageTarget target);
  void showPlotStatus(PlotStatus::enumPlotStatus type);
  void serialConnectResult(bool connected, QString message, QString details);
  void printToTerminal(QByteArray data) {
    ansiTerminalModel.printToTerminal(data);
  }
  void useSettings(QByteArray settings,
                   MessageTarget::enumMessageTarget source);
  void fileRequest(QByteArray message, MessageTarget::enumMessageTarget source);
  void printDeviceMessage(QByteArray message, bool warning, bool ended);
  void printSerialMonitor(QByteArray data);
  void signalMeasurementsResult1(double period,
                                 double freq,
                                 double amp,
                                 double min,
                                 double max,
                                 double vrms,
                                 double dc,
                                 double fs,
                                 double rise,
                                 double fall,
                                 int samples);
  void signalMeasurementsResult2(double period,
                                 double freq,
                                 double amp,
                                 double min,
                                 double max,
                                 double vrms,
                                 double dc,
                                 double fs,
                                 double rise,
                                 double fall,
                                 int samples);
  void fftResult1(QSharedPointer<QCPGraphDataContainer> data);
  void fftResult2(QSharedPointer<QCPGraphDataContainer> data);
  void xyResult(QSharedPointer<QCPCurveDataContainer> data);
  void timeCursorMovedByMouse(Cursors::enumCursors cursor,
                              int sample,
                              double value);
  void valueCursorMovedByMouse(Cursors::enumCursors cursor, double value);
  void cursorSetByMouse(int chid, Cursors::enumCursors cursor, int sample);
  void offsetChangedByMouse(int chid);
  void plotRecomendationChanged(bool wasPoint,
                                HAxisType::enumHAxisType recommandedTimeBase);
  void moveTimeCursorXY(Cursors::enumCursors cursor, double pos);
  void moveValueCursorXY(Cursors::enumCursors cursor, double pos);
  void setCursorPosXY(Cursors::enumCursors cursor, double x, double y);
  void interpolationResult(
      int chID,
      QSharedPointer<QCPGraphDataContainer> dataOriginal,
      QSharedPointer<QCPGraphDataContainer> dataInterpolated,
      bool dataIsFromInterpolationBuffer);
  void deviceError(QByteArray message, MessageTarget::enumMessageTarget source);
  void setExpectedRange(int chID, bool known, double min, double max) {
    channelExpectedRanges[chID].maximum = max;
    channelExpectedRanges[chID].minimum = min;
    channelExpectedRanges[chID].unknown = !known;
  }
  void loadCompressedQml(QByteArray data);
  void saveToFile(QByteArray data);
  void qmlDirectInput(QByteArray data);
  void setQmlProperty(QByteArray data);
  void loadQmlFile(QUrl url);
  void dataRateUpdate(int dataUpdates);
  void mainPlotHRangeChanged(QCPRange range);
  void mainPlotVRangeChanged(QCPRange range);

 signals:
  void setChDigital(int chid, int target);
  void setLogicBits(int target, int bits);
  void requestSerialBufferClear();
  void requestSerialBufferShow();
  void requestManualBufferClear();
  void requestManualBufferShow();
  void beginSerialConnection(QString port,
                             int baud,
                             QSerialPort::DataBits dataBits,
                             QSerialPort::Parity parity,
                             QSerialPort::StopBits stopBits,
                             QSerialPort::FlowControl flowControll);
  void toggleSerialConnection(QString port,
                              int baud,
                              QSerialPort::DataBits dataBits,
                              QSerialPort::Parity parity,
                              QSerialPort::StopBits stopBits,
                              QSerialPort::FlowControl flowControll);
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
  void resetMath(int mathNumber,
                 MathOperations::enumMathOperations mode,
                 QSharedPointer<QCPGraphDataContainer> in1,
                 QSharedPointer<QCPGraphDataContainer> in2,
                 bool firstIsConst,
                 bool secondIsConst,
                 double scaleFirst,
                 double scaleSecond);
  void requestXY(QSharedPointer<QCPGraphDataContainer> in1,
                 QSharedPointer<QCPGraphDataContainer> in2,
                 bool removeDC);
  void requstMeasurements1(QSharedPointer<QCPGraphDataContainer> data);
  void requstMeasurements2(QSharedPointer<QCPGraphDataContainer> data);
  void requestFFT1(QSharedPointer<QCPGraphDataContainer> data,
                   FFTType::enumFFTType type,
                   FFTWindow::enumFFTWindow window,
                   bool removeDC,
                   int pWelchtimeDivisions,
                   bool twosided,
                   bool zerocenter,
                   int minNFFT);
  void requestFFT2(QSharedPointer<QCPGraphDataContainer> data,
                   FFTType::enumFFTType type,
                   FFTWindow::enumFFTWindow window,
                   bool removeDC,
                   int pWelchtimeDivisions,
                   bool twosided,
                   bool zerocenter,
                   int minNFFT);
  void setInterpolation(int chID, bool enabled);
  void interpolate(int chID,
                   const QSharedPointer<QCPGraphDataContainer> data,
                   QCPRange visibleRange,
                   bool dataIsFromInterpolationBuffer);
  void resetAverager();
  void setAverager(bool enabled);
  void setAveragerCount(int chID, int count);
  void setInterpolationFilter(QString filename, int upsampling);
  void replyEcho(bool enabled);
  void changeSerialBaud(qint32 baud);
};
#endif  // MAINWINDOW_H
