#include "mainwindow.h"

void MainWindow::exportCSV(bool all, int ch) {
  QString name = "";
  if (all)
    name = "all";
  else {
    if (ch == XY_CHANNEL)
      name = "xy";
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
      file.write(ui->plot->exportAllCSV(separator, decimal, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVIOS->isChecked(), ui->checkBoxCSVVRO->isChecked(), ui->checkBoxCSVIncludeHidden->isChecked()));
    else {
      if (ch > 0)
        file.write(ui->plot->exportChannelCSV(separator, decimal, ch, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVIOS->isChecked(), ui->checkBoxCSVVRO->isChecked()));
      else if (ch == XY_CHANNEL)
        file.write(ui->plotxy->exportCSV(separator, decimal, ui->spinBoxCSVPrecision->value()));
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
