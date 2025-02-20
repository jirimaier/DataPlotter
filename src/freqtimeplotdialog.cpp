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

#include "freqtimeplotdialog.h"
#include "defaultpathmanager.h"
#include "ui_freqtimeplotdialog.h"

FreqTimePlotDialog::FreqTimePlotDialog(QWidget *parent) : QDialog(parent), ui(new Ui::FreqTimePlotDialog) {
  ui->setupUi(this);
  ui->plotPeak->setGridHintX(ui->horizontalSliderGridFreqtimeH->value());
  ui->plotPeak->setGridHintY(ui->horizontalSliderGridFreqtimeV->value());

  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
}

FreqTimePlotDialog::~FreqTimePlotDialog() { delete ui; }

Ui::FreqTimePlotDialog *FreqTimePlotDialog::getUi() const { return ui; }

void FreqTimePlotDialog::on_pushButtonPeakPlotClear_clicked() { ui->plotPeak->clear(); }

void FreqTimePlotDialog::on_pushButtonEXportFreqTimeCSV_clicked() { emit requestedCSVExport(); }

void FreqTimePlotDialog::on_pushButtonSaveImage_clicked() {
  QMessageBox msgBox(this);
  msgBox.setText(tr("Export FFT as image"));
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.setButtonText(QMessageBox::Yes, tr("To clipboard"));
  msgBox.setButtonText(QMessageBox::Ok, tr("To file"));
  int returnValue = msgBox.exec();

  if (returnValue == QMessageBox::Yes) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(ui->plotPeak->toPixmap().toImage());
  } else if (returnValue == QMessageBox::Ok) {
    QString defaultName = QString("freq_time.png");
    QString fileName = DefaultPathManager::getInstance().requestSaveFile(this, tr("Export FFT plot as image"), "path_export", defaultName, tr("PNG image (*.png);;Vector graphics (*.pdf);;JPEG image (*.jpg);;BMP image (*.bmp)"));
    if (fileName.isEmpty())
      return;

    bool isOK = false;

    if (fileName.right(3).toLower() == "jpg")
      isOK = ui->plotPeak->saveJpg(fileName);
    else if (fileName.right(3).toLower() == "jpeg")
      isOK = ui->plotPeak->saveJpg(fileName);
    else if (fileName.right(3).toLower() == "bmp")
      isOK = ui->plotPeak->saveBmp(fileName);
    else if (fileName.right(3).toLower() == "pdf")
      isOK = ui->plotPeak->savePdf(fileName, 0, 0, QCP::epNoCosmetic);
    else // png
      isOK = ui->plotPeak->savePng(fileName);

    if (!isOK) {
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cant write to file."));
      msgBox.setInformativeText(tr("This may be because file is opened in another program."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
    }
  }
}

void FreqTimePlotDialog::on_horizontalSliderGridFreqtimeH_valueChanged(int value) { ui->plotPeak->setGridHintX(value); }

void FreqTimePlotDialog::on_horizontalSliderGridFreqtimeV_valueChanged(int value) { ui->plotPeak->setGridHintY(value); }
