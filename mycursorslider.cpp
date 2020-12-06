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
  setSingleStep(MAX((max - min) / 250, 1));
  setPageStep(MAX((max - min) / 250, 1));
  blockSignals(false);
}

void myCursorSlider::positionChanged(int newpos) {
  realValue = newpos;
  emit realValueChanged(realValue);
}
