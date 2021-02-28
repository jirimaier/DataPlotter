#include "myfftplot.h"

MyFFTPlot::MyFFTPlot(QWidget* parent) : MyPlot(parent) {
  addGraph(); addGraph();
  xAxis->setSubTicks(false);
  yAxis->setSubTicks(false);
  this->xAxis->setRange(0, 1000);
  this->yAxis->setRange(0, 100);
  setGridHintX(-3);
  setGridHintY(-3);
}

QByteArray MyFFTPlot::exportCSV(char separator, char decimal, int precision) {
  QByteArray output = (QString("frequency%1amplitude\n").arg(separator)).toUtf8();
  for (QCPGraphDataContainer::iterator it = graph(0)->data()->begin(); it != graph(0)->data()->end(); it++) {
    output.append(QString::number(it->key, 'f', precision).replace('.', decimal).toUtf8());
    output.append(separator);
    output.append(QString::number(it->value, 'f', precision).replace('.', decimal).toUtf8());
    output.append('\n');
  }
  return output;
}

QPair<unsigned int, unsigned int> MyFFTPlot::getVisibleSamplesRange(int ch) {
  if (graph(ch)->data()->isEmpty())
    return (QPair<unsigned int, unsigned int>(0, 0));
  int i = 0;
  while (graph(ch)->data()->at(i)->key < xAxis->range().lower)
    i++;
  int min = i;
  while (graph(ch)->data()->at(i)->key <= xAxis->range().upper && i < graph(ch)->data()->size())
    i++;
  int max = i - 1;
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

void MyFFTPlot::newData(int ch, QSharedPointer<QCPGraphDataContainer> data) {
  if (data->size() != graph(ch)->data()->size())
    emit lengthChanged(ch, graph(ch)->data()->size(), data->size());
  graph(ch)->setData(data);
  if (autoSize)
    autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);

  // Přepsat text u traceru
  if (tracer->visible() && currentTracerIndex == ch) {
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
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::setStyle(int ch, int style) {
  if (style == GraphStyle::line) {
    graph(ch)->setScatterStyle(QCPScatterStyle::ssNone);
    graph(ch)->setLineStyle(QCPGraph::lsLine);
    graph(ch)->setBrush(Qt::NoBrush);
  } else if (style == GraphStyle::point) {
    graph(ch)->setScatterStyle(POINT_STYLE);
    graph(ch)->setLineStyle(QCPGraph::lsNone);
    graph(ch)->setBrush(Qt::NoBrush);
  } else if (style == GraphStyle::linePoint) {
    graph(ch)->setScatterStyle(POINT_STYLE);
    graph(ch)->setLineStyle(QCPGraph::lsLine);
    graph(ch)->setBrush(Qt::NoBrush);
  }
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::setAutoSize(bool en) {
  autoSize = en;
  setMouseControlls(!en);
  autoset();
  this->replot(QCustomPlot::RefreshPriority::rpQueuedReplot);
}

void MyFFTPlot::autoset() {
  bool foundrange;
  QCPRange yRange = graph(0)->data()->valueRange(foundrange);
  QCPRange xRange = graph(0)->data()->keyRange(foundrange);
  if (yRange.lower >= 0)
    yRange.lower = 0;
  else
    yRange.lower = -50;

  yRange.upper = ceilToNiceValue(yRange.upper);
  xRange.lower = xRange.lower;
  xRange.upper = xRange.upper;
  xAxis->setRange(xRange);
  yAxis->setRange(yRange);
}

void MyFFTPlot::setMouseCursorStyle(QMouseEvent* event) {
  // Kursory svislé
  unsigned int cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsKey.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsKey.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsKey.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsKey.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist < 20 || cur2dist < 20) {
    this->QWidget::setCursor(Qt::SizeHorCursor); // Cursor myši, ne ten grafový
    return;
  }

  // Kursory vodorovné
  cur1dist = UINT_MAX, cur2dist = UINT_MAX;
  if (cursorsVal.at(Cursors::Cursor1)->visible())
    cur1dist = (unsigned int)cursorsVal.at(Cursors::Cursor1)->selectTest(event->pos(), false);
  if (cursorsVal.at(Cursors::Cursor2)->visible())
    cur2dist = (unsigned int)cursorsVal.at(Cursors::Cursor2)->selectTest(event->pos(), false);
  if (cur1dist < 20 || cur2dist < 20) {
    this->QWidget::setCursor(Qt::SizeVerCursor); // Cursor myši, ne ten grafový
    return;
  }

  // Nic
  this->QWidget::setCursor(Qt::ArrowCursor); // Cursor myši, ne ten grafový
}

void MyFFTPlot::updateTracerText(int index) {
  QString tracerTextStr;
  if (index == 0)
    tracerTextStr.append(" " + getChName(chSourceChannel[0]) + "\n");
  if (index == 1)
    tracerTextStr.append(getChName(chSourceChannel[1]) + "\n");
  tracerTextStr.append(floatToNiceString(tracer->position->value(), 4, true, false) + getYUnit() + "\n");
  tracerTextStr.append(floatToNiceString(tracer->position->key(), 4, true, false) + getXUnit());
  tracerText->setText(tracerTextStr);
  checkIfTracerTextFits();
  tracerLayer->replot();
}

void MyFFTPlot::mouseMoved(QMouseEvent* event) {
  if (mouseDrag == MouseDrag::nothing) {
    //Nic není taženo, zobrazí tracer

    // Najde nejbližší kanál k myši, pokud žádný není blíž než 20 pixelů, vůbec se nezobrazí
    int nearestIndex = -1;
    unsigned int nearestDistance = 20;
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
      this->QWidget::setCursor(Qt::ArrowCursor); // Cursor myši, ne ten grafový
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
      emit moveTimeCursor(Cursors::Cursor1, keyToNearestSample(cur1Graph, xAxis->pixelToCoord(event->pos().x())));
    if (mouseDrag == MouseDrag::cursorX2)
      emit moveTimeCursor(Cursors::Cursor2, keyToNearestSample(cur2Graph, xAxis->pixelToCoord(event->pos().x())));
  }
}

void MyFFTPlot::mousePressed(QMouseEvent* event) {
  // Kanál
  int nearestIndex = -1;
  unsigned int nearestDistance = 20;
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
      emit setCursorPos(FFTID(nearestIndex), Cursors::Cursor2, tracer->sampleNumber());
    } else {
      mouseDrag = MouseDrag::cursorX1;
      emit setCursorPos(FFTID(nearestIndex), Cursors::Cursor1, tracer->sampleNumber());
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
    if (cur1dist < 20) {
      mouseDrag = MouseDrag::cursorX1;
      this->setInteraction(QCP::iRangeDrag, false);
      return;
    }
  } else if (cur2dist < 20) {
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
    if (cur1dist < 20) {
      mouseDrag = MouseDrag::cursorY1;
      this->setInteraction(QCP::iRangeDrag, false);
      return;
    }
  } else if (cur2dist < 20) {
    mouseDrag = MouseDrag::cursorY2;
    this->setInteraction(QCP::iRangeDrag, false);
    return;
  }
}
