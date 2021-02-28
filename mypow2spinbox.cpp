#include "mypow2spinbox.h"

MyPow2Spinbox::MyPow2Spinbox(QWidget* parent): QSpinBox(parent) {

}

void MyPow2Spinbox::stepBy(int steps) {
  switch (steps) {
    case 1 :    setValue(value() * 2);
      break;
    case -1 :   setValue(value() / 2);
      break;
  }
}

int MyPow2Spinbox::valueFromText(const QString& text) const {
  return (nextPow2(QSpinBox::valueFromText(text)));
}

void MyPow2Spinbox::setValue(int val) {
  this->QSpinBox::setValue(nextPow2(val));
}
