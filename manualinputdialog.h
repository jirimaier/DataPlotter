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

class ManualInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualInputDialog(QWidget *parent = nullptr);
    ~ManualInputDialog();

    Ui::ManualInputDialog *getUi() const;

private slots:
    void on_doubleSpinBoxRollingFreq_valueChanged(double arg1);
    void on_pushButtonRolling_clicked();
    void rollingDataTimerRoutine();
    void on_lineEditManualInput_returnPressed();
    void on_pushButtonSendManual_clicked() { on_lineEditManualInput_returnPressed(); }
    void on_pushButtonClearBuffer_2_clicked() { emit requestManualBufferClear(); }
    void on_pushButtonViewBuffer_2_clicked() { emit requestManualBufferShow(); }
    void on_pushButtonScrollDown_2_clicked();
    void on_tableWidgetRollingSetup_cellChanged(int row, int column);

signals:
    void requestManualBufferClear();
    void requestManualBufferShow();
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
};

#endif // MANUALINPUTDIALOG_H
