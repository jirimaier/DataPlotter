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
  if (ch < ANALOG_COUNT + MATH_COUNT) {
    range = ui->plot->getChVisibleSamplesRange(ch);
    fullRange = ui->plot->graph(ch)->data()->size() - 1;
  } else if (ch == XYID) {
    range = QPair<long, long>(0, ui->plotxy->graphXY->data()->size() - 1);
    fullRange = range.second;
  } else if (ch == FFTID) {
    range = ui->plotFFT->getVisibleSamplesRange();
    fullRange = ui->plotFFT->graph(0)->data()->size() - 1;
  } else {
    range = ui->plot->getChVisibleSamplesRange(GlobalFunctions::getLogicChannelID(ch - ANALOG_COUNT - MATH_COUNT, 0));
    fullRange = ui->plot->graph(GlobalFunctions::getLogicChannelID(ch - ANALOG_COUNT - MATH_COUNT, 0))->data()->size() - 1;
  }
  ui->horizontalSliderTimeCur1->updateRange(range.first, range.second);
  ui->spinBoxCur1Sample->setMaximum(fullRange);

  ch = ui->comboBoxCursor2Channel->currentIndex();
  if (ch < ANALOG_COUNT + MATH_COUNT) {
    range = ui->plot->getChVisibleSamplesRange(ch);
    fullRange = ui->plot->graph(ch)->data()->size() - 1;
  } else if (ch == XYID) {
    range = QPair<long, long>(0, ui->plotxy->graphXY->data()->size());
    fullRange = range.second;
  } else if (ch == FFTID) {
    range = ui->plotFFT->getVisibleSamplesRange();
    fullRange = ui->plotFFT->graph(0)->data()->size() - 1;
  } else {
    range = ui->plot->getChVisibleSamplesRange(GlobalFunctions::getLogicChannelID(ch - ANALOG_COUNT - MATH_COUNT, 0));
    fullRange = ui->plot->graph(GlobalFunctions::getLogicChannelID(ch - ANALOG_COUNT - MATH_COUNT, 0))->data()->size() - 1;
  }
  ui->horizontalSliderTimeCur2->updateRange(range.first, range.second);
  ui->spinBoxCur2Sample->setMaximum(fullRange);
}

