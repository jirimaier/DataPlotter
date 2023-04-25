#ifndef FREQTIMEPLOTDIALOG_H
#define FREQTIMEPLOTDIALOG_H

#include "global.h"
#include <QDialog>

namespace Ui {
class FreqTimePlotDialog;
}

class FreqTimePlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FreqTimePlotDialog(QWidget *parent = nullptr);
    ~FreqTimePlotDialog();

    Ui::FreqTimePlotDialog *getUi() const;

    void retranslate();

private slots:
    void on_pushButtonPeakPlotClear_clicked();

    void on_pushButtonEXportFreqTimeCSV_clicked();

    void on_pushButtonSaveImage_clicked();

    void on_horizontalSliderGridFreqtimeH_valueChanged(int value);

    void on_horizontalSliderGridFreqtimeV_valueChanged(int value);

private:
    Ui::FreqTimePlotDialog *ui;

signals:
    void requestedCSVExport(int ch = EXPORT_FREQTIME);
};

#endif // FREQTIMEPLOTDIALOG_H
