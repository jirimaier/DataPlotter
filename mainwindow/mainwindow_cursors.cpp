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
  for (int n : {1, 2}) {
    auto hstc = ui->horizontalSliderTimeCur1;
    auto cbc = ui->comboBoxCursor1Channel;
    auto dsbx = ui->doubleSpinBoxXCur1;
    auto cbcv = ui->checkBoxCur1Visible;
    auto cur = Cursor1;

    if (n == 2) {
      hstc = ui->horizontalSliderTimeCur2;
      cbc = ui->comboBoxCursor2Channel;
      dsbx = ui->doubleSpinBoxXCur2;
      cbcv = ui->checkBoxCur2Visible;
      cur = Cursor2;
    }

    if (hstc->isEnabled()) {
      int ch = cbc->currentIndex();
      QPair<long, long> range;
      bool empty;
      if (IS_ANALOG_OR_MATH(ch)) {
        range = ui->plot->getChVisibleSamplesRange(ch);
        empty = ui->plot->graph(ch)->data()->isEmpty();
        dsbx->setSingleStep(ui->plot->xAxis->range().size() / 100);
      } else if (IS_FFT_INDEX(ch)) {
        range = ui->plotFFT->getVisibleSamplesRange(INDEX_TO_FFT_CHID(ch));
        empty = ui->plotFFT->graph(INDEX_TO_FFT_CHID(ch))->data()->isEmpty();
        dsbx->setSingleStep(ui->plotFFT->xAxis->range().size() / 100);
      } else {
        range = ui->plot->getChVisibleSamplesRange(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0));
        empty = ui->plot->graph(getLogicChannelID(CH_LIST_INDEX_TO_LOGIC_GROUP(ch), 0))->data()->isEmpty();
        dsbx->setSingleStep(ui->plot->xAxis->range().size() / 100);
      }
      hstc->updateRange(range.first, range.second);

      if (empty) {
        cbcv->setCheckState(Qt::Unchecked);
        if (IS_ANALOG_OR_MATH(ch)) {
          ui->plot->setTimeCursorVisible(cur, false);
          ui->plot->setValueCursorVisible(cur, false);
        } else {
          ui->plotFFT->setTimeCursorVisible(cur, false);
          ui->plotFFT->setValueCursorVisible(cur, false);
        }
      }
    }
  }
}

