#include "mydial.h"

myDial::myDial(QWidget *parent) : QDial(parent) {
  lastPosition = this->value();
  connect(this, &QDial::valueChanged, this, &myDial::positionChanged);
}

void myDial::positionChanged(int position) {
  if (var) {
    if (position > lastPosition)
      position--;
    var = false;
  }
  if (position == this->maximum())
    position = this->maximum() - 1;
  lastPosition = position;
  this->setValue(position);
  emit realValueChanged(Global::logaritmicSettings[position]);
}

void myDial::updatePosition(double value) {
  value = (int)(value * 1000) / 1000.0;
  int newPos = roundToStandardValue(value);
  lastPosition = newPos;
  blockSignals(true);
  this->setValue(newPos);
  blockSignals(false);
  var = true;
  for (int i = 0; i < LOG_SET_SIZE; i++)
    if (value == Global::logaritmicSettings[i])
      var = false;
}

int myDial::roundToStandardValue(double value) {
  for (int i = 0; i < LOG_SET_SIZE; i++)
    if (value <= Global::logaritmicSettings[i])
      return i;
  return 30;
}
