//  Copyright (C) 2020-2024  Jiří Maier

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
#include "ui_developeroptions.h"
#include "ui_freqtimeplotdialog.h"

void MainWindow::comRefresh() {
  // Zjistí, jestli nastala změna v portech.
  QList<QSerialPortInfo> newPorts = QSerialPortInfo::availablePorts();
  bool change = false;

  // Remove
  for (QSerialPortInfo port : qAsConst(portList)) {
    if (!newPorts.contains(port)) {
      auto toremove = ui->listWidgetCom->findItems(
          port.portName() + " - " + port.description(), Qt::MatchExactly);
      Q_ASSERT(toremove.length() == 1);
      for (auto item : toremove) {
        if (ui->listWidgetCom->currentItem() == item) {
          attemptReconnectPort = item->text();
          ui->listWidgetCom->setCurrentRow(-1);
        }
        delete ui->listWidgetCom->takeItem(ui->listWidgetCom->row(item));
      }
      change = true;
    }
  }

  // Add
  for (QSerialPortInfo port : qAsConst(newPorts)) {
    if (!portList.contains(port)) {
      auto newItem = new QListWidgetItem();
      newItem->setText(port.portName() + " - " + port.description());
      newItem->setData(Qt::UserRole, port.portName());
      ui->listWidgetCom->addItem(newItem);
      change = true;
    }
  }

  if (change) {
    portList = newPorts;
    if (attemptReconnectPort != "") {
      auto candidates =
          ui->listWidgetCom->findItems(attemptReconnectPort, Qt::MatchExactly);
      if (!candidates.isEmpty()) {
        ui->listWidgetCom->setCurrentItem(candidates.first());
      }
    }
  }
}

void MainWindow::updateUsedChannels() {
  updateChannelComboBox(*ui->comboBoxCursor1Channel, 3);
  updateChannelComboBox(*ui->comboBoxCursor2Channel, 3);
  updateChannelComboBox(*ui->comboBoxSelectedChannel, 0);
  updateChannelComboBox(*ui->comboBoxMeasure1, 1);
  updateChannelComboBox(*ui->comboBoxMeasure2, 1);
  updateChannelComboBox(*ui->comboBoxFFTCh1, 0);
  updateChannelComboBox(*ui->comboBoxFFTCh2, 0);
  updateChannelComboBox(*ui->comboBoxXYx, 0);
  updateChannelComboBox(*ui->comboBoxXYy, 0);
  updateChannelComboBox(*ui->comboBoxMathFirst1, 1);
  updateChannelComboBox(*ui->comboBoxMathFirst2, 1);
  updateChannelComboBox(*ui->comboBoxMathFirst3, 1);
  updateChannelComboBox(*ui->comboBoxMathSecond1, 1);
  updateChannelComboBox(*ui->comboBoxMathSecond2, 1);
  updateChannelComboBox(*ui->comboBoxMathSecond3, 1);
  updateChannelComboBox(*ui->comboBoxLogic1, 0);
  updateChannelComboBox(*ui->comboBoxLogic2, 0);
  updateChannelComboBox(*ui->comboBoxAvgIndividualCh, 0);
  colorUpdateNeeded = false;
}

