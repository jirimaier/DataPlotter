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

using namespace Cursors;

void MainWindow::updateCursorRange() {
  int ch = ui->comboBoxCursor1Channel->currentIndex();
  QPair<long, long> range;
  int fullRange;
  if (IS_ANALOG_OR_MATH(ch)) {
    range = ui->plot->getChVisibleSamplesRange(ch);
    fullRange = ui->plot->graph(ch)->data()->size() - 1;
  } else if (IS_FFT(ch)) {
    range = ui->plotFFT->getVisibleSamplesRange(CHID_TO_FFT_CHID(ch));
    fullRange = ui->plotFFT->graph(CHID_TO_FFT_CHID(ch))->data()->size() - 1;
  } else {
    range = ui->plot->getChVisibleSamplesRange(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0));
    fullRange = ui->plot->graph(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0))->data()->size() - 1;
  }
  ui->horizontalSliderTimeCur1->updateRange(range.first, range.second);
  ui->spinBoxCur1Sample->setMaximum(fullRange);

  ch = ui->comboBoxCursor2Channel->currentIndex();
  if (IS_ANALOG_OR_MATH(ch)) {
    range = ui->plot->getChVisibleSamplesRange(ch);
    fullRange = ui->plot->graph(ch)->data()->size() - 1;
  } else if (IS_FFT(ch)) {
    range = ui->plotFFT->getVisibleSamplesRange(CHID_TO_FFT_CHID(ch));
    fullRange = ui->plotFFT->graph(CHID_TO_FFT_CHID(ch))->data()->size() - 1;
  } else {
    range = ui->plot->getChVisibleSamplesRange(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0));
    fullRange = ui->plot->graph(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0))->data()->size() - 1;
  }
  ui->horizontalSliderTimeCur2->updateRange(range.first, range.second);
  ui->spinBoxCur2Sample->setMaximum(fullRange);
}

