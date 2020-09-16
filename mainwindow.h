#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColorDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTime>
#include <QTranslator>

#include "enums.h"
#include "plotdata.h"
#include "serialhandler.h"
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
  void init(Settings *in_settings, SerialHandler *in_serial, PlotData *in_plotData);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  Settings *settings;
  SerialHandler *serial;
  PlotData *plotData;
  QTranslator translator;
  QByteArray buffer;

  void dataParser(QByteArray message);
  void connectSignals();
  // void useSettings(QString settings);
  int roundToStandardValue(double value);
  bool isStandardValue(double value);
  void updateChScale();

private slots:
  void on_sliderRefreshRate_valueChanged(int value);
  void on_tabs_right_currentChanged(int index);
  void radioButtonRangeType_toggled(bool checked);
  void on_dialRollingRange_valueChanged(int value);
  void on_dialVerticalRange_valueChanged(int value);
  void on_pushButtonClearChannels_clicked();
  void on_pushButtonChannelColor_clicked();
  void on_spinBoxChannelSelect_valueChanged(int arg1);
  void on_doubleSpinBoxChOffset_valueChanged(double arg1);
  void on_dialOffset_valueChanged(int value);
  void on_dialVerticalDiv_valueChanged(int value);
  void on_pushButtonVerticalZero_clicked();
  void on_dialhorizontalDiv_valueChanged(int value);
  void on_comboBoxGraphStyle_currentIndexChanged(int index);
  void scrollBarCursor_valueChanged();
  void on_spinBoxCursorCh_valueChanged(int arg1);
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
  void on_doubleSpinBoxBinarydataMax_valueChanged(double arg1) { settings->binDataSettings.valueMax = arg1; }
  void on_doubleSpinBoxBinaryDataMin_valueChanged(double arg1) { settings->binDataSettings.valueMin = arg1; }
  void on_doubleSpinBoxBinaryTimestep_valueChanged(double arg1) { settings->binDataSettings.timeStep = arg1; }
  void on_spinBoxBinaryDataFirstCh_valueChanged(int arg1) { settings->binDataSettings.firstCh = arg1; }
  void on_checkBoxBinContinuous_toggled(bool checked) { settings->binDataSettings.continuous = checked; }
  void on_dialZoom_valueChanged(int value);
  void on_horizontalScrollBarHorizontal_valueChanged(int value) { settings->plotSettings.horizontalPos = value; }
  void on_doubleSpinBoxRangeHorizontal_valueChanged(double arg1) { settings->plotSettings.rollingRange = arg1; }
  void on_doubleSpinBoxRangeVerticalRange_valueChanged(double arg1) { settings->plotSettings.verticalRange = arg1; }
  void on_verticalScrollBarVerticalCenter_valueChanged(int value) { settings->plotSettings.verticalCenter = value; }
  void on_doubleSpinBoxRangeHorizontalDiv_valueChanged(double arg1);

public slots:
  void serialErrorOccurred();
  void setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminfull, double xmaxfull, double yminfull, double ymaxfull);
  void setDataMode(int mode);
  void changeBinSettings(Settings::binDataSettings_t in_settings);
  void showErrorMessage(QByteArray message);
  void showProcessedCommand(QPair<bool, QByteArray> message);
  void printMessage(QByteArray data, bool urgent);
  void changeLanguage();
  void showPlotStatus(int type);
  void setHDivLimits(double hRange);
  void setVDivLimits(double vRange);
};
#endif // MAINWINDOW_H
