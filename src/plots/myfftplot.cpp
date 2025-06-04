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

#include "myfftplot.h"

MyFFTPlot::MyFFTPlot(QWidget *parent) : MyPlot(parent) {
  addGraph();
  addGraph();
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  autoset();
  setGridHintX(-3);
  setGridHintY(-3);
  this->setInteraction(QCP::iRangeDrag, true);
  this->setInteraction(QCP::iRangeZoom, true);
}

QPair<QVector<double>, QVector<double>> MyFFTPlot::getDataVector(int chID) {
  QVector<double> keys, values;
  for (QCPGraphDataContainer::iterator it = graph(chID)->data()->begin(); it != graph(chID)->data()->end(); it++) {
    keys.append(it->key);
    values.append(it->value);
  }
  return QPair<QVector<double>, QVector<double>>(keys, values);
}

QByteArray MyFFTPlot::exportCSV(char separator, char decimal, int precision) {
  // Stejné jako export všech kanálů v hlavním grafu.
  // označením "time" se zde myslí frekvence (prostě souřadnice X)

  QByteArray output = "";
  QVector<QPair<QVector<double>, QVector<double>>> channels;
  bool firstNonEmpty = true;
  for (int i = 0; i < graphCount(); i++) {
    if (!graph(i)->data()->isEmpty()) {
      if (firstNonEmpty) {
        firstNonEmpty = false;
        output.append(tr("frequency").toUtf8());
      }
      channels.append(getDataVector(i));
      output.append(separator);
      output.append(QString(getChName(chSourceChannel[i])).toUtf8());
    }
  }
  QList<double> times;
  for (QVector<QPair<QVector<double>, QVector<double>>>::iterator it = channels.begin(); it != channels.end(); it++)
    foreach (double time, it->first)
      if (!times.contains(time))
        times.append(time);
  std::sort(times.begin(), times.end());

  foreach (double time, times) {
    output.append('\n');
    output.append(QString::number(time, 'f', precision).replace('.', decimal).toUtf8());
    for (QVector<QPair<QVector<double>, QVector<double>>>::iterator it = channels.begin(); it != channels.end(); it++) {
      output.append(separator);
      if (!it->first.isEmpty())
        if (it->first.first() == time) {
          output.append(QString::number(it->second.first(), 'f', precision).replace('.', decimal).toUtf8());
          it->first.pop_front();
          it->second.pop_front();
        }
    }
  }
  return output;
}

QPair<unsigned int, unsigned int> MyFFTPlot::getVisibleSamplesRange(int chID) {
  if (graph(chID)->data()->isEmpty())
    return (QPair<unsigned int, unsigned int>(0, 0));
  unsigned int min = graph(chID)->findBegin(xAxis->range().lower, false);
  unsigned int max = graph(chID)->findEnd(xAxis->range().upper, false) - 1; // end je za posledním, snížit o 1
  return (QPair<unsigned int, unsigned int>(min, max));
}

bool MyFFTPlot::setChSorce(int ch, int sourceChannel, QColor color) {
  chSourceChannel[ch - 1] = sourceChannel;
  if (chSourceColor[ch - 1] != color) {
    this->graph(ch - 1)->setPen(QPen(color));
    this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
    return true;
  }
  return false;
}

void MyFFTPlot::newData(int chID, QSharedPointer<QCPGraphDataContainer> data) {
  // Pokud se změní počet vzorků, kursor by se posunul mimo původní pozici (z hlediska polohy na ose), protože se drží
  // indexu vzorku, v případě změny se tedy kursor přesune na nový vzorek, který je nejblíže původní poloze
  if (data->size() != graph(chID)->data()->size()) {
    double cur1ShouldBeAtKey = 0, cur2ShouldBeAtKey = 0;
    if (cursorsKey[Cursors::Cursor1]->visible() && cur1Graph == chID)
      cur1ShouldBeAtKey = cursorsKey[Cursors::Cursor1]->start->coords().x();
    if (cursorsKey[Cursors::Cursor2]->visible() && cur2Graph == chID)
      cur2ShouldBeAtKey = cursorsKey[Cursors::Cursor2]->start->coords().x();

    graph(chID)->setData(data);

    if (cursorsKey[Cursors::Cursor1]->visible() && cur1Graph == chID)
      emit moveTimeCursor(Cursors::Cursor1, cur1Graph == -1 ? 0 : keyToNearestSample(graph(cur1Graph), cur1ShouldBeAtKey), cur1ShouldBeAtKey);
    if (cursorsKey[Cursors::Cursor2]->visible() && cur2Graph == chID)
      emit moveTimeCursor(Cursors::Cursor2, cur2Graph == -1 ? 0 : keyToNearestSample(graph(cur2Graph), cur2ShouldBeAtKey), cur2ShouldBeAtKey);

  } else {
    int len = data->size();
    if (holdmax[chID] && data->at(len - 1)->key == graph(chID)->data()->at(len - 1)->key) {
      auto dataNew = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
      for (int i = 0; i < len; i++)
        dataNew->add(QCPGraphData(data->at(i)->key, std::max(data->at(i)->value, graph(chID)->data()->at(i)->value)));
      graph(chID)->setData(dataNew);
    } else
      graph(chID)->setData(data);
  }

  autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);

  if (outputPeakValue) {
    double peakAmp = -Q_INFINITY, peakFreq = 0;
    for (int i = graph(chID)->findBegin(xAxis->range().lower, false); i < graph(chID)->findEnd(xAxis->range().upper, false) - 1; i++) {
      if (graph(chID)->data()->at(i)->value > peakAmp) {
        peakAmp = graph(chID)->data()->at(i)->value;
        peakFreq = graph(chID)->data()->at(i)->key;
      }
    }
    emit newPeakValues(chID, peakFreq);
  }

  // Přepsat text u traceru
  if (tracer->visible() && currentTracerIndex == chID) {
    updateTracerText(currentTracerIndex);
  }
}