void MainWindow::updateChannelComboBox(QComboBox& combobox,
                                       int numberOfExcludedAtEnd) {
  // Nechá ve výběru kanálu jen ty kanály, které jsou používány
  // Pokud není používán žádný, nechá alespoň CH1 (protože vypadá blbě když v
  // nabídce není nic) Pokud je leaveAtLeastOne false (u měření, kde je v
  // nabídce vždy ještě off), tak ten CH1 nenechá
  bool atLeastOneVisible = false;

  for (int i = 0; i < combobox.count() - numberOfExcludedAtEnd; i++) {
    auto* model = qobject_cast<QStandardItemModel*>(combobox.model());
    auto* item = model->item(i);
    bool willBeVisible;
    if (i < ANALOG_COUNT + MATH_COUNT)
      willBeVisible =
          developerOptions->getUi()->checkBoxSelUnused->isChecked() ||
          ui->plot->isChUsed(i);
    else
      willBeVisible =
          developerOptions->getUi()->checkBoxSelUnused->isChecked() ||
          ui->plot->isChUsed(
              getLogicChannelID(i - ANALOG_COUNT - MATH_COUNT, 1));
    if (willBeVisible)
      atLeastOneVisible = true;

    item->setEnabled(willBeVisible);
    QListView* view = qobject_cast<QListView*>(combobox.view());
    view->setRowHidden(i, !willBeVisible);

    if (colorUpdateNeeded) {
      QPixmap color(12, 12);
      if (i < ANALOG_COUNT + MATH_COUNT)
        color.fill(ui->plot->getChColor(i));
      else
        color.fill(ui->plot->getLogicColor(i - ANALOG_COUNT - MATH_COUNT));
      item->setIcon(QIcon(color));
    }
  }

  if (numberOfExcludedAtEnd == 0 && (atLeastOneVisible == false)) {
    auto* model = qobject_cast<QStandardItemModel*>(combobox.model());
    auto* item = model->item(0);
    item->setEnabled(true);
    QListView* view = qobject_cast<QListView*>(combobox.view());
    view->setRowHidden(0, false);
  }
}

void MainWindow::updateMeasurements1() {
  if (ui->tabsControll->currentIndex() != 2)
    return;  // Stránky s měřením není zobrazena, je zbytečné počítat
  if (ui->comboBoxMeasure1->currentIndex() !=
      ui->comboBoxMeasure1->count() - 1) {
    int chid = ui->comboBoxMeasure1->currentIndex();
    if (ui->plot->graph(chid)->data()->isEmpty())
      goto empty;
    auto data = QSharedPointer<QCPGraphDataContainer>(
        new QCPGraphDataContainer(*ui->plot->graph(chid)->data()));
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
    ui->labelSig1Vrms->setText("---");
    ui->labelSig1Dc->setText("---");
    ui->labelSig1Min->setText("---");
    ui->labelSig1Max->setText("---");
    ui->labelSig1samples->setText("---");
    ui->labelSig1fs->setText("---");
    ui->labelSig1rise->setText("---");
    ui->labelSig1fall->setText("---");
  }
}

void MainWindow::updateMeasurements2() {
  if (ui->tabsControll->currentIndex() != 2)
    return;  // Stránky s měřením není zobrazena, je zbytečné počítat
  if (ui->comboBoxMeasure2->currentIndex() !=
      ui->comboBoxMeasure2->count() - 1) {
    int chid = ui->comboBoxMeasure2->currentIndex();
    if (ui->plot->graph(chid)->data()->isEmpty())
      goto empty;
    auto data = QSharedPointer<QCPGraphDataContainer>(
        new QCPGraphDataContainer(*ui->plot->graph(chid)->data()));
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
    ui->labelSig2Vrms->setText("---");
    ui->labelSig2Dc->setText("---");
    ui->labelSig2Min->setText("---");
    ui->labelSig2Max->setText("---");
    ui->labelSig2samples->setText("---");
    ui->labelSig2fs->setText("---");
    ui->labelSig2rise->setText("---");
    ui->labelSig2fall->setText("---");
  }
}

