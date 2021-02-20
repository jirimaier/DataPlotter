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

void MainWindow::updateCursorRange() {
  int ch = ui->comboBoxCursor1Channel->currentIndex();
  QPair<long, long> range;
  int fullRange;
  if (IS_ANALOG_OR_MATH(ch)) {
    range = ui->plot->getChVisibleSamplesRange(ch);
    fullRange = ui->plot->graph(ch)->data()->size() - 1;
  } else if (ch == XYID) {
    range = QPair<long, long>(0, ui->plotxy->graphXY->data()->size() - 1);
    fullRange = range.second;
  } else if (ch == FFTID) {
    range = ui->plotFFT->getVisibleSamplesRange();
    fullRange = ui->plotFFT->graph(0)->data()->size() - 1;
  } else {
    range = ui->plot->getChVisibleSamplesRange(getLogicChannelID(CH_LIST_LOGIC_GROUP(ch), 0));
    fullRange = ui->plot->graph(getLogicChannelID(CH_LIST_LOGIC_GROUP(ch), 0))->data()->size() - 1;
  }
  ui->horizontalSliderTimeCur1->updateRange(range.first, range.second);
  ui->spinBoxCur1Sample->setMaximum(fullRange);

  ch = ui->comboBoxCursor2Channel->currentIndex();
  if (IS_ANALOG_OR_MATH(ch)) {
    range = ui->plot->getChVisibleSamplesRange(ch);
    fullRange = ui->plot->graph(ch)->data()->size() - 1;
  } else if (ch == XYID) {
    range = QPair<long, long>(0, ui->plotxy->graphXY->data()->size());
    fullRange = range.second;
  } else if (ch == FFTID) {
    range = ui->plotFFT->getVisibleSamplesRange();
    fullRange = ui->plotFFT->graph(0)->data()->size() - 1;
  } else {
    range = ui->plot->getChVisibleSamplesRange(getLogicChannelID(CH_LIST_LOGIC_GROUP(ch), 0));
    fullRange = ui->plot->graph(getLogicChannelID(CH_LIST_LOGIC_GROUP(ch), 0))->data()->size() - 1;
  }
  ui->horizontalSliderTimeCur2->updateRange(range.first, range.second);
  ui->spinBoxCur2Sample->setMaximum(fullRange);
}

