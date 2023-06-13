#include "mypeakplot.h"

MyPeakPlot::MyPeakPlot(QWidget *parent) : MyPlot(parent) {
  this->yAxis->setNumberFormat("gb"); // Formát s hezkým 10^něco
  addGraph(xAxis, yAxis);
  addGraph(xAxis, yAxis);
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  yAxis2->setSubTicks(false);
  this->xAxis->setRange(0, 10);
  this->yAxis2->setRange(0, 1);
  this->yAxis->setRange(0, 1);
  setGridHintX(-3);
  setGridHintY(-3);
  infoText = new QCPItemText(this);
  infoText->position->setType(QCPItemPosition::ptViewportRatio);
  infoText->position->setCoords(0.5, 0.5);
  setInfoText();
  setXUnit("time", UnitMode::time);

  this->setInteraction(QCP::iRangeDrag, true);
  this->setInteraction(QCP::iRangeZoom, true);
}

QPair<QVector<double>, QVector<double>> MyPeakPlot::getDataVector(int chID) {
  QVector<double> keys, values;
  for (QCPGraphDataContainer::iterator it = graph(chID)->data()->begin(); it != graph(chID)->data()->end(); it++) {
    keys.append(it->key);
    values.append(it->value);
  }
  return QPair<QVector<double>, QVector<double>>(keys, values);
}

QByteArray MyPeakPlot::exportCSV(char separator, char decimal, int precision) {
  QByteArray output = "";
  QVector<QPair<QVector<double>, QVector<double>>> channels;
  bool firstNonEmpty = true;
  for (int i = 0; i < graphCount(); i++) {
    if (!graph(i)->data()->isEmpty()) {
      if (firstNonEmpty) {
        firstNonEmpty = false;
        output.append(tr("time").toUtf8());
      }
      channels.append(getDataVector(i));
      output.append(separator);
      output.append(getChName(chSourceChannel[i]).toUtf8());
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

QPair<unsigned int, unsigned int> MyPeakPlot::getVisibleSamplesRange(int chID) {
  if (graph(chID)->data()->isEmpty())
    return (QPair<unsigned int, unsigned int>(0, 0));
  unsigned int min = graph(chID)->findBegin(xAxis->range().lower, false);
  unsigned int max = graph(chID)->findEnd(xAxis->range().upper, false) - 1; // end je za posledním, snížit o 1
  return (QPair<unsigned int, unsigned int>(min, max));
}

bool MyPeakPlot::setChSorce(int ch, int sourceChannel, QColor color) {
  if (chSourceChannel[ch - 1] != sourceChannel) {
    clear(ch - 1);
    chSourceChannel[ch - 1] = sourceChannel;
  }
  if (chSourceColor[ch - 1] != color) {
    this->graph(ch - 1)->setPen(QPen(color));
    this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
    return true;
  }
  return false;
}

void MyPeakPlot::setInfoText() { infoText->setText(tr("Enable FFT and zoom-in on peak that you want to track")); }

void MyPeakPlot::newData(int chID, double freq) {
  infoText->setVisible(false);
  double timestamp = uptime->elapsed() / 1000.0;
  graph(chID)->addData(timestamp, freq);

  autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);

  // Přepsat text u traceru
  if (tracer->visible() && currentTracerIndex == chID) {
    updateTracerText(currentTracerIndex);
  }
}

void MyPeakPlot::clear(int ch) {
  graph(ch)->data().data()->clear();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyPeakPlot::clear() {
  graph(0)->data().data()->clear();
  graph(1)->data().data()->clear();
  infoText->setVisible(true);
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
  firstAutoset = true;
  mode = empty;
}

void MyPeakPlot::setStyle(int chID, int style) {
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

void MyPeakPlot::autoset() {
  bool foundrange1 = false, foundrange2 = false;

  QCPRange yRange1 = graph(0)->data()->valueRange(foundrange1);
  QCPRange yRange2 = graph(1)->data()->valueRange(foundrange2);
  if (foundrange1 && foundrange2)
    setMaxZoomY(QCPRange(floorToNiceValue(MIN(yRange1.lower * 0.9, yRange2.lower * 0.9)), ceilToNiceValue(MAX(yRange1.upper * 1.1, yRange2.upper * 1.1))), firstAutoset);
  else if (foundrange1)
    setMaxZoomY(QCPRange(floorToNiceValue(yRange1.lower * 0.9), ceilToNiceValue(yRange1.upper * 1.1)), firstAutoset);
  else if (foundrange2)
    setMaxZoomY(QCPRange(floorToNiceValue(yRange2.lower * 0.9), ceilToNiceValue(yRange2.upper * 1.1)), firstAutoset);
  else
    setMaxZoomY(QCPRange(0, 1));

  if (foundrange1 || foundrange2) {
    QCPRange xRange1 = graph(0)->data()->keyRange(foundrange1);
    QCPRange xRange2 = graph(1)->data()->keyRange(foundrange2);

    double xMax = 1, xMin = 0;
    if (foundrange1 && foundrange2) {
      xMax = MAX(xRange1.upper, xRange2.upper);
      xMin = MIN(xRange1.lower, xRange2.lower);
    } else if (foundrange1) {
      xMax = xRange1.upper;
      xMin = xRange1.lower;
    } else if (foundrange2) {
      xMax = xRange2.upper;
      xMin = xRange2.lower;
    }

    if (mode == empty) {
      mode = growing;
    } else if (mode == growing) {
      if (xMax > xAxis->range().upper) {
        mode = rolling;
        xAxis->setRange(xMax - xAxis->range().size(), xMax);
      }
    } else if (mode == free) {
      if (xMax < xAxis->range().upper)
        mode = growing;
    } else if (mode == free_locked) {
      mode = free;
    } else if (mode == rolling) {
      if (!qFuzzyCompare(xAxis->range().upper, lastSignalEnd))
        mode = free_locked;
      else
        xAxis->setRange(xMax - xAxis->range().size(), xMax);
    }

    lastSignalEnd = xMax;

    setMaxZoomX(QCPRange(xMin, xMax + xAxis->range().size()), firstAutoset || xMax > maxZoomX.upper || xMin < maxZoomX.lower);
    firstAutoset = false;
  }
}

void MyPeakPlot::setMouseCursorStyle(QMouseEvent *event) {
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

void MyPeakPlot::updateTracerText(int index) {
  QString tracerTextStr;
  tracerTextStr.append(" " + getChName(chSourceChannel[index]) + "\n");

  if (true) // TODO
    tracerTextStr.append(floatToNiceString(tracer->position->value(), 4, true, false) + getYUnit() + "\n");
  else
    tracerTextStr.append(QString::number(tracer->position->value(), 'g', 4) + " " + getYUnit() + "\n");

  tracerTextStr.append(floatToNiceString(tracer->position->key(), 4, true, false) + getXUnit());

  tracerText->setText(tracerTextStr);
  checkIfTracerTextFits();
  tracerLayer->replot();
}

void MyPeakPlot::mouseMoved(QMouseEvent *event) {
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

void MyPeakPlot::mousePressed(QMouseEvent *event) {
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
