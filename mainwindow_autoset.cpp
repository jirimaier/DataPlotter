#include "mainwindow.h"

void MainWindow::on_pushButtonAutoset_clicked() {
  QVector<int> actives;
  for (int i = 0; i < ALL_COUNT; i++)
    if (ui->plot->isChUsed(i))
      actives.append(i);
  if (actives.isEmpty())
    return;
  if (actives.length() == 1) {
    double min = ui->plot->getChMin(actives.at(0));
    if (min < 0) {
      ui->doubleSpinBoxRangeVerticalRange->setValue(Global::logaritmicSettings[GlobalFunctions::roundToStandardValue(ui->plot->getChMax(actives.at(0)))]);
      on_pushButtonCenter_clicked();
    } else {
      ui->doubleSpinBoxRangeVerticalRange->setValue(Global::logaritmicSettings[GlobalFunctions::roundToStandardValue(ui->plot->getChMax(actives.at(0)))] * 2);
      on_pushButtonPositive_clicked();
    }
  }
}
