#include "mydial.h"

myDial::myDial(QWidget *parent) : QDial(parent) { connect(this, &QDial::valueChanged, this, &myDial::positionChanged); }

void myDial::positionChanged(int position) { emit realValueChanged(logaritmicSettings[position]); }