void MainWindow::updateFFT1() {
  if (!ui->pushButtonFFT->isChecked())
    return;

  if (ui->checkBoxFFTCh1->isChecked()) {
    int chid = ui->comboBoxFFTCh1->currentIndex();

    auto data = QSharedPointer<QCPGraphDataContainer>(
        new QCPGraphDataContainer(*ui->plot->graph(chid)->data()));
    if (ui->radioButtonFFTPart->isChecked()) {
      data->removeBefore(ui->plot->xAxis->range().lower);
      data->removeAfter(ui->plot->xAxis->range().upper);
    }

    if (data->isEmpty()) {
      ui->plotFFT->clear(0);
      return;
    }

    if (ui->comboBoxFFTType->currentIndex() == FFTType::pwelch) {
      if (ui->spinBoxFFTSegments1->value() * 2 > data->size()) {
        // Není dostatek vzorků na tento počet segmentů (alespoň 2 na segment)
        ui->plotFFT->clear(0);
        return;
      }
    }

    if (ui->plotFFT->setChSorce(1, chid, ui->plot->getChColor(chid))) {
      freqTimePlotDialog->getUi()->plotPeak->setChSorce(
          1, chid, ui->plot->getChColor(chid));
      QPixmap color(12, 12);
      color.fill(ui->plot->getChColor(chid));
      auto* model = qobject_cast<QStandardItemModel*>(
          ui->comboBoxCursor1Channel->model());
      auto* item = model->item(FFT_INDEX(0));
      item->setIcon(QIcon(color));

      model = qobject_cast<QStandardItemModel*>(
          ui->comboBoxCursor2Channel->model());
      item = model->item(FFT_INDEX(0));
      item->setIcon(QIcon(color));
    }

    fftTimer1.stop();
    emit requestFFT1(
        data, (FFTType::enumFFTType)ui->comboBoxFFTType->currentIndex(),
        (FFTWindow::enumFFTWindow)ui->comboBoxFFTWindow1->currentIndex(),
        ui->checkBoxFFTNoDC1->isChecked(), ui->spinBoxFFTSegments1->value(),
        developerOptions->getUi()->checkBoxFFTTwoSided->isChecked(),
        developerOptions->getUi()->checkBoxFFTZeroCenter->isChecked(),
        ui->spinBoxFFTSamples1->value());
  } else
    ui->plotFFT->clear(0);
}

void MainWindow::updateFFT2() {
  if (!ui->pushButtonFFT->isChecked())
    return;

  if (ui->checkBoxFFTCh2->isChecked()) {
    int chid = ui->comboBoxFFTCh2->currentIndex();

    auto data = QSharedPointer<QCPGraphDataContainer>(
        new QCPGraphDataContainer(*ui->plot->graph(chid)->data()));
    if (ui->radioButtonFFTPart->isChecked()) {
      data->removeBefore(ui->plot->xAxis->range().lower);
      data->removeAfter(ui->plot->xAxis->range().upper);
    }

    if (data->isEmpty()) {
      ui->plotFFT->clear(1);
      return;
    }

    if (ui->comboBoxFFTType->currentIndex() == FFTType::pwelch) {
      if (ui->spinBoxFFTSegments2->value() * 2 > data->size()) {
        // Není dostatek vzorků na tento počet segmentů (alespoň 2 na segment)
        ui->plotFFT->clear(1);
        return;
      }
    }

    if (ui->plotFFT->setChSorce(2, chid, ui->plot->getChColor(chid))) {
      freqTimePlotDialog->getUi()->plotPeak->setChSorce(
          2, chid, ui->plot->getChColor(chid));
      QPixmap color(12, 12);
      color.fill(ui->plot->getChColor(chid));
      auto* model = qobject_cast<QStandardItemModel*>(
          ui->comboBoxCursor1Channel->model());
      auto* item = model->item(FFT_INDEX(1));
      item->setIcon(QIcon(color));

      model = qobject_cast<QStandardItemModel*>(
          ui->comboBoxCursor2Channel->model());
      item = model->item(FFT_INDEX(1));
      item->setIcon(QIcon(color));
    }
    fftTimer2.stop();
    emit requestFFT2(
        data, (FFTType::enumFFTType)ui->comboBoxFFTType->currentIndex(),
        (FFTWindow::enumFFTWindow)ui->comboBoxFFTWindow2->currentIndex(),
        ui->checkBoxFFTNoDC2->isChecked(), ui->spinBoxFFTSegments2->value(),
        developerOptions->getUi()->checkBoxFFTTwoSided->isChecked(),
        developerOptions->getUi()->checkBoxFFTZeroCenter->isChecked(),
        ui->spinBoxFFTSamples2->value());
  } else
    ui->plotFFT->clear(1);
}

