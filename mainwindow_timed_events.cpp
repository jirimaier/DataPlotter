#include "mainwindow.h"

void MainWindow::updateMath() {
  for (int i = 0; i < 4; i++) {
    if (mathEn[i]->isChecked()) {
      int firstch = mathFirst[i]->value();
      int secondch = mathSecond[i]->value();
      int operation = mathOp[i]->currentIndex();
      emit requestMath(ANALOG_COUNT + 1 + i, operation, ui->plot->getDataVector(firstch - 1, ui->checkBoxMathIOS->isChecked(), ui->checkBoxMathVRO->isChecked()), ui->plot->getDataVector(secondch - 1, ui->checkBoxMathIOS->isChecked(), ui->checkBoxMathVRO->isChecked()));
    } else
      ui->plot->clearCh(ANALOG_COUNT + 1 + i);
  }
  if (ui->checkBoxXY->isChecked())
    requestXY(ui->plot->getDataVector(ui->spinBoxXYFirst->value() - 1, ui->checkBoxXYIOS->isChecked(), ui->checkBoxXYVRO->isChecked()), ui->plot->getDataVector(ui->spinBoxXYSecond->value() - 1, ui->checkBoxXYIOS->isChecked(), ui->checkBoxXYVRO->isChecked()));
}

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
  } else
    change = true;

  // Aktualizuje seznam portů
  if (change) {
    qDebug() << "Available ports changed";
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
  for (int i = 0; i < ANALOG_COUNT + MATH_COUNT; i++) {
    channelList->setRowHidden(i, !ui->plot->isChUsed(i));
  }
}