void MainWindow::updateCursors() {
  for (int n : {1, 2}) {
    auto cbcc = ui->comboBoxCursor1Channel;
    auto hstc = ui->horizontalSliderTimeCur1;
    auto cbyc = ui->checkBoxYCur1;
    auto lct = ui->labelCur1Time;
    auto lcv = ui->labelCur1Val;
    auto cbcv = ui->checkBoxCur1Visible;
    auto cursor = Cursor1;

    if (n == 2) {
      cbcc = ui->comboBoxCursor2Channel;
      hstc = ui->horizontalSliderTimeCur2;
      cbyc = ui->checkBoxYCur2;
      lct = ui->labelCur2Time;
      lcv = ui->labelCur2Val;
      cbcv = ui->checkBoxCur2Visible;
      cursor = Cursor2;
    }

    int ch = cbcc->currentIndex();

    double time = 0, value = 0;
    QByteArray timeStr, valueStr;

    if (cbcv->checkState() == Qt::CheckState::PartiallyChecked) {
      updateCursor(cursor, ch, hstc->getRealValue(), time, value, timeStr, valueStr, cbyc->checkState() == Qt::CheckState::PartiallyChecked);
      lct->setText(timeStr);
      lcv->setText(valueStr);
    } else {
      lct->setText("---");
      lcv->setText("---");
    }
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
      if (freqUseUnits)
        ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, false) + ui->plotFFT->getXUnit());
      else
        ui->labelCurDeltaTime->setText(QString::number(dt, 'g', 3));
      ui->labelCurDeltaValue->setText(QString::number(dy, 'f', 3).rightJustified(7) + " dB");
    }

    // linear FFT
    else if (IS_FFT_INDEX(ch1) && IS_FFT_INDEX(ch2) && ui->comboBoxFFTType->currentIndex() == FFTType::spectrum) {
      if (freqUseUnits)
        ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, false) + ui->plotFFT->getXUnit());
      else
        ui->labelCurDeltaTime->setText(QString::number(dt, 'g', 3));
      ui->labelCurDeltaValue->setText(floatToNiceString(dy, 4, true, true));
      ui->labelCurRatio->setText(floatToNiceString(value1 / value2, 4, true, true));
    }

    // Time
    else if (IS_ANALOG_OR_MATH_OR_LOGIC(ch1) && IS_ANALOG_OR_MATH_OR_LOGIC(ch2)) {
      if (IS_ANALOG_OR_MATH(ch1) && IS_ANALOG_OR_MATH(ch2)) {
        // Both analog
        if (timeUseUnits)
          ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, true) + ui->plot->getXUnit());
        else
          ui->labelCurDeltaTime->setText(QString::number(dt, 'g', 3));

        if (ui->plot->getValueCursorVisible(Cursor1) || ui->plot->getValueCursorVisible(Cursor2)) {
          QString YUnit = ui->plot->getYUnit();
          if (valuesUseUnits) {
            ui->labelCurDeltaValue->setText(floatToNiceString(dy, 4, true, true) + YUnit);
            ui->labelCurRatio->setText(floatToNiceString(value1 / value2, 4, true, true) + YUnit + "/" + YUnit);

            QString slope = floatToNiceString(dy / dt, 4, true, true);
            if (QString(" mnpfa" + QString::fromUtf8("\xc2\xb5")).contains(slope.at(slope.length() - 1)) || ui->plot->getXUnit().isEmpty()) {
              slope = slope + YUnit + "/" + (ui->plot->getXUnit().isEmpty() ? "1" : ui->plot->getXUnit());
            } else {
              // Předělání hodnot ve stylu "kV/s" na "V/ms"
              QChar prefix = slope.at(slope.length() - 1);
              slope.remove(slope.length() - 1, 1);

              if (prefix == 'k')
                prefix = 'm';
              else if (prefix == 'M')
                prefix = QString::fromUtf8("\xc2\xb5").at(0); // mikro
              else if (prefix == 'G')
                prefix = 'n';
              else if (prefix == 'T')
                prefix = 'p';
              else if (prefix == 'P')
                prefix = 'f';
              else if (prefix == 'E')
                prefix = 'a';

              slope = slope + YUnit + "/" + prefix + ui->plot->getXUnit();
            }

            ui->labelCurSlope->setText(slope);
          } else {
            ui->labelCurDeltaValue->setText(QString::number(dy, 'g', 3) + YUnit);
            ui->labelCurRatio->setText(QString::number(value1 / value2, 'g', 3) + (YUnit.isEmpty() ? "" : (YUnit + "/" + YUnit)));
            ui->labelCurSlope->setText(QString::number(dy / dt, 'g', 3) + (YUnit.isEmpty() && ui->plot->getXUnit().isEmpty() ? "" : (YUnit + "/" + ui->plot->getXUnit())));
          }
        }

        if (freqUseUnits)
          ui->labelCurFreq->setText(floatToNiceString(std::abs(1.0 / dt), 4, true, true) + ui->plotFFT->getXUnit());
        else
          ui->labelCurFreq->setText(QString::number(std::abs(1.0 / dt), 'g', 3));
      } else if (IS_ANALOG_OR_MATH(ch1) || IS_ANALOG_OR_MATH(ch2)) {
        // Analog and Logic
        if (timeUseUnits)
          ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, false) + ui->plot->getXUnit());
        else
          ui->labelCurDeltaTime->setText(QString::number(dt, 'g', 3));

        if (freqUseUnits)
          ui->labelCurFreq->setText(floatToNiceString(std::abs(1.0 / dt), 4, true, false) + ui->plotFFT->getXUnit());
        else
          ui->labelCurFreq->setText(QString::number(std::abs(1.0 / dt), 'g', 3));
      } else {
        if (timeUseUnits)
          ui->labelCurDeltaTime->setText(floatToNiceString(dt, 4, true, false) + ui->plot->getXUnit());
        else
          ui->labelCurDeltaTime->setText(QString::number(dt, 'g', 3));

        if (freqUseUnits)
          ui->labelCurFreq->setText(floatToNiceString(std::abs(1.0 / dt), 4, true, false) + ui->plotFFT->getXUnit());
        else
          ui->labelCurFreq->setText(QString::number(std::abs(1.0 / dt), 'g', 3));
      }
    }
  }
}

