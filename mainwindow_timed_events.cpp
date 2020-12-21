//  Copyright (C) 2020  Jiří Maier

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
      if (port.description().contains(PORT_NUCLEO_DESCRIPTION_IDENTIFIER))
        portWithStName = i;
    }

    // Znovu vypere původní port; pokud neexistuje, vybere port který je asi Nucleo, pokud žádný popisem neodpovídá, vybere ten první.
    ui->comboBoxCom->setCurrentIndex(ui->comboBoxCom->findText(current) == -1 ? MAX(portWithStName, 0) : ui->comboBoxCom->findText(current));
  }
}

void MainWindow::updateUsedChannels() {
  updateChannelComboBox(*ui->comboBoxCursor1Channel);
  updateChannelComboBox(*ui->comboBoxCursor2Channel);
  updateChannelComboBox(*ui->comboBoxSelectedChannel);
  colorUpdateNeeded = false;
}

void MainWindow::updateChannelComboBox(QComboBox &combobox) {
  // Nechá ve výběru kanálu jen ty kanály, které jsou používány
  // Pokud není používán žádný, nechá alespoň CH1 (protože vypadá blbě když v nabídce není nic)
  bool atLeastOneVisible = false;
  for (int i = 0; i < LOGIC_GROUPS; i++) {
    auto *model = qobject_cast<QStandardItemModel *>(combobox.model());
    auto *item = model->item(i + ANALOG_COUNT + MATH_COUNT);
    bool willBeVisible = ui->checkBoxSelUnused->isChecked() || ui->plot->isChUsed(GlobalFunctions::getLogicChannelID(i, 1));
    if (willBeVisible)
      atLeastOneVisible = true;
    item->setEnabled(willBeVisible);
    QListView *view = qobject_cast<QListView *>(combobox.view());
    view->setRowHidden(i + ANALOG_COUNT + MATH_COUNT, !willBeVisible);
    if (colorUpdateNeeded) {
      QPixmap color(12, 12);
      color.fill(ui->plot->getLogicColor(i));
      item->setIcon(QIcon(color));
    }
  }
  for (int i = ANALOG_COUNT + MATH_COUNT - 1; i >= 0; i--) {
    auto *model = qobject_cast<QStandardItemModel *>(combobox.model());
    auto *item = model->item(i);
    bool willBeVisible = ui->checkBoxSelUnused->isChecked() || ui->plot->isChUsed(i);
    if (willBeVisible)
      atLeastOneVisible = true;
    if (i == 0 && !atLeastOneVisible)
      willBeVisible = true;
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
