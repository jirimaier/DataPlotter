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
  bool empty;
  if (IS_ANALOG_OR_MATH(ch)) {
    range = ui->plot->getChVisibleSamplesRange(ch);
    empty = ui->plot->graph(ch)->data()->isEmpty();
    ui->doubleSpinBoxXCur1->setSingleStep(ui->plot->xAxis->range().size() / 100);
  } else if (IS_FFT_INDEX(ch)) {
    range = ui->plotFFT->getVisibleSamplesRange(INDEX_TO_FFT_CHID(ch));
    empty = ui->plotFFT->graph(INDEX_TO_FFT_CHID(ch))->data()->isEmpty();
    ui->doubleSpinBoxXCur1->setSingleStep(ui->plotFFT->xAxis->range().size() / 100);
  } else {
    range = ui->plot->getChVisibleSamplesRange(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0));
    empty = ui->plot->graph(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0))->data()->isEmpty();
    ui->doubleSpinBoxXCur1->setSingleStep(ui->plot->xAxis->range().size() / 100);
  }
  ui->horizontalSliderTimeCur1->updateRange(range.first, range.second);

  if (empty && ui->checkBoxCur1Visible->checkState() == Qt::CheckState::PartiallyChecked) {
    if (IS_ANALOG_OR_MATH(ch)) {
      ui->plot->setTimeCursorVisible(Cursor1, false);
      ui->plot->setValueCursorVisible(Cursor1, false);
    } else {
      ui->plotFFT->setTimeCursorVisible(Cursor1, false);
      ui->plotFFT->setValueCursorVisible(Cursor1, false);
    }
  }

  ch = ui->comboBoxCursor2Channel->currentIndex();
  if (IS_ANALOG_OR_MATH(ch)) {
    range = ui->plot->getChVisibleSamplesRange(ch);
    empty = ui->plot->graph(ch)->data()->isEmpty();
    ui->doubleSpinBoxXCur2->setSingleStep(ui->plot->xAxis->range().size() / 100);
  } else if (IS_FFT_INDEX(ch)) {
    range = ui->plotFFT->getVisibleSamplesRange(INDEX_TO_FFT_CHID(ch));
    empty = ui->plotFFT->graph(INDEX_TO_FFT_CHID(ch))->data()->isEmpty();
    ui->doubleSpinBoxXCur2->setSingleStep(ui->plotFFT->xAxis->range().size() / 100);
  } else {
    range = ui->plot->getChVisibleSamplesRange(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0));
    empty = ui->plot->graph(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0))->data()->isEmpty();
    ui->doubleSpinBoxXCur2->setSingleStep(ui->plot->xAxis->range().size() / 100);
  }
  ui->horizontalSliderTimeCur2->updateRange(range.first, range.second);

  if (empty && ui->checkBoxCur2Visible->checkState() == Qt::CheckState::PartiallyChecked) {
    if (IS_ANALOG_OR_MATH(ch)) {
      ui->plot->setTimeCursorVisible(Cursor2, false);
      ui->plot->setValueCursorVisible(Cursor2, false);
    } else {
      ui->plotFFT->setTimeCursorVisible(Cursor2, false);
      ui->plotFFT->setValueCursorVisible(Cursor2, false);
    }
  }
}

void MainWindow::updateCursors() {
  int ch1 = ui->comboBoxCursor1Channel->currentIndex();

  double time1 = 0, value1 = 0, time2 = 0, value2 = 0;
  QByteArray timeStr, valueStr;

  if (ui->checkBoxCur1Visible->checkState() == Qt::CheckState::PartiallyChecked) {
    updateCursor(Cursor1, ch1, ui->horizontalSliderTimeCur1->getRealValue(), time1, value1, timeStr, valueStr, ui->checkBoxYCur1->checkState() == Qt::CheckState::PartiallyChecked);
    ui->labelCur1Time->setText(timeStr);
    ui->labelCur1Val->setText(valueStr);
  } else {
    ui->labelCur1Time->setText("---");
    ui->labelCur1Val->setText("---");
  }

  int ch2 = ui->comboBoxCursor2Channel->currentIndex();
  timeStr = "";
  valueStr = "";

  if (ui->checkBoxCur2Visible->checkState() == Qt::CheckState::PartiallyChecked) {
    updateCursor(Cursor2, ch2, ui->horizontalSliderTimeCur2->getRealValue(), time2, value2, timeStr, valueStr, ui->checkBoxYCur2->checkState() == Qt::CheckState::PartiallyChecked);
    ui->labelCur2Time->setText(timeStr);
    ui->labelCur2Val->setText(valueStr);
  } else {
    ui->labelCur2Time->setText("---");
    ui->labelCur2Val->setText("---");
  }

  updateCursorMeasurementsText();
}

