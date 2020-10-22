#include "mainwindow.h"

void MainWindow::updatePlot() {
  ui->plot->update();
  for (int i = 0; i < 4; i++) {
    if (mathEn[i]->isChecked()) {
      int firstch = mathFirst[i]->value();
      int secondch = mathSecond[i]->value();
      int operation = mathOp[i]->currentIndex();
      emit requestMath(CHANNEL_COUNT + 1 + i, operation, ui->plot->getDataVector(firstch - 1, false, true), ui->plot->getDataVector(secondch - 1, false, true));
    } else
      ui->plot->clearCh(CHANNEL_COUNT + 1 + i);
  }
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

void MainWindow::updateInfo() {
  if (!receivedListBuffer.isEmpty()) {
    foreach (QString line, receivedListBuffer)
      ui->textEditSerialDebug->append(line);
    receivedListBuffer.clear();
  }

  QString text = tr("Binary mode settings:");
  text.append(QString::number(binSettings.bits) + tr(" bits") + "\n");
  if (binSettings.bits != 64)
    text.append("Max (0x" + QString::number(((quint64)1 << binSettings.bits) - 1, 16).toUpper() + "): " + QString::number(binSettings.valueMax) + "\n");
  else
    text.append("Max (0xFFFFFFFFFFFFFFFF): " + QString::number(binSettings.valueMax) + "\n");
  text.append("Min (0x00): " + QString::number(binSettings.valueMin) + "\n");
  text.append(tr("Time step: ") + QString::number(binSettings.timeStep) + tr(" / sample") + "\n");
  if (binSettings.numCh == 1)
    text.append(tr("Channel ") + QString::number(binSettings.firstCh) + "\n");
  else
    text.append(tr("Channels ") + QString::number(binSettings.firstCh) + tr(" - ") + QString::number(binSettings.firstCh + binSettings.numCh - 1) + "\n");
  if (binSettings.continuous)
    text.append(tr("continous") + "\n");
  ui->labelBinSettings->setText(text.trimmed());
}

void MainWindow::autoResetChannels() {
  ui->plot->resetChannels();

  emit resetChannels();
}