void MyFFTPlot::clear(int ch) {
  graph(ch)->data().data()->clear();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::clear() {
  graph(0)->data().data()->clear();
  graph(1)->data().data()->clear();
  firstAutoset = true;
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::setStyle(int chID, int style) {
  if (style == GraphStyle::line) {
    graph(chID)->setScatterStyle(QCPScatterStyle::ssNone);
    graph(chID)->setLineStyle(QCPGraph::lsLine);
    graph(chID)->setBrush(Qt::NoBrush);
  } else if (style == GraphStyle::point) {
    graph(chID)->setScatterStyle(POINT_STYLE);
    graph(chID)->setLineStyle(QCPGraph::lsNone);
    graph(chID)->setBrush(Qt::NoBrush);
  } else if (style == GraphStyle::linePoint) {
    graph(chID)->setScatterStyle(POINT_STYLE);
    graph(chID)->setLineStyle(QCPGraph::lsLine);
    graph(chID)->setBrush(Qt::NoBrush);
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::setOutputPeakValue(bool newOutputPeakValue) { outputPeakValue = newOutputPeakValue; }

void MyFFTPlot::autoset() {
  QList<int> yMaxs, xMaxs, xMins;

  for (int i = 0; i < 2; i++) {
    bool foundrange = false;
    QCPRange yRange = graph(i)->data()->valueRange(foundrange);
    QCPRange xRange = graph(i)->data()->keyRange(foundrange);
    if (foundrange) {
      yMaxs.append(yRange.upper);
      xMins.append(xRange.lower);
      xMaxs.append(xRange.upper);
    }
  }

  if (xMaxs.isEmpty()) {
    this->xAxis->setRange(0, 1000);
    this->yAxis->setRange(0, 100);
    setMaxZoomX(xAxis->range());
    setMaxZoomY(yAxis->range());
    return;
  }

  double yMax = *std::max_element(yMaxs.begin(), yMaxs.end());
  double yMin;
  double xMax = *std::max_element(xMaxs.begin(), xMaxs.end());
  double xMin = *std::max_element(xMins.begin(), xMins.end());

  if (getYUnit().isDecibell()) {
    yMax = ceil(yMax / 10.0) * 10.0 + 20;
    yMin = yMax - 120;
  } else {
    yMax = ceilToNiceValue(1.5 * yMax);
    yMin = 0;
  }

  setMaxZoomX(QCPRange(xMin, xMax), firstAutoset || xMax > maxZoomX.upper || xMin < -maxZoomX.lower);
  setMaxZoomY(QCPRange(yMin, yMax), firstAutoset);
  firstAutoset = false;
}

void MyFFTPlot::setMouseCursorStyle(QMouseEvent *event) {
  // Kursory svislé
  unsigned int cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsKey.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsKey.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsKey.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsKey.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE || cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    this->QWidget::setCursor(Qt::SizeHorCursor); // Cursor myši, ne ten grafový
    return;
  }

  // Kursory vodorovné
  cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsVal.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsVal.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsVal.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsVal.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE || cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    this->QWidget::setCursor(Qt::SizeVerCursor); // Cursor myši, ne ten grafový
    return;
  }

  // Nic
  this->QWidget::setCursor(defaultMouseCursor); // Cursor myši, ne ten grafový
}

void MyFFTPlot::updateTracerText(int index) {
  QString tracerTextStr;
  tracerTextStr.append(getChName(chSourceChannel[index]) + "\n");

  tracerTextStr.append(floatToNiceString(tracer->position->value(), 4, true, false, false, getYUnit()) + "\n");
  tracerTextStr.append(floatToNiceString(tracer->position->key(), 4, true, false, false, getXUnit()));

  tracerText->setText(tracerTextStr);
  checkIfTracerTextFits();
  tracerLayer->replot();
}

void MyFFTPlot::mouseMoved(QMouseEvent *event) {
  if (mouseDrag == MouseDrag::nothing) {
    // Nic není taženo, zobrazí tracer

    // Najde nejbližší kanál k myši, pokud žádný není blíž než TRACER_MOUSE_DISTANCE pixelů, vůbec se nezobrazí
    int nearestIndex = -1;
    unsigned int nearestDistance = TRACER_MOUSE_DISTANCE;
    for (int i = 0; i < graphCount(); i++) {
      if (graph(i)->visible()) {
        unsigned int distance = (unsigned int)graph(i)->selectTest(event->pos(), false);
        if (distance < nearestDistance) {
          nearestIndex = i;
          nearestDistance = distance;
        }
      }
    }

    if (nearestIndex != -1) { // Myš je na grafu
      tracer->setVisible(true);
      tracerText->setVisible(true);
      tracer->setGraph(graph(nearestIndex));
      tracer->setYAxis(graph(nearestIndex)->valueAxis());
      tracer->setPoint(event->pos());
      updateTracerText(nearestIndex);
      currentTracerIndex = nearestIndex;
      this->QWidget::setCursor(defaultMouseCursor); // Cursor myši, ne ten grafový
    } else {
      if (tracer->visible())
        hideTracer();
      setMouseCursorStyle(event);
    }
  } else {
    if (tracer->visible())
      hideTracer();

    if (mouseDrag == MouseDrag::cursorY1)
      emit moveValueCursor(Cursors::Cursor1, yAxis->pixelToCoord(event->pos().y()));
    else if (mouseDrag == MouseDrag::cursorY2)
      emit moveValueCursor(Cursors::Cursor2, yAxis->pixelToCoord(event->pos().y()));
    if (mouseDrag == MouseDrag::cursorX1)
      emit moveTimeCursor(Cursors::Cursor1, cur1Graph == -1 ? 0 : keyToNearestSample(graph(cur1Graph), xAxis->pixelToCoord(event->pos().x())), xAxis->pixelToCoord(event->pos().x()));
    if (mouseDrag == MouseDrag::cursorX2)
      emit moveTimeCursor(Cursors::Cursor2, cur2Graph == -1 ? 0 : keyToNearestSample(graph(cur2Graph), xAxis->pixelToCoord(event->pos().x())), xAxis->pixelToCoord(event->pos().x()));
  }
}

void MyFFTPlot::mousePressed(QMouseEvent *event) {
  // Kanál
  int nearestIndex = -1;
  unsigned int nearestDistance = TRACER_MOUSE_DISTANCE;
  for (int i = 0; i < graphCount(); i++) {
    if (graph(i)->visible()) {
      unsigned int distance = (unsigned int)graph(i)->selectTest(event->pos(), false);
      if (distance < nearestDistance) {
        nearestIndex = i;
        nearestDistance = distance;
      }
    }
  }

  if (nearestIndex != -1) {
    tracer->setGraph(graph(nearestIndex));
    tracer->setPoint(event->pos());
    tracer->updatePosition();
    if (event->button() == Qt::RightButton) {
      mouseDrag = MouseDrag::cursorX2;
      this->setInteraction(QCP::iRangeDrag, false);
      emit setCursorPos(FFT_INDEX(nearestIndex), Cursors::Cursor2, tracer->sampleNumber());
    } else {
      mouseDrag = MouseDrag::cursorX1;
      this->setInteraction(QCP::iRangeDrag, false);
      emit setCursorPos(FFT_INDEX(nearestIndex), Cursors::Cursor1, tracer->sampleNumber());
    }
    return;
  }

  // Kursory svislé
  unsigned int cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsKey.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsKey.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsKey.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsKey.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist <= cur2dist) {
    if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
      mouseDrag = MouseDrag::cursorX1;
      this->setInteraction(QCP::iRangeDrag, false);
      return;
    }
  } else if (cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    mouseDrag = MouseDrag::cursorX2;
    this->setInteraction(QCP::iRangeDrag, false);
    return;
  }

  // Kursory vodorovné
  cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsVal.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsVal.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsVal.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsVal.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist <= cur2dist) {
    if (cur1dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
      mouseDrag = MouseDrag::cursorY1;
      this->setInteraction(QCP::iRangeDrag, false);
      return;
    }
  } else if (cur2dist < PLOT_ELEMENTS_MOUSE_DISTANCE) {
    mouseDrag = MouseDrag::cursorY2;
    this->setInteraction(QCP::iRangeDrag, false);
    return;
  }
}
