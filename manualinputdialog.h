//  Copyright (C) 2020-2024  Jiří Maier

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

#ifndef MANUALINPUTDIALOG_H
#define MANUALINPUTDIALOG_H

#include "global.h"
#include "math/variableexpressionparser.h"
#include "qicon.h"
#include "qtimer.h"
#include <QDialog>

namespace Ui {
class ManualInputDialog;
}

class ManualInputDialog : public QDialog {
  Q_OBJECT

public:
  explicit ManualInputDialog(QWidget *parent = nullptr);
  ~ManualInputDialog();

  Ui::ManualInputDialog *getUi() const;

public slots:
  void stopAll();

private slots:
  void on_doubleSpinBoxRollingFreq_valueChanged(double arg1);
  void on_pushButtonRolling_clicked();
  void rollingDataTimerRoutine();
  void on_tableWidgetRollingSetup_cellChanged(int row, int column);
  void on_pushButtonRollingResetTime_clicked();

signals:
  void sendManualInput(QByteArray bytes);

private:
  Ui::ManualInputDialog *ui;
  QTimer rollingTimer;
  QIcon iconRun;
  QIcon iconPause;
  QList<VariableExpressionParser *> rollingChannelEvaluators;
  double rollingTimestamp = 0;
  QJSEngine rollingEngine;

  void setRollingExprRows(int rows);
  void initRollingTable();
};

#endif // MANUALINPUTDIALOG_H
