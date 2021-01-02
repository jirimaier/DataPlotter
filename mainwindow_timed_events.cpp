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

void MainWindow::comRefresh() {
  // Zjistí, jestli nastala změna v portech.
  QList<QSerialPortInfo> newPorts = QSerialPortInfo::availablePorts();
  bool change = false;
  if (newPorts.length() == portList.length()) {
    for (quint8 i = 0; i < newPorts.length(); i++)
      if (newPorts.at(i).portName() != portList.at(i).portName()) {
        change = true;
        break;
      }
  } else {
    change = true;
  }

  // Aktualizuje seznam portů
  if (change) {
    QString current = ui->comboBoxCom->currentText();
    ui->comboBoxCom->clear();
    portList = newPorts;
    int portWithStName = -1;

    // Nandá nové porty do comboboxu, pokusí se najít port jehož popis poukazuje na Nucleo
    for (int i = 0; i < portList.length(); i++) {
      QSerialPortInfo port = portList.at(i);
      ui->comboBoxCom->addItem(port.portName() + " - " + port.description());
      if (port.description().contains(PORT_NUCLEO_DESCRIPTION_IDENTIFIER)) portWithStName = i;
    }

    // Znovu vypere původní port; pokud neexistuje, vybere port který je asi Nucleo, pokud žádný popisem neodpovídá, vybere ten první.
    ui->comboBoxCom->setCurrentIndex(ui->comboBoxCom->findText(current) == -1 ? MAX(portWithStName, 0) : ui->comboBoxCom->findText(current));
  }
}

void MainWindow::updateUsedChannels() {
  updateChannelComboBox(*ui->comboBoxCursor1Channel, true, true);
  updateChannelComboBox(*ui->comboBoxCursor2Channel, true, true);
  updateChannelComboBox(*ui->comboBoxSelectedChannel, true, true);
  updateChannelComboBox(*ui->comboBoxMeasure1, false, false);
  updateChannelComboBox(*ui->comboBoxMeasure2, false, false);
  colorUpdateNeeded = false;
}

void MainWindow::updateChannelComboBox(QComboBox &combobox, bool includeLogic, bool leaveAtLeastOne) {
  // Nechá ve výběru kanálu jen ty kanály, které jsou používány
  // Pokud není používán žádný, nechá alespoň CH1 (protože vypadá blbě když v nabídce není nic)
  // Pokud je leaveAtLeastOne false (u měření, kde je v nabídce vždy ještě off), tak ten CH1 nenechá
  bool atLeastOneVisible = !leaveAtLeastOne;
  if (includeLogic) {
    for (int i = 0; i < LOGIC_GROUPS; i++) {
      auto *model = qobject_cast<QStandardItemModel *>(combobox.model());
      auto *item = model->item(i + ANALOG_COUNT + MATH_COUNT);
      bool willBeVisible = ui->checkBoxSelUnused->isChecked() || ui->plot->isChUsed(GlobalFunctions::getLogicChannelID(i, 1));
      if (willBeVisible) atLeastOneVisible = true;
      item->setEnabled(willBeVisible);
      QListView *view = qobject_cast<QListView *>(combobox.view());
      view->setRowHidden(i + ANALOG_COUNT + MATH_COUNT, !willBeVisible);
      if (colorUpdateNeeded) {
        QPixmap color(12, 12);
        color.fill(ui->plot->getLogicColor(i));
        item->setIcon(QIcon(color));
      }
    }
  }
  for (int i = ANALOG_COUNT + MATH_COUNT - 1; i >= 0; i--) {
    auto *model = qobject_cast<QStandardItemModel *>(combobox.model());
    auto *item = model->item(i);
    bool willBeVisible = ui->checkBoxSelUnused->isChecked() || ui->plot->isChUsed(i);
    if (willBeVisible) atLeastOneVisible = true;
    if (i == 0 && !atLeastOneVisible && !ui->pushButtonXY->isChecked()) willBeVisible = true;
    item->setEnabled(willBeVisible);
    QListView *view = qobject_cast<QListView *>(combobox.view());
    view->setRowHidden(i, !willBeVisible);
    if (colorUpdateNeeded) {
      QPixmap color(12, 12);
      color.fill(ui->plot->getChColor(i));
      item->setIcon(QIcon(color));
    }
  }
}

void MainWindow::updateMeasurements1() {
  if (ui->comboBoxMeasure1->currentIndex() != ui->comboBoxMeasure1->count() - 1) {
    int chid = ui->comboBoxMeasure1->currentIndex();
    if (ui->plot->graph(chid)->data()->isEmpty()) goto empty;
    auto data = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*ui->plot->graph(chid)->data()));
    if (ui->radioButtonSigPart->isChecked()) {
      data->removeBefore(ui->plot->xAxis->range().lower);
      data->removeAfter(ui->plot->xAxis->range().upper);
    }
    measureRefreshTimer1.stop();
    emit requstMeasurements1(data);
  } else {
  empty:
    ui->labelSig1Period->setText("---");
    ui->labelSig1Freq->setText("---");
    ui->labelSig1Amp->setText("---");
    ui->labelSig1Vpp->setText("---");
    ui->labelSig1Vrms->setText("---");
    ui->labelSig1Dc->setText("---");
    ui->labelSig1Min->setText("---");
    ui->labelSig1Max->setText("---");
  }
}

void MainWindow::updateMeasurements2() {
  if (ui->comboBoxMeasure2->currentIndex() != ui->comboBoxMeasure2->count() - 1) {
    int chid = ui->comboBoxMeasure2->currentIndex();
    if (ui->plot->graph(chid)->data()->isEmpty()) goto empty;
    auto data = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*ui->plot->graph(chid)->data()));
    if (ui->radioButtonSigPart->isChecked()) {
      data->removeBefore(ui->plot->xAxis->range().lower);
      data->removeAfter(ui->plot->xAxis->range().upper);
    }
    measureRefreshTimer2.stop();
    emit requstMeasurements2(data);
  } else {
  empty:
    ui->labelSig2Period->setText("---");
    ui->labelSig2Freq->setText("---");
    ui->labelSig2Amp->setText("---");
    ui->labelSig2Vpp->setText("---");
    ui->labelSig2Vrms->setText("---");
    ui->labelSig2Dc->setText("---");
    ui->labelSig2Min->setText("---");
    ui->labelSig2Max->setText("---");
  }
}

void MainWindow::updateFFT() {
  if (ui->pushButtonFFT->isChecked()) {
    int chid = ui->spinBoxFFT->value() - 1;
    if (ui->plot->graph(chid)->data()->isEmpty()) {
      ui->plotFFT->clear();
      return;
    }

    auto data = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(*ui->plot->graph(chid)->data()));
    if (ui->radioButtonFFTPart->isChecked()) {
      data->removeBefore(ui->plot->xAxis->range().lower);
      data->removeAfter(ui->plot->xAxis->range().upper);
    }
    emit requestFFT(data, ui->checkBoxFFTdB->isChecked(), (FFTWindow::enumerator)ui->comboBoxFFTWindow->currentIndex());
  }
}