void MainWindow::updateCursors() {
  int ch1 = ui->comboBoxCursor1Channel->currentIndex();
  updateCursorRange();
  double time1 = 0, value1 = 0, time2 = 0, value2 = 0;
  QByteArray timeStr, valueStr;

  if (ui->checkBoxCur1Visible->isChecked()) {
    updateCursor(Cursors::X1, ch1, ui->spinBoxCur1Sample->value(), time1, value1, timeStr, valueStr);
    ui->labelCur1Time->setText(timeStr);
    ui->labelCur1Val->setText(valueStr);
  } else {
    ui->labelCur1Time->setText("---");
    ui->labelCur1Val->setText("---");
  }

  int ch2 = ui->comboBoxCursor2Channel->currentIndex();
  timeStr = "";
  valueStr = "";

  if (ui->checkBoxCur2Visible->isChecked()) {
    updateCursor(Cursors::X2, ch2, ui->spinBoxCur2Sample->value(), time2, value2, timeStr, valueStr);
    ui->labelCur2Time->setText(timeStr);
    ui->labelCur2Val->setText(valueStr);
  } else {
    ui->labelCur2Time->setText("---");
    ui->labelCur2Val->setText("---");
  }

  if (ui->checkBoxCur1Visible->isChecked() && ui->checkBoxCur2Visible->isChecked()) {
    double dt = time2 - time1;
    double dy = value2 - value1;
    // dB FFT
    if (ui->comboBoxCursor1Channel->currentIndex() == FFTID && ui->comboBoxCursor2Channel->currentIndex() == FFTID && ui->comboBoxFFTType->currentIndex() != FFTType::spectrum) {
      ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plotFFT->getXUnit());
      ui->labelCurDeltaValue->setText(floatToNiceString(dy, 4, true, true) + ui->plotFFT->getYUnit());
      ui->labelCurRatio->setText("");
      ui->labelCurSlope->setText("");
      ui->labelCurFreq->setText("");
    }

    // linear FFT
    else if (ui->comboBoxCursor1Channel->currentIndex() == FFTID && ui->comboBoxCursor2Channel->currentIndex() == FFTID && ui->comboBoxFFTType->currentIndex() == FFTType::spectrum) {
      ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plotFFT->getXUnit());
      ui->labelCurDeltaValue->setText(floatToNiceString(dy, 4, true, true));
      ui->labelCurRatio->setText(floatToNiceString(value1 / value2, 4, true, true));
      ui->labelCurSlope->setText("");
      ui->labelCurFreq->setText("");
    }

    // XY
    else if (ui->comboBoxCursor1Channel->currentIndex() == XYID && ui->comboBoxCursor2Channel->currentIndex() == XYID) {
      ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plotxy->getXUnit());
      ui->labelCurDeltaValue->setText(floatToNiceString(dy, 4, true, true) + ui->plotxy->getXUnit());
      ui->labelCurRatio->setText("");
      ui->labelCurSlope->setText("");
      ui->labelCurFreq->setText("");
    }

    // Time
    else if (ui->comboBoxCursor1Channel->currentIndex() < XYID && ui->comboBoxCursor2Channel->currentIndex() < XYID) {
      if (IS_ANALOG_OR_MATH(ch1) && IS_ANALOG_OR_MATH(ch2)) {
        // Both analog
        ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plot->getXUnit());
        ui->labelCurDeltaValue->setText(floatToNiceString(dy, 4, true, true) + ui->plot->getYUnit());
        ui->labelCurRatio->setText(floatToNiceString(value1 / value2, 4, true, true) + ui->plot->getYUnit() + "/" + ui->plot->getYUnit());
        ui->labelCurSlope->setText(floatToNiceString(dy / dt, 4, true, true) + ui->plot->getYUnit() + "/" + ui->plot->getXUnit());
        ui->labelCurFreq->setText(floatToNiceString(1.0 / dt, 4, true, true) + ui->plotFFT->getXUnit());
      } else if (IS_ANALOG_OR_MATH(ch1) || IS_ANALOG_OR_MATH(ch2)) {
        // Analog and Logic
        ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, false) + ui->plot->getXUnit());
        ui->labelCurDeltaValue->setText("");
        ui->labelCurRatio->setText("");
        ui->labelCurSlope->setText("");
        ui->labelCurFreq->setText(floatToNiceString(1.0 / dt, 4, true, false) + ui->plotFFT->getXUnit());
      } else {
        ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, false) + ui->plot->getXUnit());
        ui->labelCurDeltaValue->setText((dy >= 0 ? " 0x" : "-0x") + QString::number((uint32_t)std::abs(dy), 16).toUpper().rightJustified(valueStr.indexOf('\n') - 2, '0'));
        ui->labelCurRatio->setText("");
        ui->labelCurSlope->setText("");
        ui->labelCurFreq->setText(floatToNiceString(1.0 / dt, 4, true, false) + ui->plotFFT->getXUnit());
      }

    }

    else
      goto EMPTY;
  } else {
  EMPTY:
    ui->labelCurDeltaTime->setText("");
    ui->labelCurDeltaValue->setText("");
    ui->labelCurSlope->setText("");
    ui->labelCurRatio->setText("");
    ui->labelCurFreq->setText("");
  }
}

