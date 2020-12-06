#ifndef MYCURSORSLIDER_H
#define MYCURSORSLIDER_H

#include <QObject>
#include <QSlider>

#include "enums_defines_constants.h"

class myCursorSlider : public QSlider {
  Q_OBJECT
public:
  explicit myCursorSlider(QWidget *parent = nullptr);
  int realValue;
  void updateRange(int min, int max);

private slots:
  void positionChanged(int newpos);

signals:
  void realValueChanged(int value);
};

#endif // MYCURSORSLIDER_H
