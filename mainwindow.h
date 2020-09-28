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

#include "plotdata.h"
#include "serialparser.h"
#include "settings.h"

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
  QTimer plotUpdateTimer;
  QTimer listUpdateTimer;
  QTimer portsRefreshTimer;
  Ui::MainWindow *ui;
  QList<QSerialPortInfo> portList;
  QStringList receivedListBuffer;
  void dataParser(QByteArray message);
  BinDataSettings_t binSettings;
  void connectSignals();
  // void useSettings(QString settings);
  int roundToStandardValue(double value);
  bool isStandardValue(double value);
  void updateChScale();
  void changeLanguage(QString code = "en");

private slots:
  void on_tabs_right_currentChanged(int index);
  void on_dialRollingRange_valueChanged(int value);
  void on_dialVerticalRange_valueChanged(int value);
  void on_pushButtonClearChannels_clicked();
  void on_pushButtonChannelColor_clicked();
  void on_spinBoxChannelSelect_valueChanged(int arg1);
  void on_doubleSpinBoxChOffset_valueChanged(double arg1);
  void on_dialOffset_valueChanged(int value);
  void on_comboBoxGraphStyle_currentIndexChanged(int index);
  void scrollBarCursor_valueChanged();
  void on_pushButtonConnect_clicked();
  void on_pushButtonDisconnect_clicked();
  void on_pushButtonSendCommand_clicked();
  void on_doubleSpinBoxChScale_valueChanged(double arg1);
  void on_dialChScale_valueChanged(int value);
  void on_pushButtonSelectedCSV_clicked();
  void on_dialZoom_valueChanged(int value);
  void on_comboBoxPlotRangeType_currentIndexChanged(int index);
  void on_radioButtonEn_toggled(bool checked);
  void on_radioButtonCz_toggled(bool checked);
  void updateInfo();
  void on_lineEditCommand_returnPressed();
  void updateDivs(double vertical, double horizontal);
  void on_checkBoxChInvert_toggled(bool checked);
  void on_horizontalSliderLineTimeout_valueChanged(int value);
  void on_pushButtonPrintBuffer_clicked();
  void on_pushButtonDataModeApply_clicked();
  void on_checkBoxModeManual_toggled(bool checked);
  void updatePlot();

  void on_comboBoxOutputLevel_currentIndexChanged(int index);

public slots:
  void comRefresh();
  void setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminfull, double xmaxfull, double yminfull, double ymaxfull);
  void changedDataMode(int mode);
  void changedBinSettings(BinDataSettings_t in_settings) { binSettings = in_settings; }
  void showProcessedCommand(QString message);
  void printMessage(QByteArray data, bool urgent);
  void showPlotStatus(int type);
  void addDataToPlot(int ch, QVector<double> *time, QVector<double> *value, bool continous);
  void serialConnectResult(bool connected, QString message);
  void printToTerminal(QByteArray data);
  void serialFinishedWriting();
  void bufferDebug(QByteArray data);

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
};
#endif // MAINWINDOW_H