void MainWindow::updateCursor(Cursors::enumCursors cursor, int selectedChannel, unsigned int sample, double &time, double &value, QByteArray &timeStr, QByteArray &valueStr, bool useValueCursor) {
  if (IS_ANALOG_OR_MATH(selectedChannel)) {
    // Analogový kanál
    time = ui->plot->graph(selectedChannel)->data()->at(sample)->key;
    value = ui->plot->graph(selectedChannel)->data()->at(sample)->value;

    if (timeUseUnits)
      timeStr = QString(floatToNiceString(time, 5, true, false) + ui->plot->getXUnit()).toUtf8();
    else
      timeStr = QString(QString::number(time, 'g', 5)).toUtf8();

    QString unit = ui->plotxy->getYUnit();
    if (valuesUseUnits)
      valueStr = QString(floatToNiceString(value, 5, true, false) + unit).toUtf8();
    else
      valueStr = QString(QString::number(value, 'g', 5) + unit).toUtf8();

    if (useValueCursor) {
      ui->plot->updateValueCursor(cursor, value, valueStr, ui->plot->getAnalogAxis(selectedChannel));
    }
    ui->plot->updateTimeCursor(cursor, time, timeStr, selectedChannel);

  } else if (IS_FFT_INDEX(selectedChannel)) {
    // FFT
    double freq = ui->plotFFT->graph(INDEX_TO_FFT_CHID(selectedChannel))->data()->at(sample)->key;
    value = ui->plotFFT->graph(INDEX_TO_FFT_CHID(selectedChannel))->data()->at(sample)->value;

    if (freqUseUnits)
      timeStr = QString(floatToNiceString(freq, 5, true, false) + ui->plotFFT->getXUnit()).toUtf8();
    else
      timeStr = QString(QString::number(freq, 'g', 5)).toUtf8();

    valueStr = (QString::number(value, 'f', 3).rightJustified(8) + " " + ui->plotFFT->getYUnit()).toUtf8();
    time = freq;
    if (useValueCursor) {
      ui->plotFFT->updateValueCursor(cursor, value, valueStr, ui->plotFFT->yAxis);
    }
    ui->plotFFT->updateTimeCursor(cursor, freq, timeStr, INDEX_TO_FFT_CHID(selectedChannel));

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

    if (timeUseUnits)
      timeStr = QString(floatToNiceString(time, 5, true, false) + ui->plot->getXUnit()).toUtf8();
    else
      timeStr = QString(QString::number(time, 'g', 5)).toUtf8();

    valueStr = "0x" + QString::number((uint32_t)value, 16).toUpper().rightJustified(ceil(bitsUsed / 8.0) * 2, '0').toLocal8Bit();
    valueStr.append("\n" + bits);
    ui->plot->updateTimeCursor(cursor, time, valueStr + "\n" + timeStr, getLogicChannelID(group, 0));
  }
}

