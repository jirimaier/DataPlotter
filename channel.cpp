#include "channel.h"

Channel::Channel() {}

void Channel::clear() {
  if (!this->isEmpty()) {
    time.clear();
    value.clear();
  }
}

double Channel::lastTime() {
  if (this->isEmpty())
    return 0;
  else
    return time.at(time.length() - 1);
}

void Channel::applyScaleAndOffset(double scale, double offset) {
  if (scale != 1) {
    for (QVector<double>::iterator it = value.begin(); it != value.end(); it++)
      (*it) *= scale;
  }
  if (offset != 0) {
    for (QVector<double>::iterator it = value.begin(); it != value.end(); it++)
      (*it) += offset;
  }
}

void Channel::addValue(double in_value, double in_time) {
  if (this->time.length() > 0)
    if (in_time <= this->time.at(this->time.length() - 1))
      clear();
  this->time.append(in_time);
  this->value.append(in_value);
  lastAddedTime = in_time;
}
