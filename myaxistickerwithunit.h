#ifndef MYAXISTICKERWITHUNIT_H
#define MYAXISTICKERWITHUNIT_H

#include "enums_defines_constants.h"
#include "qcustomplot.h"

class MyAxisTickerWithUnit : public QCPAxisTickerFixed {
public:
  QString unit = "";
  QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision);
};

#endif // MYAXISTICKERWITHUNIT_H