void MainWindow::on_checkBoxCurXXXVisible_stateChanged(int n, int arg1) {
  Q_ASSERT(n == 1 || n == 2);

  auto cbcc = ui->comboBoxCursor1Channel;
  auto cbcv = ui->checkBoxCur1Visible;
  auto cbyc = ui->checkBoxYCur1;
  auto dsbxc = ui->doubleSpinBoxXCur1;
  auto hstc = ui->horizontalSliderTimeCur1;
  auto lct = ui->labelCur1Time;
  auto lcs = ui->labelCur1Time;
  auto cur = Cursor1;

  if (n == 2) {
    cbcc = ui->comboBoxCursor2Channel;
    cbcv = ui->checkBoxCur2Visible;
    cbyc = ui->checkBoxYCur2;
    dsbxc = ui->doubleSpinBoxXCur2;
    hstc = ui->horizontalSliderTimeCur2;
    lct = ui->labelCur2Time;
    lcs = ui->labelCur2Time;
    cur = Cursor2;
  }

  int selectedChannel = cbcc->currentIndex();

  if (!IS_ANALOG_OR_MATH(selectedChannel) && selectedChannel != CURSOR_ABSOLUTE && arg1 == Qt::CheckState::Checked) {
    cbcv->setCheckState(Qt::CheckState::Unchecked);
    return;
  }

  if (selectedChannel == CURSOR_ABSOLUTE && arg1 == Qt::PartiallyChecked) {
    cbcv->setCheckState(Qt::CheckState::Checked);
    return;
  }

  if (cbyc->checkState() == Qt::PartiallyChecked && arg1 == Qt::Unchecked)
    cbyc->setCheckState(Qt::Unchecked);

  dsbxc->setVisible(arg1 == Qt::CheckState::Checked);
  hstc->setDisabled(arg1 == Qt::CheckState::Checked || selectedChannel == CURSOR_ABSOLUTE);
  lct->setHidden(arg1 == Qt::CheckState::Checked);
  lcs->setText("---");
  setCursorsVisibility(cur, selectedChannel, arg1 != Qt::CheckState::Unchecked, cbyc->checkState());
  if (arg1 == Qt::CheckState::Checked) {
    if (cbyc->checkState() == Qt::CheckState::PartiallyChecked)
      cbyc->setCheckState(Qt::CheckState::Checked);
    if (IS_FFT_INDEX(selectedChannel))
      dsbxc->setValue(ui->plotFFT->getTimeCursorPosition(cur));
    else if (IS_ANALOG_OR_MATH(selectedChannel))
      dsbxc->setValue(ui->plot->getTimeCursorPosition(cur));
  } else if (arg1 == Qt::CheckState::PartiallyChecked)
    updateCursors();
}

void MainWindow::on_comboBoxCursorXXXChannel_currentIndexChanged(int n, int index) {
  Q_ASSERT(n == 1 || n == 2);

  auto cbcv = ui->checkBoxCur1Visible;
  auto cbyc = ui->checkBoxYCur1;
  auto dsbxc = ui->doubleSpinBoxXCur1;
  auto hstc = ui->horizontalSliderTimeCur1;
  auto dsby = ui->doubleSpinBoxYCur1;
  auto cur = Cursor1;

  if (n == 2) {
    cbcv = ui->checkBoxCur2Visible;
    cbyc = ui->checkBoxYCur2;
    dsbxc = ui->doubleSpinBoxXCur2;
    hstc = ui->horizontalSliderTimeCur2;
    dsby = ui->doubleSpinBoxYCur2;
    cur = Cursor2;
  }

  hstc->setEnabled(cbcv->checkState() != Qt::Checked && index != CURSOR_ABSOLUTE);

  if (index == CURSOR_ABSOLUTE && cbcv->checkState() == Qt::PartiallyChecked)
    cbcv->setCheckState(Qt::CheckState::Checked);

  setCursorsVisibility(cur, index, cbcv->checkState() != Qt::CheckState::Unchecked, cbyc->checkState());
  if (IS_FFT_INDEX(index)) {
    dsbxc->setSuffix(tr("Hz"));
    cbcv->setText(tr("Frequency"));
    if (ui->comboBoxFFTType->currentIndex() == FFTType::spectrum)
      dsby->setSuffix("");
    else
      dsby->setSuffix("dB");
  } else {
    cbcv->setText(tr("Time"));
    dsbxc->setSuffix(tr("s"));
    dsby->setSuffix(ui->lineEditVUnit->text());
  }

  cbyc->setHidden(IS_LOGIC_INDEX(index));

  if (cbyc->checkState() == Qt::CheckState::Checked)
    on_doubleSpinBoxYCurXXX_valueChanged(n, dsby->value());
  updateCursorRange();
  updateCursors();
}

