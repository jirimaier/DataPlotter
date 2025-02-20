#ifndef CHECKBUTTONS_H
#define CHECKBUTTONS_H

#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

class CheckButtons : public QWidget {
  Q_OBJECT

public:
  explicit CheckButtons(QWidget *parent = nullptr);
  void setLabels(QString off, QString on);
  void setLabels(QString off, QString tri, QString on);
  bool isChecked() { return !button1->isChecked(); }
  void setChecked(bool checked);
  Qt::CheckState checkState() { return button1->isChecked() ? Qt::Unchecked : button2->isChecked() ? Qt::PartiallyChecked : Qt::Checked; }
  void setCheckState(int state);

private slots:
  void onButtonClicked();

private:
  QPushButton *button1;
  QPushButton *button2;
  QPushButton *button3;

  void updateButtonStates(QPushButton *checkedButton);

signals:
  void stateChanged(int selection);
};

#endif // CHECKBUTTONS_H
