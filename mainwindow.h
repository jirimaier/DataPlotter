#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColorDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTime>
#include <QTranslator>

#include "enums.h"
#include "plotting.h"
#include "serialHandler.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  SerialHandler *serial;
  Plotting *plotting;
  QTranslator translator;
  QByteArray buffer;
  QPixmap resume, pause, single;
  void init();
  void dataParser(QByteArray message);
  void loadIcons();
  void connectSignals();
  void useSettings(QString settings);
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
  void setBitMode(int bits, double valMin, double valMax, double timeStep, int numCh, int firstCh, bool continous);
  void on_pushButtonComRefresh_clicked();
  void on_pushButtonConnect_clicked();
  void on_pushButtonDisconnect_clicked();
  void on_pushButtonSendCommand_clicked();
  void changeLanguage();
  void clearBuffer() { buffer.clear(); };
  void showPlotStatus(int type);
  void setHDivLimits(double hRange);
  void setVDivLimits(double vRange);
  void serialErrorOccurred();
  void setCursorBounds(double xmin, double xmax, double ymin, double ymax, double xminfull, double xmaxfull, double yminfull, double ymaxfull);
  void setDataMode(int mode);
  void showErrorMessage(QByteArray message);
  void showProcessedCommand(QPair<bool, QByteArray> message);
  void printMessage(QByteArray data, bool urgent);
  void on_doubleSpinBoxChScale_valueChanged(double arg1);
  void on_dialChScale_valueChanged(int value);
  void on_doubleSpinBoxRangeVerticalDiv_valueChanged(double arg1);
};

#endif // MAINWINDOW_H
