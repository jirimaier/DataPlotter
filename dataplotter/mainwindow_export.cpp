//  Copyright (C) 2020-2021  Jiří Maier

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

#include "mainwindow.h"

void MainWindow::exportCSV(int ch) {
  QString name = "";
  if (ch == EXPORT_ALL)
    name = tr("all");
  else {
    if (ch == EXPORT_FFT)
      name = "fft";
    else if (ch == EXPORT_XY)
      name = "xy";
    else if (ch == ANALOG_COUNT + MATH_COUNT + LOGIC_GROUPS - 1)
      name = tr("logic");
    else if (ch >= ANALOG_COUNT + MATH_COUNT)
      name = tr("logic %1").arg(ch - ANALOG_COUNT - MATH_COUNT + 1);
    else {
      name = getChName(ch);
    }
  }

  QByteArray data;
  char decimal = ui->radioButtonCSVDot->isChecked() ? '.' : ',';
  char separator = ui->radioButtonCSVDot->isChecked() ? ',' : ';';
  if (ch == EXPORT_ALL)
    data = (ui->plot->exportAllCSV(separator, decimal, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVVRO->isChecked(), ui->checkBoxCSVIncludeHidden->isChecked()));
  else {
    if (ch >= ANALOG_COUNT + MATH_COUNT)
      data = (ui->plot->exportLogicCSV(separator, decimal, ch - ANALOG_COUNT - MATH_COUNT, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVVRO->isChecked()));
    else if (ch == EXPORT_XY)
      data = (ui->plotxy->exportCSV(separator, decimal, ui->spinBoxCSVPrecision->value()));
    else if (ch == EXPORT_FFT)
      data = (ui->plotFFT->exportCSV(separator, decimal, ui->spinBoxCSVPrecision->value()));
    else
      data = (ui->plot->exportChannelCSV(separator, decimal, ch, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVVRO->isChecked()));
  }

  if (data.isEmpty()) {
    QMessageBox msgBox(this);
    msgBox.setText(tr("No data to export"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  QMessageBox msgBox(this);
  msgBox.setText(tr("Export %1 as table").arg(name));
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.setButtonText(QMessageBox::Yes, tr("To clipboard"));
  msgBox.setButtonText(QMessageBox::Ok, tr("To CSV file"));
  int returnValue = msgBox.exec();
  if (returnValue == QMessageBox::Cancel) // Okno bylo zavřeno křížkem
    return;
  bool toClipboard = (returnValue == QMessageBox::Yes);

  if (toClipboard) {
    data.replace(separator, '\t'); // V Excelovském formátu tabulky jsou hodnoty oddělené tabulátory
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(data);
  } else {
    QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/export/%1.csv").arg(name);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export %1").arg(name), defaultName, tr("Comma separated values (*.csv)"));
    if (fileName.isEmpty())
      return;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
      file.write(data);
      file.close();
    } else {
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cant write to file."));
      msgBox.setInformativeText(tr("This may be because file is opened in another program."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
    }
  }
}

void MainWindow::on_pushButtonPlotImage_clicked() {
  QMessageBox msgBox(this);
  msgBox.setText(tr("Export main plot as image"));
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.setButtonText(QMessageBox::Yes, tr("To clipboard"));
  msgBox.setButtonText(QMessageBox::Ok, tr("To file"));
  int returnValue = msgBox.exec();

  if (returnValue == QMessageBox::Yes) {
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setImage(ui->plot->toPixmap().toImage());
  } else if (returnValue == QMessageBox::Ok) {
    QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/export/plot.png");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export main plot as image"), defaultName, tr("PNG image (*.png);;Vector graphics (*.pdf);;JPEG image (*.jpg);;BMP image (*.bmp)"));
    if (fileName.isEmpty())
      return;

    bool isOK = false;

    if (fileName.right(3).toLower() == "jpg")
      isOK = ui->plot->saveJpg(fileName);
    else if (fileName.right(3).toLower() == "jpeg")
      isOK = ui->plot->saveJpg(fileName);
    else if (fileName.right(3).toLower() == "bmp")
      isOK = ui->plot->saveBmp(fileName);
    else if (fileName.right(3).toLower() == "pdf")
      isOK = ui->plot->savePdf(fileName, 0, 0, QCP::epNoCosmetic);
    else // png
      isOK = ui->plot->savePng(fileName);

    if (!isOK) {
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cant write to file."));
      msgBox.setInformativeText(tr("This may be because file is opened in another program."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
    }
  }
}

void MainWindow::on_pushButtonXYImage_clicked() {
  QMessageBox msgBox(this);
  msgBox.setText(tr("Export XY as image"));
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.setButtonText(QMessageBox::Yes, tr("To clipboard"));
  msgBox.setButtonText(QMessageBox::Ok, tr("To file"));
  int returnValue = msgBox.exec();

  if (returnValue == QMessageBox::Yes) {
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setImage(ui->plotxy->toPixmap().toImage());
  } else if (returnValue == QMessageBox::Ok) {
    QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/export/xy.png");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export XY plot as image"), defaultName,  tr("PNG image (*.png);;Vector graphics (*.pdf);;JPEG image (*.jpg);;BMP image (*.bmp)"));
    if (fileName.isEmpty())
      return;

    bool isOK = false;

    if (fileName.right(3).toLower() == "jpg")
      isOK = ui->plotxy->saveJpg(fileName);
    else if (fileName.right(3).toLower() == "jpeg")
      isOK = ui->plotxy->saveJpg(fileName);
    else if (fileName.right(3).toLower() == "bmp")
      isOK = ui->plotxy->saveBmp(fileName);
    else if (fileName.right(3).toLower() == "pdf")
      isOK = ui->plotxy->savePdf(fileName, 0, 0, QCP::epNoCosmetic);
    else // png
      isOK = ui->plotxy->savePng(fileName);

    if (!isOK) {
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cant write to file."));
      msgBox.setInformativeText(tr("This may be because file is opened in another program."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
    }
  }
}

void MainWindow::on_pushButtonFFTImage_clicked() {
  QMessageBox msgBox(this);
  msgBox.setText(tr("Export FFT as image"));
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.setButtonText(QMessageBox::Yes, tr("To clipboard"));
  msgBox.setButtonText(QMessageBox::Ok, tr("To file"));
  int returnValue = msgBox.exec();

  if (returnValue == QMessageBox::Yes) {
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setImage(ui->plotFFT->toPixmap().toImage());
  } else if (returnValue == QMessageBox::Ok) {
    QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/export/fft.png");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export FFT plot as image"), defaultName,  tr("PNG image (*.png);;Vector graphics (*.pdf);;JPEG image (*.jpg);;BMP image (*.bmp)"));
    if (fileName.isEmpty())
      return;

    bool isOK = false;

    if (fileName.right(3).toLower() == "jpg")
      isOK = ui->plotFFT->saveJpg(fileName);
    else if (fileName.right(3).toLower() == "jpeg")
      isOK = ui->plotFFT->saveJpg(fileName);
    else if (fileName.right(3).toLower() == "bmp")
      isOK = ui->plotFFT->saveBmp(fileName);
    else if (fileName.right(3).toLower() == "pdf")
      isOK = ui->plotFFT->savePdf(fileName, 0, 0, QCP::epNoCosmetic);
    else // png
      isOK = ui->plotFFT->savePng(fileName);

    if (!isOK) {
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cant write to file."));
      msgBox.setInformativeText(tr("This may be because file is opened in another program."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
    }
  }
}