void MainWindow::updateCursors() {
  int ch = ui->comboBoxCursor1Channel->currentIndex();
  updateCursorRange();
  double time1 = 0, value1 = 0, time2 = 0, value2 = 0;
  QByteArray timeStr, valueStr;

  if (ui->checkBoxCur1Visible->isChecked()) {
    updateCursor(Cursors::X1, ch, ui->spinBoxCur1Sample->value(), time1, value1, timeStr, valueStr);
    ui->labelCur1Time->setText(timeStr);
    ui->labelCur1Val->setText(valueStr);
  } else {
    ui->labelCur1Time->setText("---");
    ui->labelCur1Val->setText("---");
  }

  ch = ui->comboBoxCursor2Channel->currentIndex();
  timeStr = "";
  valueStr = "";

  if (ui->checkBoxCur2Visible->isChecked()) {
    updateCursor(Cursors::X2, ch, ui->spinBoxCur2Sample->value(), time2, value2, timeStr, valueStr);
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
      ui->labelCurDeltaTime->setText(GlobalFunctions::floatToNiceString(dt, 2, true) + "Hz");
      ui->labelCurDeltaValue->setText(GlobalFunctions::floatToNiceString(dy, 2, true) + "dB");
      ui->labelCurRatio->setText("");
      ui->labelCurSlope->setText("");
      ui->labelCurFreq->setText("");
    }

    // linear FFT
    else if (ui->comboBoxCursor1Channel->currentIndex() == FFTID && ui->comboBoxCursor2Channel->currentIndex() == FFTID && ui->comboBoxFFTType->currentIndex() == FFTType::spectrum) {
      ui->labelCurDeltaTime->setText(GlobalFunctions::floatToNiceString(dt, 2, true) + "Hz");
      ui->labelCurDeltaValue->setText(GlobalFunctions::floatToNiceString(dy, 2, true));
      ui->labelCurRatio->setText(GlobalFunctions::floatToNiceString(value1 / value2, 2, true));
      ui->labelCurSlope->setText("");
      ui->labelCurFreq->setText("");
    }

    // XY
    else if (ui->comboBoxCursor1Channel->currentIndex() == XYID && ui->comboBoxCursor2Channel->currentIndex() == XYID) {
      ui->labelCurDeltaTime->setText(GlobalFunctions::floatToNiceString(dt, 2, true) + "V");
      ui->labelCurDeltaValue->setText(GlobalFunctions::floatToNiceString(dy, 2, true) + "V");
      ui->labelCurRatio->setText("");
      ui->labelCurSlope->setText("");
      ui->labelCurFreq->setText("");
    }

    // Time
    else if (ui->comboBoxCursor1Channel->currentIndex() < XYID && ui->comboBoxCursor2Channel->currentIndex() < XYID) {
      ui->labelCurDeltaTime->setText(GlobalFunctions::floatToNiceString(dt, 2, true) + "V");
      ui->labelCurDeltaValue->setText(GlobalFunctions::floatToNiceString(dy, 2, true) + "V");
      ui->labelCurRatio->setText(GlobalFunctions::floatToNiceString(value1 / value2, 2, true) + "  ");
      ui->labelCurSlope->setText(GlobalFunctions::floatToNiceString(dy / dt, 2, true) + "V/s");
      ui->labelCurFreq->setText(GlobalFunctions::floatToNiceString(1.0 / dt, 2, true) + "Hz");
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

void MainWindow::updateCursor(Cursors::enumerator cursor, int selectedChannel, unsigned int sample, double &time, double &value, QByteArray &timeStr, QByteArray &valueStr) {
  if (selectedChannel < ANALOG_COUNT + MATH_COUNT) {
    // Analogový kanál
    time = ui->plot->graph(selectedChannel)->data()->at(sample)->key;
    value = ui->plot->graph(selectedChannel)->data()->at(sample)->value;
    if (ui->plot->isChInverted(selectedChannel))
      value *= (-1);
    double valueOffseted = value * ui->plot->getChScale(selectedChannel) + ui->plot->getChOffset(selectedChannel);
    timeStr = QString(GlobalFunctions::floatToNiceString(time, 3) + "s").toUtf8();
    timeStr = QString(GlobalFunctions::floatToNiceString(value, 3) + "V").toUtf8();
    ui->plot->setCursorVisible(cursor, true);
    ui->plot->setCursorVisible(cursor + 2, true);
    ui->plot->updateCursor(cursor, time);
    ui->plot->updateCursor(cursor + 2, valueOffseted);
  } else if (selectedChannel == XYID) {
    // XY
    double realyTime = ui->plotxy->graphXY->data()->at(sample)->t;
    double valueX = ui->plotxy->graphXY->data()->at(sample)->key;
    double valueY = ui->plotxy->graphXY->data()->at(sample)->value;

    timeStr = QString(GlobalFunctions::floatToNiceString(realyTime, 3) + "V").toUtf8();
    valueStr = ("X: " + QString(GlobalFunctions::floatToNiceString(valueX, 3) + "V") + "\n" + "Y: " + QString(GlobalFunctions::floatToNiceString(valueY, 3) + "V")).toUtf8();
    time = valueX;
    value = valueY;
    ui->plotxy->setCursorVisible(cursor, true);
    ui->plotxy->setCursorVisible(cursor + 2, true);
    ui->plotxy->updateCursor(cursor, valueX);
    ui->plotxy->updateCursor(cursor + 2, valueY);
  } else if (selectedChannel == FFTID) {
    // FFT
    double freq = ui->plotFFT->graph(0)->data()->at(sample)->key;
    value = ui->plotFFT->graph(0)->data()->at(sample)->value;

    timeStr = QString(GlobalFunctions::floatToNiceString(freq, 3) + "Hz").toUtf8();
    valueStr = QString::number(value, 'f', 3).rightJustified(8).toLocal8Bit() + (ui->comboBoxFFTType->currentIndex() != FFTType::spectrum ? " dB" : "");
    time = freq;
    ui->plotFFT->setCursorVisible(cursor, true);
    ui->plotFFT->setCursorVisible(cursor + 2, true);
    ui->plotFFT->updateCursor(cursor, freq);
    ui->plotFFT->updateCursor(cursor + 2, value);
  } else {
    // Logický kanál
    QByteArray bits;
    value = 0.0;
    int group = selectedChannel - ANALOG_COUNT - MATH_COUNT;
    time = ui->plot->graph(GlobalFunctions::getLogicChannelID(group, 0))->data()->at(sample)->key;
    int bitsUsed = ui->plot->getLogicBitsUsed(group);
    for (int bit = 0; bit < bitsUsed; bit++) {
      int chid = GlobalFunctions::getLogicChannelID(group, bit);
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
    timeStr = QString::number(time, 'f', 3).toLocal8Bit();
    valueStr = "0x" + QString::number((uint32_t)value, 16).toUpper().rightJustified(ceil(bitsUsed / 4.0), '0').toLocal8Bit();
    valueStr.append("\n" + bits);
    ui->plot->setCursorVisible(cursor, true);
    ui->plot->setCursorVisible(cursor + 2, false);
    ui->plot->updateCursor(cursor, time);
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