void MainWindow::horizontalSliderTimeCurXXX_realValueChanged(int n, int arg1) {
  Q_ASSERT(n == 1 || n == 2);

  auto lcs = ui->labelCur1Sample;
  auto cbcv = ui->checkBoxCur1Visible;

  if (n == 2) {
    lcs = ui->labelCur2Sample;
    cbcv = ui->checkBoxCur2Visible;
  }

  lcs->setText(QString::number(arg1));
  cbcv->setCheckState(Qt::CheckState::PartiallyChecked);
  updateCursors();
}

void MainWindow::timeCursorMovedByMouse(Cursors::enumCursors cursor, int sample, double value) {
  auto cbcv = ui->checkBoxCur1Visible;
  auto hstc = ui->horizontalSliderTimeCur1;
  auto dsbc = ui->doubleSpinBoxXCur1;

  if (cursor == Cursor2) {
    cbcv = ui->checkBoxCur2Visible;
    hstc = ui->horizontalSliderTimeCur2;
    dsbc = ui->doubleSpinBoxXCur2;
  }

  if (cbcv->checkState() == Qt::CheckState::Checked)
    dsbc->setValue(value);
  else
    hstc->setRealValue(sample);
}

void MainWindow::valueCursorMovedByMouse(enumCursors cursor, double value) {
  auto cbyc = ui->checkBoxYCur1;
  auto dsby = ui->doubleSpinBoxYCur1;

  if (cursor == Cursor2) {
    cbyc = ui->checkBoxYCur2;
    dsby = ui->doubleSpinBoxYCur2;
  }

  if (cbyc->checkState() != Qt::CheckState::Checked)
    cbyc->setCheckState(Qt::CheckState::Checked);
  dsby->setValue(value);
}

void MainWindow::cursorSetByMouse(int chid, Cursors::enumCursors cursor, int sample) {
  int n = (cursor == Cursor1 ? 1 : 2);

  auto cbcc = ui->comboBoxCursor1Channel;
  auto cbcv = ui->checkBoxCur1Visible;
  auto cbyc = ui->checkBoxYCur1;
  auto hstc = ui->horizontalSliderTimeCur1;

  if (n == 2) {
    cbcc = ui->comboBoxCursor2Channel;
    cbcv = ui->checkBoxCur2Visible;
    cbyc = ui->checkBoxYCur2;
    hstc = ui->horizontalSliderTimeCur2;
  }

  // Zvolený kanál se změní tady, aby se nastavení checkboxů řídilo pravidly
  // pro typ nově zvoleného kanálu Ale funkce vyvolaná změnou kanálu je
  // vyvolána až na konec
  cbcc->blockSignals(true);
  cbcc->setCurrentIndex(chid);
  cbcc->blockSignals(false);

  cbcv->setCheckState(Qt::CheckState::PartiallyChecked);
  cbyc->setCheckState(Qt::CheckState::PartiallyChecked);
  hstc->setRealValue(sample);

  on_comboBoxCursorXXXChannel_currentIndexChanged(n, chid);
}

void MainWindow::offsetChangedByMouse(int chid) {
  if (ui->comboBoxSelectedChannel->currentIndex() == chid)
    ui->doubleSpinBoxChOffset->setValue(ui->plot->getChOffset(chid));
}

