#ifndef MYPOW2SPINBOX_H
#define MYPOW2SPINBOX_H

#include <QObject>
#include <QSpinBox>

#include "enums_defines_constants.h"

class MyPow2Spinbox : public QSpinBox {
  Q_OBJECT
 public:
  explicit MyPow2Spinbox(QWidget* parent = nullptr);

 public slots:
  void stepBy(int steps);
  void setValue(int val);

 private:
  int valueFromText(const QString& text) const;

 signals:

};

#endif // MYPOW2SPINBOX_H