void MainWindow::updateCursorMeasurementsText() {
  ui->labelCurDeltaTime->setText("---");
  ui->labelCurDeltaValue->setText("---");
  ui->labelCurSlope->setText("---");
  ui->labelCurRatio->setText("---");
  ui->labelCurFreq->setText("---");

  int ch1 = ui->comboBoxCursor1Channel->currentIndex();
  int ch2 = ui->comboBoxCursor2Channel->currentIndex();
  if (ui->checkBoxCur1Visible->checkState() != Qt::CheckState::Unchecked && ui->checkBoxCur2Visible->checkState() != Qt::CheckState::Unchecked) {
    double time1 = IS_FFT_INDEX(ch1) ? ui->plotFFT->getTimeCursorPosition(Cursor1) : ui->plot->getTimeCursorPosition(Cursor1);
    double time2 = IS_FFT_INDEX(ch2) ? ui->plotFFT->getTimeCursorPosition(Cursor2) : ui->plot->getTimeCursorPosition(Cursor2);
    double value1 = IS_FFT_INDEX(ch1) ? ui->plotFFT->getValueCursorPosition(Cursor1) : ui->plot->getValueCursorPosition(Cursor1);
    double value2 = IS_FFT_INDEX(ch2) ? ui->plotFFT->getValueCursorPosition(Cursor2) : ui->plot->getValueCursorPosition(Cursor2);
    double dt = time2 - time1;
    double dy = value2 - value1;
    // dB FFT
    if (IS_FFT_INDEX(ch1) && IS_FFT_INDEX(ch2) && ui->comboBoxFFTType->currentIndex() != FFTType::spectrum) {
      ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plotFFT->getXUnit());
      ui->labelCurDeltaValue->setText(QString::number(dy, 'f', 3).rightJustified(7) + " dB");
    }

    // linear FFT
    else if (IS_FFT_INDEX(ch1) && IS_FFT_INDEX(ch2) && ui->comboBoxFFTType->currentIndex() == FFTType::spectrum) {
      ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plotFFT->getXUnit());
      ui->labelCurDeltaValue->setText(floatToNiceString(dy, 4, true, true));
      ui->labelCurRatio->setText(floatToNiceString(value1 / value2, 4, true, true));
    }

    // Time
    else if (IS_ANALOG_OR_MATH_OR_LOGIC(ch1) && IS_ANALOG_OR_MATH_OR_LOGIC(ch2)) {
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
        ui->labelCurFreq->setText(floatToNiceString(1.0 / dt, 4, true, false) + ui->plotFFT->getXUnit());
      } else {
        ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, false) + ui->plot->getXUnit());
        ui->labelCurFreq->setText(floatToNiceString(1.0 / dt, 4, true, false) + ui->plotFFT->getXUnit());
      }
    }
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

  } else if (IS_FFT_INDEX(selectedChannel)) {
    // FFT
    double freq = ui->plotFFT->graph(INDEX_TO_FFT_CHID(selectedChannel))->data()->at(sample)->key;
    value = ui->plotFFT->graph(INDEX_TO_FFT_CHID(selectedChannel))->data()->at(sample)->value;

    timeStr = QString(floatToNiceString(freq, 5, true, false) + ui->plotFFT->getXUnit()).toUtf8();
    valueStr = (QString::number(value, 'f', 3).rightJustified(8) + " " + ui->plotFFT->getYUnit()).toUtf8();
    time = freq;
    if (useValueCursor) {
      ui->plotFFT->updateValueCursor(cursor, value, valueStr, ui->plotFFT->yAxis);
    }
    ui->plotFFT->updateTimeCursor(cursor, freq, timeStr, ui->plotFFT->graph(INDEX_TO_FFT_CHID(selectedChannel)));

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

void MainWindow::on_checkBoxCur1Visible_stateChanged(int arg1) {
  if (IS_LOGIC_INDEX(ui->comboBoxCursor1Channel->currentIndex()) && arg1 == Qt::CheckState::Checked) {
    ui->checkBoxCur1Visible->setCheckState(Qt::CheckState::Unchecked);
    return;
  }
  ui->doubleSpinBoxXCur1->setVisible(arg1 == Qt::CheckState::Checked);
  ui->horizontalSliderTimeCur1->setDisabled(arg1 == Qt::CheckState::Checked);
  ui->labelCur1Time->setHidden(arg1 == Qt::CheckState::Checked);
  ui->labelCur1Sample->setText("---");
  setCursorsVisibility(Cursor1, ui->comboBoxCursor1Channel->currentIndex(), arg1 != Qt::CheckState::Unchecked, ui->checkBoxYCur1->checkState());
  if (arg1 == Qt::CheckState::Checked) {
    if (ui->checkBoxYCur1->checkState() == Qt::CheckState::PartiallyChecked)
      ui->checkBoxYCur1->setCheckState(Qt::CheckState::Checked);
    if (IS_FFT_INDEX(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxXCur1->setValue(ui->plotFFT->getTimeCursorPosition(Cursor1));
    else if (IS_ANALOG_OR_MATH(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxXCur1->setValue(ui->plot->getTimeCursorPosition(Cursor1));
  } else if (arg1 == Qt::CheckState::PartiallyChecked)
    updateCursors();
}

void MainWindow::on_checkBoxCur2Visible_stateChanged(int arg1) {
  if (IS_LOGIC_INDEX(ui->comboBoxCursor2Channel->currentIndex()) && arg1 == Qt::CheckState::Checked) {
    ui->checkBoxCur2Visible->setCheckState(Qt::CheckState::PartiallyChecked);
    return;
  }

  ui->doubleSpinBoxXCur2->setVisible(arg1 == Qt::CheckState::Checked);
  ui->horizontalSliderTimeCur2->setDisabled(arg1 == Qt::CheckState::Checked);
  ui->labelCur2Time->setHidden(arg1 == Qt::CheckState::Checked);
  ui->labelCur2Sample->setText("---");
  setCursorsVisibility(Cursor2, ui->comboBoxCursor2Channel->currentIndex(), arg1 != Qt::CheckState::Unchecked, ui->checkBoxYCur2->checkState());
  if (arg1 == Qt::CheckState::Checked) {
    if (ui->checkBoxYCur2->checkState() == Qt::CheckState::PartiallyChecked)
      ui->checkBoxYCur2->setCheckState(Qt::CheckState::Checked);
    if (IS_FFT_INDEX(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxXCur2->setValue(ui->plotFFT->getTimeCursorPosition(Cursor2));
    else if (IS_ANALOG_OR_MATH(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxXCur2->setValue(ui->plot->getTimeCursorPosition(Cursor2));
  } else if (arg1 == Qt::CheckState::PartiallyChecked)
    updateCursors();
}

void MainWindow::on_comboBoxCursor1Channel_currentIndexChanged(int index) {
  setCursorsVisibility(Cursor1, index, ui->checkBoxCur1Visible->checkState() != Qt::CheckState::Unchecked, ui->checkBoxYCur1->checkState());
  if (IS_FFT_INDEX(index)) {
    ui->doubleSpinBoxXCur1->setSuffix(tr("Hz"));
    if (ui->comboBoxFFTType->currentIndex() == FFTType::spectrum)
      ui->doubleSpinBoxYCur1->setSuffix("");
    else
      ui->doubleSpinBoxYCur1->setSuffix("dB");
  } else {
    ui->doubleSpinBoxXCur1->setSuffix(tr("s"));
    ui->doubleSpinBoxYCur1->setSuffix(ui->lineEditVUnit->text());
  }

  ui->checkBoxYCur1->setHidden(IS_LOGIC_INDEX(index));

  if (ui->checkBoxYCur1->checkState() == Qt::CheckState::Checked)
    on_doubleSpinBoxYCur1_valueChanged(ui->doubleSpinBoxYCur1->value());
  updateCursorRange();
  updateCursors();
}

void MainWindow::on_comboBoxCursor2Channel_currentIndexChanged(int index) {
  setCursorsVisibility(Cursor2, index, ui->checkBoxCur2Visible->checkState() != Qt::CheckState::Unchecked, ui->checkBoxYCur2->checkState());
  if (IS_FFT_INDEX(index)) {
    ui->doubleSpinBoxXCur2->setSuffix(tr("Hz"));
    if (ui->comboBoxFFTType->currentIndex() == FFTType::spectrum)
      ui->doubleSpinBoxYCur2->setSuffix("");
    else
      ui->doubleSpinBoxYCur2->setSuffix("dB");
  } else {
    ui->doubleSpinBoxXCur2->setSuffix(tr("s"));
    ui->doubleSpinBoxYCur2->setSuffix(ui->lineEditVUnit->text());
  }

  ui->checkBoxYCur2->setHidden(IS_LOGIC_INDEX(index));

  if (ui->checkBoxYCur2->checkState() == Qt::CheckState::Checked)
    on_doubleSpinBoxYCur2_valueChanged(ui->doubleSpinBoxYCur2->value());
  updateCursorRange();
  updateCursors();
}
void MainWindow::horizontalSliderTimeCur1_realValueChanged(int arg1) {
  ui->labelCur1Sample->setText(QString::number(arg1));
  ui->checkBoxCur1Visible->setCheckState(Qt::CheckState::PartiallyChecked);
  updateCursors();
}

void MainWindow::horizontalSliderTimeCur2_realValueChanged(int arg1) {
  ui->labelCur2Sample->setText(QString::number(arg1));
  ui->checkBoxCur2Visible->setCheckState(Qt::CheckState::PartiallyChecked);
  updateCursors();
}

void MainWindow::timeCursorMovedByMouse(Cursors::enumCursors cursor, int sample, double value) {
  if (cursor == Cursor1) {
    if (ui->checkBoxCur1Visible->checkState() == Qt::CheckState::Checked)
      ui->doubleSpinBoxXCur1->setValue(value);
    else
      ui->horizontalSliderTimeCur1->setRealValue(sample);
  } else {
    if (ui->checkBoxCur2Visible->checkState() == Qt::CheckState::Checked)
      ui->doubleSpinBoxXCur2->setValue(value);
    else
      ui->horizontalSliderTimeCur2->setRealValue(sample);
  }
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
    ui->checkBoxCur1Visible->setCheckState(Qt::CheckState::PartiallyChecked);
    ui->checkBoxYCur1->setCheckState(Qt::CheckState::PartiallyChecked);
    ui->comboBoxCursor1Channel->setCurrentIndex(chid);
    on_comboBoxCursor1Channel_currentIndexChanged(chid); // Pokud byl předtím stejný, neaktualizovala by se viditelnost.
    ui->horizontalSliderTimeCur1->setRealValue(sample);
  } else {
    ui->checkBoxCur2Visible->setCheckState(Qt::CheckState::PartiallyChecked);
    ui->checkBoxYCur2->setCheckState(Qt::CheckState::PartiallyChecked);
    ui->comboBoxCursor2Channel->setCurrentIndex(chid);
    on_comboBoxCursor2Channel_currentIndexChanged(chid); // Pokud byl předtím stejný, neaktualizovala by se viditelnost.
    ui->horizontalSliderTimeCur2->setRealValue(sample);
  }
}

void MainWindow::offsetChangedByMouse(int chid) {
  if (ui->comboBoxSelectedChannel->currentIndex() == chid)
    ui->doubleSpinBoxChOffset->setValue(ui->plot->getChOffset(chid));
}

void MainWindow::on_checkBoxYCur1_stateChanged(int arg1) {
  if ((ui->checkBoxCur1Visible->checkState() != Qt::PartiallyChecked) && ui->checkBoxYCur1->checkState() == Qt::PartiallyChecked) {
    ui->checkBoxYCur1->setChecked(Qt::Checked);
    return;
  }

  ui->doubleSpinBoxYCur1->setVisible(arg1 == Qt::CheckState::Checked);
  ui->labelCur1Val->setHidden(arg1 == Qt::CheckState::Checked);
  setCursorsVisibility(Cursor1, ui->comboBoxCursor1Channel->currentIndex(), ui->checkBoxCur1Visible->checkState(), arg1);
  if (arg1 == Qt::CheckState::Checked) {
    if (IS_FFT_INDEX(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setValue(ui->plotFFT->getValueCursorPosition(Cursor1));
    else if (IS_ANALOG_OR_MATH(ui->comboBoxCursor1Channel->currentIndex()))
      ui->doubleSpinBoxYCur1->setValue(ui->plot->getValueCursorPosition(Cursor1));
    else
      ui->checkBoxYCur1->setCheckState(Qt::CheckState::Unchecked);
  } else if (arg1 == Qt::CheckState::PartiallyChecked)
    updateCursors();
}

void MainWindow::on_checkBoxYCur2_stateChanged(int arg1) {
  if (ui->checkBoxCur2Visible->checkState() != Qt::PartiallyChecked && ui->checkBoxYCur2->checkState() == Qt::PartiallyChecked) {
    ui->checkBoxYCur2->setChecked(Qt::Checked);
    return;
  }
  ui->doubleSpinBoxYCur2->setVisible(arg1 == Qt::CheckState::Checked);
  ui->labelCur2Val->setHidden(arg1 == Qt::CheckState::Checked);
  setCursorsVisibility(Cursor2, ui->comboBoxCursor2Channel->currentIndex(), ui->checkBoxCur2Visible->checkState(), arg1);
  if (arg1 == Qt::CheckState::Checked) {
    if (IS_FFT_INDEX(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setValue(ui->plotFFT->getValueCursorPosition(Cursor2));
    else if (IS_ANALOG_OR_MATH(ui->comboBoxCursor2Channel->currentIndex()))
      ui->doubleSpinBoxYCur2->setValue(ui->plot->getValueCursorPosition(Cursor2));
    else
      ui->checkBoxYCur2->setCheckState(Qt::CheckState::Unchecked);
  } else if (arg1 == Qt::CheckState::PartiallyChecked)
    updateCursors();
}

void MainWindow::setCursorsVisibility(Cursors::enumCursors cursor, int graph, int timeCurState, int valueCurState) {
  if (IS_FFT_INDEX(graph)) {
    ui->plot->setTimeCursorVisible(cursor, false);
    ui->plot->setValueCursorVisible(cursor, false);
    ui->plotFFT->setTimeCursorVisible(cursor, timeCurState);
    ui->plotFFT->setValueCursorVisible(cursor, valueCurState);
  } else if (IS_ANALOG_OR_MATH(graph)) {
    ui->plot->setTimeCursorVisible(cursor, timeCurState);
    ui->plot->setValueCursorVisible(cursor, valueCurState);
    ui->plotFFT->setTimeCursorVisible(cursor, false);
    ui->plotFFT->setValueCursorVisible(cursor, false);
  } else { // Logic
    ui->plot->setTimeCursorVisible(cursor, timeCurState);
    ui->plot->setValueCursorVisible(cursor, false);
    ui->plotFFT->setTimeCursorVisible(cursor, false);
    ui->plotFFT->setValueCursorVisible(cursor, false);
  }
}


void MainWindow::on_doubleSpinBoxYCur1_valueChanged(double arg1) {
  int chID = ui->comboBoxCursor1Channel->currentIndex();
  if (IS_FFT_INDEX(chID))
    ui->plotFFT->updateValueCursor(Cursor1, arg1, ui->doubleSpinBoxYCur1->text(), ui->plotFFT->yAxis);
  else
    ui->plot->updateValueCursor(Cursor1, arg1, ui->doubleSpinBoxYCur1->text(), ui->plot->getAnalogAxis(chID));
  updateCursors();
}

void MainWindow::on_doubleSpinBoxYCur2_valueChanged(double arg1) {
  int chID = ui->comboBoxCursor2Channel->currentIndex();
  if (IS_FFT_INDEX(chID))
    ui->plotFFT->updateValueCursor(Cursor2, arg1, ui->doubleSpinBoxYCur2->text(), ui->plotFFT->yAxis);
  else
    ui->plot->updateValueCursor(Cursor2, arg1, ui->doubleSpinBoxYCur2->text(), ui->plot->getAnalogAxis(chID));
  updateCursors();
}

void MainWindow::on_doubleSpinBoxXCur1_valueChanged(double arg1) {
  int chID = ui->comboBoxCursor1Channel->currentIndex();
  if (IS_FFT_INDEX(chID))
    ui->plotFFT->updateTimeCursor(Cursor1, arg1, ui->doubleSpinBoxYCur1->text(), ui->plotFFT->graph(INDEX_TO_FFT_CHID(chID)));
  else
    ui->plot->updateTimeCursor(Cursor1, arg1, ui->doubleSpinBoxYCur1->text(), ui->plot->graph(chID));
  updateCursors();
}

void MainWindow::on_doubleSpinBoxXCur2_valueChanged(double arg1) {
  int chID = ui->comboBoxCursor2Channel->currentIndex();
  if (IS_FFT_INDEX(chID))
    ui->plotFFT->updateTimeCursor(Cursor2, arg1, ui->doubleSpinBoxYCur2->text(), ui->plotFFT->graph(INDEX_TO_FFT_CHID(chID)));
  else
    ui->plot->updateTimeCursor(Cursor2, arg1, ui->doubleSpinBoxYCur2->text(), ui->plot->graph(chID));
  updateCursors();
}

