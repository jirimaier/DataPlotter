#ifndef MYDIAL_H
#define MYDIAL_H

#include <QDebug>
#include <QDial>
#include <QObject>

#include "enums_defines_constants.h"

class myDial : public QDial {
  Q_OBJECT
public:
  explicit myDial(QWidget *parent = nullptr);

private:
  int lastPosition;
  bool var = false;
  int roundToStandardValue(double value);

private slots:
  void positionChanged(int position);

public slots:
  void updatePosition(double value);

signals:
  void realValueChanged(double);
};

#endif // MYDIAL_H
