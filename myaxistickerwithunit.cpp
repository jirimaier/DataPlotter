#include "myaxistickerwithunit.h"

QString MyAxisTickerWithUnit::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) {
  if (unit.isEmpty())
    return (locale.toString(tick, formatChar.toLatin1(), precision));
  else
    return (GlobalFunctions::floatToNiceString(tick, 2, false, false, false, 1) + unit);
}
