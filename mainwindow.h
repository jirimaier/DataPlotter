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
  QTimer portsRefreshTimer, activeChRefreshTimer;
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
  PlotFrame_t plotFrame;
  void setGuiArrays();
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
  void updateChannelComboBox(QComboBox &combobox);
  bool colorUpdateNeeded = true;

private slots:
  void updateCursors();
  void setAdaptiveSpinBoxes();
  void updateDivs();
  void comRefresh();
  void rangeTypeChanged();
  void updateCursor(Cursors::enumerator cursor, int selectedChannel, unsigned int sample, double &time, double &value, QByteArray &timeStr, QByteArray &valueStr);

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
  void on_checkBoxChInvert_toggled(bool checked) { ui->plot->setChScale(ui->comboBoxSelectedChannel->currentIndex(), ui->doubleSpinBoxChScale->value() * (checked ? -1 : 1)); }
  void on_pushButtonSaveSettings_clicked();
  void on_pushButtonReset_clicked();
  void on_pushButtonAutoset_clicked();
  void on_pushButtonCSVXY_clicked() { exportCSV(false, XY_CHANNEL); }
  void on_comboBoxHAxisType_currentIndexChanged(int index);
  void on_pushButtonOpenHelp_clicked();
  void on_pushButtonPositive_clicked() { ui->dialVerticalCenter->setValue(ui->dialVerticalCenter->maximum()); }
  void on_pushButtonCenter_clicked();
  void on_pushButtonScrollDown_clicked();
  void on_checkBoxPlotOpenGL_toggled(bool checked);
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
  void on_checkBoxLog1_toggled(bool checked) { emit setChDigital(1, checked ? ui->spinBoxLog1source->value() : 0); }
  void on_checkBoxLog2_toggled(bool checked) { emit setChDigital(2, checked ? ui->spinBoxLog2source->value() : 0); }
  void on_checkBoxLog3_toggled(bool checked) { emit setChDigital(3, checked ? ui->spinBoxLog3source->value() : 0); }
  void on_checkBoxLog4_toggled(bool checked) { emit setChDigital(4, checked ? ui->spinBoxLog4source->value() : 0); }
  void on_spinBoxLog1bits_valueChanged(int arg1) { emit setLogicBits(1, arg1); }
  void on_spinBoxLog2bits_valueChanged(int arg1) { emit setLogicBits(2, arg1); }
  void on_spinBoxLog3bits_valueChanged(int arg1) { emit setLogicBits(3, arg1); }
  void on_spinBoxLog4bits_valueChanged(int arg1) { emit setLogicBits(4, arg1); }
  void on_spinBoxLog1source_valueChanged(int arg1) { emit setChDigital(1, ui->checkBoxLog1->isChecked() ? arg1 : 0); }
  void on_spinBoxLog2source_valueChanged(int arg1) { emit setChDigital(2, ui->checkBoxLog2->isChecked() ? arg1 : 0); }
  void on_spinBoxLog3source_valueChanged(int arg1) { emit setChDigital(3, ui->checkBoxLog3->isChecked() ? arg1 : 0); }
  void on_spinBoxLog4source_valueChanged(int arg1) { emit setChDigital(4, ui->checkBoxLog4->isChecked() ? arg1 : 0); }
  void on_pushButtonPlotImage_clicked();
  void on_pushButtonXYImage_clicked();
  void on_horizontalSliderTimeCur1_realValueChanged();
  void on_horizontalSliderTimeCur2_realValueChanged();
  void on_checkBoxCur1Visible_toggled(bool checked);
  void on_checkBoxCur2Visible_toggled(bool checked);
  void on_pushButtonChangeChColor_clicked();
  void on_pushButtonClearAll_clicked() { ui->plot->resetChannels(); }

  void on_checkBoxChInverted_toggled(bool checked);

  void on_labelLicense_linkActivated();

  void on_pushButtonHideCh_toggled(bool checked);

  void on_checkBoxMath1_toggled(bool checked);
  void on_checkBoxMath2_toggled(bool checked);
  void on_checkBoxMath3_toggled(bool checked);
  void on_checkBoxMath4_toggled(bool checked);

  void on_checkBoxXY_toggled(bool checked);

  void on_comboBoxMath1Op_currentIndexChanged(int index) { emit setMathMode(1, (MathOperations::enumetrator)index); }
  void on_comboBoxMath2Op_currentIndexChanged(int index) { emit setMathMode(2, (MathOperations::enumetrator)index); }
  void on_comboBoxMath3Op_currentIndexChanged(int index) { emit setMathMode(3, (MathOperations::enumetrator)index); }
  void on_comboBoxMath4Op_currentIndexChanged(int index) { emit setMathMode(4, (MathOperations::enumetrator)index); }

  void on_spinBoxMath1First_valueChanged(int arg1) { emit setMathFirst(1, ui->checkBoxMath1->isChecked() ? arg1 : 0); };
  void on_spinBoxMath2First_valueChanged(int arg1) { emit setMathFirst(2, ui->checkBoxMath2->isChecked() ? arg1 : 0); };
  void on_spinBoxMath3First_valueChanged(int arg1) { emit setMathFirst(3, ui->checkBoxMath3->isChecked() ? arg1 : 0); };
  void on_spinBoxMath4First_valueChanged(int arg1) { emit setMathFirst(4, ui->checkBoxMath4->isChecked() ? arg1 : 0); };

  void on_spinBoxMath1Second_valueChanged(int arg1) { emit setMathSecond(1, ui->checkBoxMath1->isChecked() ? arg1 : 0); };
  void on_spinBoxMath2Second_valueChanged(int arg1) { emit setMathSecond(2, ui->checkBoxMath2->isChecked() ? arg1 : 0); };
  void on_spinBoxMath3Second_valueChanged(int arg1) { emit setMathSecond(3, ui->checkBoxMath3->isChecked() ? arg1 : 0); };
  void on_spinBoxMath4Second_valueChanged(int arg1) { emit setMathSecond(4, ui->checkBoxMath4->isChecked() ? arg1 : 0); };

  void on_spinBoxXYFirst_valueChanged(int arg1) { emit setXYFirst(ui->checkBoxXY->isChecked() ? arg1 : 0); }
  void on_spinBoxXYSecond_valueChanged(int arg1) { emit setXYSecond(ui->checkBoxXY->isChecked() ? arg1 : 0); }

  void on_dial_valueChanged(int value);

public slots:
  void printMessage(QString messageHeader, QByteArray messageBody, int type, MessageTarget::enumerator target);
  void showPlotStatus(PlotStatus::enumerator type);
  void serialConnectResult(bool connected, QString message);
  void printToTerminal(QByteArray data) { ui->myTerminal->printToTerminal(data); }
  void serialFinishedWriting();
  void useSettings(QByteArray settings, MessageTarget::enumerator source);
  void printDeviceMessage(QByteArray messageBody, bool warning, bool ended);
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
  void requestMath(int mathNumber, bool isFirst, QSharedPointer<QCPGraphDataContainer> in);
  void requestXY(bool isFirst, QSharedPointer<QCPGraphDataContainer> in);
  void sendManualInput(QByteArray data);
  void parseError(QByteArray, int type = DataLineType::debugMessage);
  void setSerialMessageLevel(OutputLevel::enumerator level);
  void setManualMessageLevel(OutputLevel::enumerator level);
  void enableSerialMonitor(bool enabled);
  void setMathFirst(int math, int ch);
  void setMathSecond(int math, int ch);
  void setXYFirst(int ch);
  void setXYSecond(int ch);
  void setMathMode(int math, MathOperations::enumetrator mode);
};
#endif // MAINWINDOW_H