void MainWindow::updateCursor(Cursors::enumCursors cursor, int selectedChannel, unsigned int sample, double &time, double &value, QByteArray &timeStr, QByteArray &valueStr) {
  if (IS_ANALOG_OR_MATH(selectedChannel)) {
    // Analogový kanál
    time = ui->plot->graph(selectedChannel)->data()->at(sample)->key;
    value = ui->plot->graph(selectedChannel)->data()->at(sample)->value;
    if (ui->plot->isChInverted(selectedChannel))
      value *= (-1);
    double valueOffseted = value * ui->plot->getChScale(selectedChannel) + ui->plot->getChOffset(selectedChannel);
    timeStr = QString(floatToNiceString(time, 5, true, false) + ui->plot->getXUnit()).toUtf8();
    valueStr = QString(floatToNiceString(value, 5, true, false) + ui->plot->getYUnit()).toUtf8();
    ui->plot->setCursorVisible(cursor, true);
    ui->plot->setCursorVisible(cursor + 2, true);
    ui->plot->updateCursor(cursor, time, timeStr);
    ui->plot->updateCursor(cursor + 2, valueOffseted, valueStr);
  } else if (selectedChannel == XYID) {
    // XY
    double realyTime = ui->plotxy->graphXY->data()->at(sample)->t;
    double valueX = ui->plotxy->graphXY->data()->at(sample)->key;
    double valueY = ui->plotxy->graphXY->data()->at(sample)->value;

    timeStr = QString(floatToNiceString(realyTime, 5, true, false) + ui->plotxy->tUnit).toUtf8();
    QString valStrX = QString(floatToNiceString(valueX, 5)) + ui->plotxy->getXUnit();
    QString valStrY = QString(floatToNiceString(valueY, 5)) + ui->plotxy->getYUnit();
    valueStr = ("X: " + valStrX + "\n" + "Y: " + valStrY).toUtf8();
    time = valueX;
    value = valueY;
    ui->plotxy->setCursorVisible(cursor, true);
    ui->plotxy->setCursorVisible(cursor + 2, true);
    ui->plotxy->updateCursor(cursor, valueX, valStrX);
    ui->plotxy->updateCursor(cursor + 2, valueY, valStrY);
  } else if (selectedChannel == FFTID) {
    // FFT
    double freq = ui->plotFFT->graph(0)->data()->at(sample)->key;
    value = ui->plotFFT->graph(0)->data()->at(sample)->value;

    timeStr = QString(floatToNiceString(freq, 5, true, false) + ui->plotFFT->getXUnit()).toUtf8();
    valueStr = (QString::number(value, 'f', 3).rightJustified(8) + ui->plotFFT->getYUnit()).toUtf8();
    time = freq;
    ui->plotFFT->setCursorVisible(cursor, true);
    ui->plotFFT->setCursorVisible(cursor + 2, true);
    ui->plotFFT->updateCursor(cursor, freq, timeStr);
    ui->plotFFT->updateCursor(cursor + 2, value, valueStr);
  } else {
    // Logický kanál
    QByteArray bits;
    value = 0.0;
    int group = CH_LIST_LOGIC_GROUP(selectedChannel);
    time = ui->plot->graph(getLogicChannelID(group, 0))->data()->at(sample)->key;
    int bitsUsed = ui->plot->getLogicBitsUsed(group);
    for (int bit = 0; bit < bitsUsed; bit++) {
      int chid = getLogicChannelID(group, bit);
      // Potřebuji zjistit, zda je 0 nebo 1. To nejde jen tak (hodnota obsahuje
      // offset vůči prvnímu bitu) Že je nula zjistím tak, že hodnota je
      // dělitelná třemi (nulová úroveň každého jena násobku 3). Za každou
      // jedničku přičtu 2^bit
      if ((uint32_t)ui->plot->graph(chid)->data()->at(sample)->value % 3) {
        value += (uint32_t)1 << (bit);
        bits.push_front('1');
      } else {
        bits.push_front('0');
      }
      if (!((bit + 1) % 4))
        bits.push_front(' ');
    }
    if (bits.left(1) == " ")
      bits = bits.mid(1);
    timeStr = QString(floatToNiceString(time, 5, true, false) + ui->plot->getXUnit()).toUtf8();
    valueStr = "0x" + QString::number((uint32_t)value, 16).toUpper().rightJustified(ceil(bitsUsed / 4.0), '0').toLocal8Bit();
    valueStr.append("\n" + bits);
    ui->plot->setCursorVisible(cursor, true);
    ui->plot->setCursorVisible(cursor + 2, false);
    ui->plot->updateCursor(cursor, time, valueStr + "\n" + timeStr);
  }
}

