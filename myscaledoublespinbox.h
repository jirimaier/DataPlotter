#ifndef MYSCALEDOUBLESPINBOX_H
#define MYSCALEDOUBLESPINBOX_H

#include "enumsDefinesConstants.h"
#include <QDoubleSpinBox>
#include <QWidget>

#define MULTIPLY QChar((uint16_t)0x00D7)
#define DIVIDE QChar((uint16_t)0x00F7)

class MyScaleDoubleSpinBox : public QDoubleSpinBox {
  Q_OBJECT
 public:
  explicit MyScaleDoubleSpinBox(QWidget* parent = nullptr);

 public slots:
  void stepBy(int steps);

 private:
  QValidator::State validate(QString& input, int& pos) const;
  QString textFromValue(double val) const;
  double valueFromText(const QString& text) const;

 signals:

};

#endif // MYSCALEDOUBLESPINBOX_H
