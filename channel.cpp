#include "channel.h"

Channel::Channel() {}

void Channel::clear() {
  if (!this->isEmpty()) {
    time.clear();
    value.clear();
  }
}

void Channel::addValue(double in_value, double in_time) {
  if (this->time.length() > 0)
    if (in_time <= this->time.at(this->time.length() - 1))
      clear();
  this->time.append(in_time);
  this->value.append(in_value);
}