void MainWindow::on_checkBoxYCurXXX_stateChanged(int n, int arg1) {
  Q_ASSERT(n == 1 || n == 2);

  auto cbcc = ui->comboBoxCursor1Channel;
  auto cbcv = ui->checkBoxCur1Visible;
  auto cbyc = ui->checkBoxYCur1;
  auto lcv = ui->labelCur1Val;
  auto dsby = ui->doubleSpinBoxYCur1;
  auto cur = Cursor1;

  if (n == 2) {
    cbcc = ui->comboBoxCursor2Channel;
    cbcv = ui->checkBoxCur2Visible;
    cbyc = ui->checkBoxYCur2;
    lcv = ui->labelCur2Val;
    dsby = ui->doubleSpinBoxYCur2;
    cur = Cursor2;
  }

  int selectedChannel = cbcc->currentIndex();

  if ((cbcv->checkState() != Qt::PartiallyChecked) && cbyc->checkState() == Qt::PartiallyChecked) {
    cbyc->setCheckState(Qt::Checked);
    return;
  }

  dsby->setVisible(arg1 == Qt::CheckState::Checked);
  lcv->setHidden(arg1 == Qt::CheckState::Checked);
  setCursorsVisibility(cur, selectedChannel, cbcv->checkState(), arg1);
  if (arg1 == Qt::CheckState::Checked) {
    if (IS_FFT_INDEX(selectedChannel))
      dsby->setValue(ui->plotFFT->getValueCursorPosition(cur));
    else if (IS_ANALOG_OR_MATH(selectedChannel) || selectedChannel == CURSOR_ABSOLUTE)
      dsby->setValue(ui->plot->getValueCursorPosition(cur));
    else
      cbyc->setCheckState(Qt::CheckState::Unchecked);
  } else if (arg1 == Qt::CheckState::PartiallyChecked)
    updateCursors();
}

void MainWindow::setCursorsVisibility(Cursors::enumCursors cursor, int graph, int timeCurState, int valueCurState) {
  if (IS_FFT_INDEX(graph)) {
    ui->plot->setTimeCursorVisible(cursor, false);
    ui->plot->setValueCursorVisible(cursor, false);
    ui->plotFFT->setTimeCursorVisible(cursor, timeCurState);
    ui->plotFFT->setValueCursorVisible(cursor, valueCurState);
  } else if (IS_ANALOG_OR_MATH(graph) || graph == CURSOR_ABSOLUTE) {
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

void MainWindow::on_doubleSpinBoxYCurXXX_valueChanged(int n, double arg1) {
  Q_ASSERT(n == 1 || n == 2);

  auto dsbyc = ui->doubleSpinBoxYCur1;
  auto cbcc = ui->comboBoxCursor1Channel;
  auto cur = Cursor1;

  if (n == 2) {
    dsbyc = ui->doubleSpinBoxYCur2;
    cbcc = ui->comboBoxCursor2Channel;
    cur = Cursor2;
  }

  int chID = cbcc->currentIndex();
  if (IS_FFT_INDEX(chID))
    ui->plotFFT->updateValueCursor(cur, arg1, dsbyc->text(), ui->plotFFT->yAxis);
  else
    ui->plot->updateValueCursor(cur, arg1, dsbyc->text(), chID == CURSOR_ABSOLUTE ? ui->plot->yAxis : ui->plot->getAnalogAxis(chID));
  updateCursors();
}

void MainWindow::on_doubleSpinBoxXCurXXX_valueChanged(int n, double arg1) {
  Q_ASSERT(n == 1 || n == 2);

  auto dsbxc = ui->doubleSpinBoxXCur1;
  auto cbcc = ui->comboBoxCursor1Channel;
  auto cur = Cursor1;

  if (n == 2) {
    dsbxc = ui->doubleSpinBoxXCur2;
    cbcc = ui->comboBoxCursor2Channel;
    cur = Cursor2;
  }

  int chID = cbcc->currentIndex();
  if (IS_FFT_INDEX(chID))
    ui->plotFFT->updateTimeCursor(cur, arg1, dsbxc->text(), INDEX_TO_FFT_CHID(chID));
  else if (chID == CURSOR_ABSOLUTE)
    ui->plot->updateTimeCursor(cur, arg1, dsbxc->text(), -1);
  else
    ui->plot->updateTimeCursor(cur, arg1, dsbxc->text(), chID);
  updateCursors();
}
