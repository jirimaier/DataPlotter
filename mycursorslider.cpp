#include "mycursorslider.h"

myCursorSlider::myCursorSlider(QWidget *parent) : QSlider(parent) {
  realValue = value();
  connect(this, &QAbstractSlider::valueChanged, this, &myCursorSlider::positionChanged);
}

void myCursorSlider::updateRange(int min, int max) {
  blockSignals(true);
  setMinimum(min);
  setMaximum(max);
  setValue(realValue);
  if (realValue < min || realValue > max)
    setStyleSheet("QSlider::handle:horizontal {background-color:gray;}");
  else
    setStyleSheet("");
  setSingleStep(MAX((max - min) / 250, 1));
  setPageStep(MAX((max - min) / 250, 1));
  blockSignals(false);
}

void myCursorSlider::positionChanged(int newpos) {
  realValue = newpos;
  setStyleSheet("");
  emit realValueChanged(realValue);
}
