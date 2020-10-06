#ifndef MYDIAL_H
#define MYDIAL_H

#include <QDial>
#include <QObject>

#include "settings.h"

class myDial : public QDial {
  Q_OBJECT
public:
  explicit myDial(QWidget *parent = nullptr);

private:
  double realValue = 10000;

private slots:
  void positionChanged(int position);

signals:
  void realValueChanged(double);
};

#endif // MYDIAL_H