void MainWindow::updateCursors() {
  int ch1 = ui->comboBoxCursor1Channel->currentIndex();
  updateCursorRange();
  double time1 = 0, value1 = 0, time2 = 0, value2 = 0;
  QByteArray timeStr, valueStr;

  if (ui->spinBoxCur1Sample->maximum() < 0)
    ui->checkBoxCur1Visible->setChecked(false);

  if (ui->checkBoxCur1Visible->isChecked()) {
    updateCursor(Cursor1, ch1, ui->spinBoxCur1Sample->value(), time1, value1, timeStr, valueStr, ui->checkBoxYCur1->checkState() == Qt::CheckState::PartiallyChecked);
    ui->labelCur1Time->setText(timeStr);
    ui->labelCur1Val->setText(valueStr);
  } else {
    ui->labelCur1Time->setText("---");
    ui->labelCur1Val->setText("---");
  }

  int ch2 = ui->comboBoxCursor2Channel->currentIndex();
  timeStr = "";
  valueStr = "";

  if (ui->spinBoxCur2Sample->maximum() < 0)
    ui->checkBoxCur2Visible->setChecked(false);

  if (ui->checkBoxCur2Visible->isChecked()) {
    updateCursor(Cursor2, ch2, ui->spinBoxCur2Sample->value(), time2, value2, timeStr, valueStr, ui->checkBoxYCur2->checkState() == Qt::CheckState::PartiallyChecked);
    ui->labelCur2Time->setText(timeStr);
    ui->labelCur2Val->setText(valueStr);
  } else {
    ui->labelCur2Time->setText("---");
    ui->labelCur2Val->setText("---");
  }

  if (ui->checkBoxCur1Visible->isChecked() && ui->checkBoxCur2Visible->isChecked()) {
    double dt = time2 - time1;
    if (ui->checkBoxYCur1->checkState() == Qt::CheckState::Checked)
      value1 = ui->doubleSpinBoxYCur1->value();
    if (ui->checkBoxYCur2->checkState() == Qt::CheckState::Checked)
      value2 = ui->doubleSpinBoxYCur2->value();
    double dy = value2 - value1;
    // dB FFT
    if (IS_FFT(ui->comboBoxCursor1Channel->currentIndex()) && IS_FFT(ui->comboBoxCursor2Channel->currentIndex()) && ui->comboBoxFFTType->currentIndex() != FFTType::spectrum) {
      ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plotFFT->getXUnit());
      ui->labelCurDeltaValue->setText(QString::number(dy, 'f', 3).rightJustified(7) + " dB");
      ui->labelCurRatio->setText("");
      ui->labelCurSlope->setText("");
      ui->labelCurFreq->setText("");
    }

    // linear FFT
    else if (IS_FFT(ui->comboBoxCursor1Channel->currentIndex()) && IS_FFT(ui->comboBoxCursor2Channel->currentIndex()) && ui->comboBoxFFTType->currentIndex() == FFTType::spectrum) {
      ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plotFFT->getXUnit());
      ui->labelCurDeltaValue->setText(floatToNiceString(dy, 4, true, true));
      ui->labelCurRatio->setText(floatToNiceString(value1 / value2, 4, true, true));
      ui->labelCurSlope->setText("");
      ui->labelCurFreq->setText("");
    }

    // Time
    else if (IS_ANALOG_OR_MATH_OR_LOGIC(ui->comboBoxCursor1Channel->currentIndex()) && IS_ANALOG_OR_MATH_OR_LOGIC(ui->comboBoxCursor2Channel->currentIndex())) {
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

void MainWindow::updateCursor(Cursors::enumCursors cursor, int selectedChannel, unsigned int sample, double& time, double& value, QByteArray& timeStr, QByteArray& valueStr, bool useValueCursor) {
  if (IS_ANALOG_OR_MATH(selectedChannel)) {
    // Analogový kanál
    time = ui->plot->graph(selectedChannel)->data()->at(sample)->key;
    value = ui->plot->graph(selectedChannel)->data()->at(sample)->value;
//    if (ui->plot->isChInverted(selectedChannel))
//      value *= (-1);
    timeStr = QString(floatToNiceString(time, 5, true, false) + ui->plot->getXUnit()).toUtf8();
    valueStr = QString(floatToNiceString(value, 5, true, false) + ui->plot->getYUnit()).toUtf8();
    if (useValueCursor) {
      ui->plot->updateValueCursor(cursor, value, valueStr, ui->plot->getAnalogAxis(selectedChannel));
    }
    ui->plot->updateTimeCursor(cursor, time, timeStr, ui->plot->graph(selectedChannel));

  } else if (IS_FFT(selectedChannel)) {
    // FFT
    double freq = ui->plotFFT->graph(CHID_TO_FFT_CHID(selectedChannel))->data()->at(sample)->key;
    value = ui->plotFFT->graph(CHID_TO_FFT_CHID(selectedChannel))->data()->at(sample)->value;

    timeStr = QString(floatToNiceString(freq, 5, true, false) + ui->plotFFT->getXUnit()).toUtf8();
    valueStr = (QString::number(value, 'f', 3).rightJustified(8) + " " + ui->plotFFT->getYUnit()).toUtf8();
    time = freq;
    if (useValueCursor) {
      ui->plotFFT->updateValueCursor(cursor, value, valueStr, ui->plotFFT->yAxis);
    }
    ui->plotFFT->updateTimeCursor(cursor, freq, timeStr, ui->plotFFT->graph(CHID_TO_FFT_CHID(selectedChannel)));

  } else {
    // Logický kanál
    QByteArray bits;
    value = 0.0;
    int group = CH_LIST_INDEX_TO_LOGIC_GROUP(selectedChannel);
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
    ui->plot->updateTimeCursor(cursor, time, valueStr + "\n" + timeStr, ui->plot->graph(getLogicChannelID(group, 0)));
  }
}

void MainWindow::on_checkBoxCur1Visible_toggled(bool checked) {
  setCursorsVisibility(Cursor1, ui->comboBoxCursor1Channel->currentIndex(), checked, ui->checkBoxYCur1->checkState());
  updateCursors();
}

void MainWindow::on_checkBoxCur2Visible_toggled(bool checked) {
  setCursorsVisibility(Cursor2, ui->comboBoxCursor2Channel->currentIndex(), checked, ui->checkBoxYCur2->checkState());
  updateCursors();
}

void MainWindow::on_comboBoxCursor1Channel_currentIndexChanged(int index) {
  setCursorsVisibility(Cursor1, index, ui->checkBoxCur1Visible->isChecked(), ui->checkBoxYCur1->checkState());
  if (IS_FFT(index)) {
    ui->labelCur1TimeCaption->setText(tr("Frequency"));
    if (ui->comboBoxFFTType->currentIndex() == FFTType::spectrum)
      ui->doubleSpinBoxYCur1->setSuffix("");
    else
      ui->doubleSpinBoxYCur1->setSuffix("");
  } else {
    ui->labelCur1TimeCaption->setText(tr("Time"));
    ui->doubleSpinBoxYCur1->setSuffix(ui->lineEditVUnit->text());
  }
  if (ui->checkBoxYCur1->checkState() == Qt::CheckState::Checked)
    on_doubleSpinBoxYCur1_valueChanged(ui->doubleSpinBoxYCur1->value());
  updateCursors();
}

void MainWindow::on_comboBoxCursor2Channel_currentIndexChanged(int index) {
  setCursorsVisibility(Cursor2, index, ui->checkBoxCur2Visible->isChecked(), ui->checkBoxYCur2->checkState());
  if (IS_FFT(index)) {
    ui->labelCur2TimeCaption->setText(tr("Frequency"));
    if (ui->comboBoxFFTType->currentIndex() == FFTType::spectrum)
      ui->doubleSpinBoxYCur2->setSuffix("");
    else
      ui->doubleSpinBoxYCur2->setSuffix("");
  } else {
    ui->labelCur2TimeCaption->setText(tr("Time"));
    ui->doubleSpinBoxYCur2->setSuffix(ui->lineEditVUnit->text());
  }
  if (ui->checkBoxYCur2->checkState() == Qt::CheckState::Checked)
    on_doubleSpinBoxYCur2_valueChanged(ui->doubleSpinBoxYCur2->value());
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

void MainWindow::timeCursorMovedByMouse(Cursors::enumCursors cursor, int sample) {
  if (cursor == Cursor1)
    ui->spinBoxCur1Sample->setValue(sample);
  else
    ui->spinBoxCur2Sample->setValue(sample);
}

void MainWindow::valueCursorMovedByMouse(enumCursors cursor, double value) {
  if (cursor == Cursor1) {
    if (ui->checkBoxYCur1->checkState() != Qt::CheckState::Checked)
      ui->checkBoxYCur1->setCheckState(Qt::CheckState::Checked);
    ui->doubleSpinBoxYCur1->setValue(value);
  } else {
    if (ui->checkBoxYCur2->checkState() != Qt::CheckState::Checked)
      ui->checkBoxYCur2->setCheckState(Qt::CheckState::Checked);
    ui->doubleSpinBoxYCur2->setValue(value);
  }
}

void MainWindow::cursorSetByMouse(int chid, Cursors::enumCursors cursor, int sample) {
  if (cursor == Cursor1) {
    if (ui->checkBoxYCur1->checkState() == Qt::CheckState::Checked)
      ui->checkBoxYCur1->setCheckState(Qt::CheckState::PartiallyChecked);
    ui->comboBoxCursor1Channel->setCurrentIndex(chid);
    ui->spinBoxCur1Sample->setValue(sample);
  } else {
    if (ui->checkBoxYCur2->checkState() == Qt::CheckState::Checked)
      ui->checkBoxYCur2->setCheckState(Qt::CheckState::PartiallyChecked);
    ui->comboBoxCursor2Channel->setCurrentIndex(chid);
    ui->spinBoxCur2Sample->setValue(sample);
  }
}

void MainWindow::offsetChangedByMouse(int chid) {
  if (ui->comboBoxSelectedChannel->currentIndex() == chid)
    ui->doubleSpinBoxChOffset->setValue(ui->plot->getChOffset(chid));
}

void MainWindow::on_checkBoxYCur1_stateChanged(int arg1) {
  ui->doubleSpinBoxYCur1->setVisible(arg1 == Qt::CheckState::Checked);
  setCursorsVisibility(Cursor1, ui->comboBoxCursor1Channel->currentIndex(), ui->checkBoxCur1Visible->isChecked(), arg1);
  if (arg1 == Qt::CheckState::Checked) {
    if (IS_FFT(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setValue(ui->plotFFT->getValueCursorPosition(Cursor1));
    else if (IS_ANALOG_OR_MATH(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setValue(ui->plot->getValueCursorPosition(Cursor1));
    else
      ui->checkBoxYCur1->setCheckState(Qt::CheckState::Unchecked);
  } else if (arg1 == Qt::CheckState::PartiallyChecked)
    updateCursors();
}

void MainWindow::on_checkBoxYCur2_stateChanged(int arg1) {
  ui->doubleSpinBoxYCur2->setVisible(arg1 == Qt::CheckState::Checked);
  setCursorsVisibility(Cursor2, ui->comboBoxCursor2Channel->currentIndex(), ui->checkBoxCur2Visible->isChecked(), arg1);
  if (arg1 == Qt::CheckState::Checked) {
    if (IS_FFT(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setValue(ui->plotFFT->getValueCursorPosition(Cursor2));
    else if (IS_ANALOG_OR_MATH(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setValue(ui->plot->getValueCursorPosition(Cursor2));
    else
      ui->checkBoxYCur2->setCheckState(Qt::CheckState::Unchecked);
  } else if (arg1 == Qt::CheckState::PartiallyChecked)
    updateCursors();
}

void MainWindow::setCursorsVisibility(Cursors::enumCursors cursor, int graph, bool timeCurVisible, int valueCurState) {
  bool valueCurVisible;
  if (valueCurState == Qt::CheckState::Unchecked)
    valueCurVisible = false;
  else if (valueCurState == Qt::CheckState::Checked)
    valueCurVisible = true;
  else
    valueCurVisible = timeCurVisible;

  if (IS_FFT(graph)) {
    ui->plot->setTimeCursorVisible(cursor, false);
    ui->plot->setValueCursorVisible(cursor, false);
    ui->plotFFT->setTimeCursorVisible(cursor, timeCurVisible);
    ui->plotFFT->setValueCursorVisible(cursor, valueCurVisible);
  } else if (IS_ANALOG_OR_MATH(graph)) {
    ui->plot->setTimeCursorVisible(cursor, timeCurVisible);
    ui->plot->setValueCursorVisible(cursor, valueCurVisible);
    ui->plotFFT->setTimeCursorVisible(cursor, false);
    ui->plotFFT->setValueCursorVisible(cursor, false);
  } else { // Logic
    ui->plot->setTimeCursorVisible(cursor, timeCurVisible);
    ui->plot->setValueCursorVisible(cursor, false);
    ui->plotFFT->setTimeCursorVisible(cursor, false);
    ui->plotFFT->setValueCursorVisible(cursor, false);
  }
}


void MainWindow::on_doubleSpinBoxYCur1_valueChanged(double arg1) {
  int chID = ui->comboBoxCursor1Channel->currentIndex();
  if (IS_FFT(chID))
    ui->plotFFT->updateValueCursor(Cursor1, arg1, ui->doubleSpinBoxYCur1->text(), ui->plotFFT->yAxis);
  else
    ui->plot->updateValueCursor(Cursor1, arg1, ui->doubleSpinBoxYCur1->text(), ui->plot->getAnalogAxis(chID));
}

void MainWindow::on_doubleSpinBoxYCur2_valueChanged(double arg1) {
  int chID = ui->comboBoxCursor2Channel->currentIndex();
  if (IS_FFT(chID))
    ui->plotFFT->updateValueCursor(Cursor2, arg1, ui->doubleSpinBoxYCur2->text(), ui->plotFFT->yAxis);
  else
    ui->plot->updateValueCursor(Cursor2, arg1, ui->doubleSpinBoxYCur2->text(), ui->plot->getAnalogAxis(chID));
}

