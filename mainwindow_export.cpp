#include "mainwindow.h"

void MainWindow::exportCSV(bool all, int ch) {
  QString name = "";
  if (all)
    name = tr("all");
  else {
    if (ch == XY_CHANNEL)
      name = tr("xy");
    if (ch >= ANALOG_COUNT + MATH_COUNT)
      name = tr("Logic %1").arg(ch - ANALOG_COUNT - MATH_COUNT + 1);
    else {
      name = GlobalFunctions::getChName(ch);
    }
  }
  QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/export/%1.csv").arg(name);
  QString fileName = QFileDialog::getSaveFileName(this, tr("Export %1").arg(name), defaultName, tr("Comma separated values (*.csv)"));
  if (fileName.isEmpty())
    return;
  QFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Truncate)) {
    char decimal = ui->radioButtonCSVDot->isChecked() ? '.' : ',';
    char separator = ui->radioButtonCSVDot->isChecked() ? ',' : ';';
    if (all)
      file.write(ui->plot->exportAllCSV(separator, decimal, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVVRO->isChecked(), ui->checkBoxCSVIncludeHidden->isChecked()));
    else {
      if (ch >= ANALOG_COUNT + MATH_COUNT)
        file.write(ui->plot->exportLogicCSV(separator, decimal, ch - ANALOG_COUNT - MATH_COUNT, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVVRO->isChecked()));
      else if (ch == XY_CHANNEL)
        file.write(ui->plotxy->exportCSV(separator, decimal, ui->spinBoxCSVPrecision->value()));
      else
        file.write(ui->plot->exportChannelCSV(separator, decimal, ch, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVVRO->isChecked()));
    }
    file.close();
  } else {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant write to file."));
    msgBox.setInformativeText(tr("This may be because file is opened in another program."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}

void MainWindow::on_pushButtonPlotImage_clicked() {
  QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/export/plot.png");
  QString fileName = QFileDialog::getSaveFileName(this, tr("Export main plot as image"), defaultName, tr("Portable network graphics (*.png)"));
  if (fileName.isEmpty())
    return;
  ui->plot->toPNG().save(fileName);
}

void MainWindow::on_pushButtonXYImage_clicked() {
  QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/export/xy.png");
  QString fileName = QFileDialog::getSaveFileName(this, tr("Export XY plot as image"), defaultName, tr("Portable network graphics (*.png)"));
  if (fileName.isEmpty())
    return;
  ui->plotxy->toPNG().save(fileName);
}