void MainWindow::on_checkBoxCur1Visible_toggled(bool checked) {
  if (ui->comboBoxCursor1Channel->currentIndex() == XYID) {
    ui->plot->setCursorVisible(Cursors::X1, false);
    ui->plot->setCursorVisible(Cursors::Y1, false);
    ui->plotFFT->setCursorVisible(Cursors::X1, false);
    ui->plotFFT->setCursorVisible(Cursors::Y1, false);
    ui->plotxy->setCursorVisible(Cursors::X1, checked);
    ui->plotxy->setCursorVisible(Cursors::Y1, checked);
  } else if (ui->comboBoxCursor1Channel->currentIndex() == FFTID) {
    ui->plot->setCursorVisible(Cursors::X1, false);
    ui->plot->setCursorVisible(Cursors::Y1, false);
    ui->plotFFT->setCursorVisible(Cursors::X1, checked);
    ui->plotFFT->setCursorVisible(Cursors::Y1, checked);
    ui->plotxy->setCursorVisible(Cursors::X1, false);
    ui->plotxy->setCursorVisible(Cursors::Y1, false);
  } else {
    ui->plotxy->setCursorVisible(Cursors::X1, false);
    ui->plotxy->setCursorVisible(Cursors::Y1, false);
    ui->plotFFT->setCursorVisible(Cursors::X1, false);
    ui->plotFFT->setCursorVisible(Cursors::Y1, false);
    ui->plot->setCursorVisible(Cursors::X1, checked);
    ui->plot->setCursorVisible(Cursors::Y1, checked);
  }
  updateCursors();
}

void MainWindow::on_checkBoxCur2Visible_toggled(bool checked) {
  if (ui->comboBoxCursor2Channel->currentIndex() == XYID) {
    ui->plot->setCursorVisible(Cursors::X2, false);
    ui->plot->setCursorVisible(Cursors::Y2, false);
    ui->plotFFT->setCursorVisible(Cursors::X2, false);
    ui->plotFFT->setCursorVisible(Cursors::Y2, false);
    ui->plotxy->setCursorVisible(Cursors::X2, checked);
    ui->plotxy->setCursorVisible(Cursors::Y2, checked);
  } else if (ui->comboBoxCursor2Channel->currentIndex() == FFTID) {
    ui->plot->setCursorVisible(Cursors::X2, false);
    ui->plot->setCursorVisible(Cursors::Y2, false);
    ui->plotFFT->setCursorVisible(Cursors::X2, checked);
    ui->plotFFT->setCursorVisible(Cursors::Y2, checked);
    ui->plotxy->setCursorVisible(Cursors::X2, false);
    ui->plotxy->setCursorVisible(Cursors::Y2, false);
  } else {
    ui->plotxy->setCursorVisible(Cursors::X2, false);
    ui->plotxy->setCursorVisible(Cursors::Y2, false);
    ui->plotFFT->setCursorVisible(Cursors::X2, false);
    ui->plotFFT->setCursorVisible(Cursors::Y2, false);
    ui->plot->setCursorVisible(Cursors::X2, checked);
    ui->plot->setCursorVisible(Cursors::Y2, checked);
  }
  updateCursors();
}

void MainWindow::on_comboBoxCursor1Channel_currentIndexChanged(int index) {
  if (index == XYID) {
    ui->plot->setCursorVisible(Cursors::X1, false);
    ui->plot->setCursorVisible(Cursors::Y1, false);
    ui->plotFFT->setCursorVisible(Cursors::X1, false);
    ui->plotFFT->setCursorVisible(Cursors::Y1, false);
    ui->plotxy->setCursorVisible(Cursors::X1, ui->checkBoxCur1Visible->isChecked());
    ui->plotxy->setCursorVisible(Cursors::Y1, ui->checkBoxCur1Visible->isChecked());
  } else if (index == FFTID) {
    ui->plot->setCursorVisible(Cursors::X1, false);
    ui->plot->setCursorVisible(Cursors::Y1, false);
    ui->plotFFT->setCursorVisible(Cursors::X1, ui->checkBoxCur1Visible->isChecked());
    ui->plotFFT->setCursorVisible(Cursors::Y1, ui->checkBoxCur1Visible->isChecked());
    ui->plotxy->setCursorVisible(Cursors::X1, false);
    ui->plotxy->setCursorVisible(Cursors::Y1, false);
  } else {
    ui->plotxy->setCursorVisible(Cursors::X1, false);
    ui->plotxy->setCursorVisible(Cursors::Y1, false);
    ui->plotFFT->setCursorVisible(Cursors::X1, false);
    ui->plotFFT->setCursorVisible(Cursors::Y1, false);
    ui->plot->setCursorVisible(Cursors::X1, ui->checkBoxCur1Visible->isChecked());
    ui->plot->setCursorVisible(Cursors::Y1, ui->checkBoxCur1Visible->isChecked());
  }
  if (index == FFTID)
    ui->labelCur1TimeCaption->setText(tr("Frequency"));
  else
    ui->labelCur1TimeCaption->setText(tr("Time"));

  updateCursors();
}

