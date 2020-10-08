#include "mydial.h"

myDial::myDial(QWidget *parent) : QDial(parent) {
  lastPosition = this->value();
  connect(this, &QDial::valueChanged, this, &myDial::positionChanged);
}

void myDial::positionChanged(int position) {
  if (lastPosition != position) {
    if (var) {
      if (position > lastPosition)
        position--;
      var = false;
    }
    if (position == this->maximum())
      position = this->maximum() - 1;
    lastPosition = position;
    this->setValue(position);
    emit realValueChanged(logaritmicSettings[position]);
  }
}

void myDial::updatePosition(double value) {
  value = (int)(value * 1000) / 1000.0;
  int newPos = roundToStandardValue(value);
  lastPosition = newPos;
  this->setValue(newPos);
  var = true;
  for (int i = 0; i < LOG_SET_SIZE; i++)
    if (value == logaritmicSettings[i])
      var = false;
}

int myDial::roundToStandardValue(double value) {
  for (int i = 0; i < LOG_SET_SIZE; i++)
    if (value <= logaritmicSettings[i])
      return i;
  return 30;
}
