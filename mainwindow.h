#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QMainWindow>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QTime>
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
  Ui::MainWindow *ui;
  QTranslator translator;
  QByteArray buffer;
  QStringList portList;
  void dataParser(QByteArray message);
  void connectSignals();
  // void useSettings(QString settings);
  int roundToStandardValue(double value);
  bool isStandardValue(double value);
  void updateChScale();

private slots:
  void on_tabs_right_currentChanged(int index);
  void on_dialRollingRange_valueChanged(int value);
  void on_dialVerticalRange_valueChanged(int value);
  void on_pushButtonClearChannels_clicked();
  void on_pushButtonChannelColor_clicked();
  void on_spinBoxChannelSelect_valueChanged(int arg1);
  void on_doubleSpinBoxChOffset_valueChanged(double arg1);
  void on_dialOffset_valueChanged(int value);
  void on_dialVerticalDiv_valueChanged(int value);
  void on_dialhorizontalDiv_valueChanged(int value);
  void on_comboBoxGraphStyle_currentIndexChanged(int index);
  void scrollBarCursor_valueChanged();
  void on_spinBoxDataBinaryBits_valueChanged(int arg1);
  void on_spinBoxBinaryDataNumCh_valueChanged(int arg1);
  void on_pushButtonComRefresh_clicked();
  void on_pushButtonConnect_clicked();
  void on_pushButtonDisconnect_clicked();
  void on_pushButtonSendCommand_clicked();
  void on_doubleSpinBoxChScale_valueChanged(double arg1);
  void on_dialChScale_valueChanged(int value);
  void on_doubleSpinBoxRangeVerticalDiv_valueChanged(double arg1);
  void on_pushButtonSelectedCSV_clicked();
  void on_doubleSpinBoxBinarydataMax_valueChanged(double arg1) { emit setBinMax(arg1); }
  void on_doubleSpinBoxBinaryDataMin_valueChanged(double arg1) { emit setBinMin(arg1); }
  void on_doubleSpinBoxBinaryTimestep_valueChanged(double arg1) { emit setBinStep(arg1); }
  void on_spinBoxBinaryDataFirstCh_valueChanged(int arg1) { emit setBinFCh(arg1); }
  void on_checkBoxBinContinuous_toggled(bool checked) { emit setBinCont(checked); }
  void on_dialZoom_valueChanged(int value);
  void on_comboBoxPlotRangeType_currentIndexChanged(int index);
  void on_listWidgetDataMode_currentRowChanged(int currentRow);

public slots:
  void setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminfull, double xmaxfull, double yminfull, double ymaxfull);
  void changedDataMode(int mode);
  void changeBinSettings(binDataSettings_t in_settings);
  void showProcessedCommand(QPair<bool, QByteArray> message);
  void printMessage(QByteArray data, bool urgent);
  void changeLanguage();
  void showPlotStatus(int type);
  void setHDivLimits(double hRange);
  void setVDivLimits(double vRange);
  void addDataToPlot(QVector<Channel *> channels);
  void serialConnectResult(bool connected, QString message);

signals:
  void connectSerial(QString port, int baud);
  void disconnectSerial();
  void setBinBits(int value);
  void setBinMin(double value);
  void setBinMax(double value);
  void setBinStep(double value);
  void setBinNCh(int value);
  void setBinFCh(int value);
  void setBinCont(bool value);
  void setMode(int mode);
};
#endif // MAINWINDOW_H