void MainWindow::on_comboBoxCursor2Channel_currentIndexChanged(int index) {
  if (index == XYID) {
    ui->plot->setCursorVisible(Cursors::X2, false);
    ui->plot->setCursorVisible(Cursors::Y2, false);
    ui->plotFFT->setCursorVisible(Cursors::X2, false);
    ui->plotFFT->setCursorVisible(Cursors::Y2, false);
    ui->plotxy->setCursorVisible(Cursors::X2, ui->checkBoxCur2Visible->isChecked());
    ui->plotxy->setCursorVisible(Cursors::Y2, ui->checkBoxCur2Visible->isChecked());
  } else if (index == FFTID) {
    ui->plot->setCursorVisible(Cursors::X2, false);
    ui->plot->setCursorVisible(Cursors::Y2, false);
    ui->plotFFT->setCursorVisible(Cursors::X2, ui->checkBoxCur2Visible->isChecked());
    ui->plotFFT->setCursorVisible(Cursors::Y2, ui->checkBoxCur2Visible->isChecked());
    ui->plotxy->setCursorVisible(Cursors::X2, false);
    ui->plotxy->setCursorVisible(Cursors::Y2, false);
  } else {
    ui->plotxy->setCursorVisible(Cursors::X2, false);
    ui->plotxy->setCursorVisible(Cursors::Y2, false);
    ui->plotFFT->setCursorVisible(Cursors::X2, false);
    ui->plotFFT->setCursorVisible(Cursors::Y2, false);
    ui->plot->setCursorVisible(Cursors::X2, ui->checkBoxCur2Visible->isChecked());
    ui->plot->setCursorVisible(Cursors::Y2, ui->checkBoxCur2Visible->isChecked());
  }
  if (index == FFTID)
    ui->labelCur2TimeCaption->setText(tr("Frequency"));
  else
    ui->labelCur2TimeCaption->setText(tr("Time"));
  updateCursors();
}
void MainWindow::on_spinBoxCur1Sample_valueChanged(int arg1) {
  if (arg1 != -1) {
    ui->checkBoxCur1Visible->setChecked(true);
    ui->horizontalSliderTimeCur1->setRealVaule(arg1);
    updateCursors();
  } else {
    ui->checkBoxCur1Visible->setChecked(false);
  }
}

void MainWindow::on_spinBoxCur2Sample_valueChanged(int arg1) {
  if (arg1 != -1) {
    ui->checkBoxCur2Visible->setChecked(true);
    ui->horizontalSliderTimeCur2->setRealVaule(arg1);
    updateCursors();
  } else {
    ui->checkBoxCur1Visible->setChecked(false);
  }
}

void MainWindow::moveCursor(int chid, int cursor, int sample) {
  if (!IS_ANALOG_OR_MATH(chid) && !(chid == FFTID) && !(chid == XYID)) {
    chid = ChID_TO_LOGIC_GROUP(chid) + ANALOG_COUNT + MATH_COUNT;
  }
  if (cursor == 1) {
    ui->comboBoxCursor1Channel->setCurrentIndex(chid);
    ui->spinBoxCur1Sample->setValue(sample);
  } else {
    ui->comboBoxCursor2Channel->setCurrentIndex(chid);
    ui->spinBoxCur2Sample->setValue(sample);
  }
}

void MainWindow::offsetChangedByMouse(int chid) {
  if (ui->comboBoxSelectedChannel->currentIndex() == chid)
    ui->doubleSpinBoxChOffset->setValue(ui->plot->getChOffset(chid));
}