void MainWindow::updateInterpolation() {
  for (int chid = 0; chid < ANALOG_COUNT + MATH_COUNT; chid++) {
    if (ui->plot->isChInterpolated(chid)) {
      bool dataIsFromInterpolationBuffer;
      QSharedPointer<QCPGraphDataContainer> data;
      if (ui->plot->dataToBeInterpolated.at(chid).isNull()) {
        data = QSharedPointer<QCPGraphDataContainer>(
            new QCPGraphDataContainer(*ui->plot->graph(chid)->data()));
        dataIsFromInterpolationBuffer = false;
      } else {
        data = ui->plot->dataToBeInterpolated.at(chid);
        ui->plot->dataToBeInterpolated[chid]
            .clear();  // Vymaže pointer ze seznamu, pozor, ne jeho obsah;
        dataIsFromInterpolationBuffer = true;
      }
      interpolationsRunning++;
      emit interpolate(chid, data, ui->plot->xAxis->range(),
                       dataIsFromInterpolationBuffer);
    }
  }

  if (interpolationsRunning > 0)
    interpolationTimer.stop();
}

void MainWindow::updateSerialMonitor() {
  if (serialMonitor.isEmpty() || !ui->pushButtonSerialMonitor->isChecked())
    return;

  serialMonitorTimer.stop();

  QElapsedTimer tmr;
  tmr.start();

  QString text;
  QDebug printer(&text);
  printer.nospace() << serialMonitor;
  serialMonitor.clear();

  if (ui->pushButtonDolarNewline->isChecked()) {
    text.replace("$$", "\n$$");
  }

  text.remove(0, 1);
  text.remove(text.length() - 1, 1);

  QScrollBar* scroll = ui->plainTextEditConsole_3->verticalScrollBar();
  int lastVal = -1;
  if (scroll->value() != scroll->maximum())
    lastVal = scroll->value();

  auto cursor = ui->plainTextEditConsole_3->textCursor();
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(text);

  if (lastVal == -1)
    scroll->setValue(scroll->maximum());
  else
    scroll->setValue(lastVal);

  if (tmr.elapsed() > 100) {
    ui->pushButtonSerialMonitor->setChecked(false);
    QMessageBox::warning(
        this, tr("Freeze prevention"),
        tr("Printing text takes too long, disabling serial monitor!"),
        QMessageBox::Ok, QMessageBox::Ok);
  }

  serialMonitorTimer.start();
}

void MainWindow::updateConsole() {
  if (consoleBuffer.isEmpty())
    return;

  consoleTimer.stop();

  QElapsedTimer tmr;
  tmr.start();
  foreach (QString str, consoleBuffer)
    ui->plainTextEditConsole->appendHtml(str);
  consoleBuffer.clear();
  if (tmr.elapsed() > 100 &&
      developerOptions->getUi()->checkBoxFreezeSafe->isChecked() &&
      ui->comboBoxOutputLevel->currentIndex() == OutputLevel::info) {
    ui->comboBoxOutputLevel->setCurrentIndex(OutputLevel::warning);
    QMessageBox::warning(
        this, tr("Freeze prevention"),
        tr("Printing text takes too long, disabling info messages!"),
        QMessageBox::Ok, QMessageBox::Ok);
  }
  consoleTimer.start();
}

void MainWindow::dataRateUpdate(int dataUpdates) {
  if (dataUpdates > 0) {
    ui->labelUpdateRate->setText(
        tr("Data rate: ") + QString::number(dataUpdates) + tr(" updates / s"));
  } else
    ui->labelUpdateRate->clear();
  this->dataUpdates = dataUpdates;
}

void MainWindow::updateXY() {
  if (ui->pushButtonXY->isChecked()) {
    auto in1 = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(
        *ui->plot->graph(ui->comboBoxXYx->currentIndex())->data()));
    auto in2 = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer(
        *ui->plot->graph(ui->comboBoxXYy->currentIndex())->data()));
    if (ui->radioButtonXYPart->isChecked()) {
      in1->removeBefore(ui->plot->xAxis->range().lower);
      in1->removeAfter(ui->plot->xAxis->range().upper);
      in2->removeBefore(ui->plot->xAxis->range().lower);
      in2->removeAfter(ui->plot->xAxis->range().upper);
    }
    if (in2->isEmpty() || in1->isEmpty()) {
      ui->plotxy->clear();
      return;
    }

    xyTimer.stop();
    emit requestXY(in1, in2, ui->checkBoxXYNoDC->isChecked());
  }
}
