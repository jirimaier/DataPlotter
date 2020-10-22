#include "mainwindow.h"

void MainWindow::on_pushButtonAutoset_clicked() {
  QVector<int> activeChanntels;
  for (int i = 1; i <= CHANNEL_COUNT + MATH_COUNT; i++) {
    if (ui->plot->isChUsed(i) && ui->plot->getChStyle(i) != GraphStyle::hidden)
      activeChanntels.append(i);
  }

  if (activeChanntels.length() == 1) {
    double valueRange;
    double verticalCenter;
    if (dataMode == DataMode::binData) {
      valueRange = binSettings.valueMax - MIN(0, binSettings.valueMin);
      verticalCenter = MIN(0, binSettings.valueMin) + valueRange / 2;
    } else {
      double chmin = ui->plot->getChMinValue(activeChanntels.first());
      double chmax = ui->plot->getChMaxValue(activeChanntels.first());
      valueRange = chmax - MIN(0, chmin);
      verticalCenter = MIN(0, chmin) + valueRange / 2;
    }

    useSettings("vpos:" + QString::number(verticalCenter * 200 / valueRange).toUtf8());
    useSettings("vrange:" + QString::number(logaritmicSettings[roundToStandardValue(valueRange)]).toUtf8());
    useSettings("plotvval:1");

  } else {
    int currentlySelectedCh = ui->spinBoxChannelSelect->value();
    useSettings("plotvval:0");
    useSettings("vpos:100");
    double valueRange;
    if (dataMode == DataMode::binData) {
      valueRange = binSettings.valueMax - MIN(0, binSettings.valueMin);
      valueRange = logaritmicSettings[roundToStandardValue(valueRange)];

    } else {
      QVector<double> ranges;
      foreach (int ch, activeChanntels) {
        double chmin = ui->plot->getChMinValue(ch);
        double chmax = ui->plot->getChMaxValue(ch);
        ranges.append(chmax - MIN(0, chmin));
      }
      valueRange = *std::max_element(ranges.begin(), ranges.end());
    }
    for (int i = 0; i < activeChanntels.length(); i++) {
      useSettings("ch:" + QString::number(activeChanntels.at(i)).toUtf8());
      useSettings("choff:" + QString::number(valueRange * activeChanntels.length() - i * valueRange - valueRange / 2).toUtf8());
    }
    useSettings("vrange:" + QString::number(valueRange * activeChanntels.length()).toUtf8());
    useSettings("ch:" + QString::number(currentlySelectedCh).toUtf8());
  }
}
