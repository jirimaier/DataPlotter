#ifndef MANUALINPUTDIALOG_H
#define MANUALINPUTDIALOG_H

#include <QDialog>
#include "global.h"
#include "math/variableexpressionparser.h"
#include "qicon.h"
#include "qtimer.h"

namespace Ui {
class ManualInputDialog;
}

class ManualInputDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ManualInputDialog(QWidget* parent = nullptr);
  ~ManualInputDialog();

  Ui::ManualInputDialog* getUi() const;

 private slots:
  void on_doubleSpinBoxRollingFreq_valueChanged(double arg1);
  void on_pushButtonRolling_clicked();
  void rollingDataTimerRoutine();
  void on_tableWidgetRollingSetup_cellChanged(int row, int column);
  void on_pushButtonRollingResetTime_clicked();

 signals:
  void sendManualInput(QByteArray bytes);

 private:
  Ui::ManualInputDialog* ui;
  QTimer rollingTimer;
  QIcon iconRun;
  QIcon iconPause;
  QList<VariableExpressionParser*> rollingChannelEvaluators;
  double rollingTimestamp = 0;
  QJSEngine rollingEngine;

  void setRollingExprRows(int rows);
  void initRollingTable();
};

#endif  // MANUALINPUTDIALOG_H
