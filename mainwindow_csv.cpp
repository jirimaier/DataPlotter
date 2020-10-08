#include "mainwindow.h"

void MainWindow::exportCSV(bool all, int ch) {
  QString defaultName = QString(QCoreApplication::applicationDirPath()) + QString("/export/%1.csv").arg(all ? "all" : QString("ch%1").arg(ch + 1));
  QString fileName = QFileDialog::getSaveFileName(this, all ? tr("Export Channel %1").arg(ch + 1) : tr("Export all channels"), defaultName, tr("Comma separated values (*.csv)"));
  if (fileName.isEmpty())
    return;
  QFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Truncate)) {
    char decimal = ui->radioButtonCSVDot->isChecked() ? '.' : ',';
    char separator = ui->radioButtonCSVDot->isChecked() ? ',' : ';';
    if (all)
      file.write(ui->plot->exportAllCSV(separator, decimal, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVoffsets->isChecked()));
    else
      file.write(ui->plot->exportChannelCSV(separator, decimal, ch, ui->spinBoxCSVPrecision->value(), ui->checkBoxCSVoffsets->isChecked()));
    file.close();
  } else {
    QMessageBox msgBox;
    msgBox.setText(tr("Cant write to file."));
    msgBox.setInformativeText(tr("This may be because file is opened in another program."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
}
