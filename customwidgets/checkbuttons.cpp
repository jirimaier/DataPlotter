#include "checkbuttons.h"

CheckButtons::CheckButtons(QWidget *parent) : QWidget(parent) {
  button1 = new QPushButton(this);
  button2 = new QPushButton(this);
  button3 = new QPushButton(this);

  button1->setCheckable(true);
  button2->setCheckable(true);
  button3->setCheckable(true);

  button1->setChecked(true);

  // Use a horizontal layout
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(button1);
  layout->addWidget(button2);
  layout->addWidget(button3);
  layout->setMargin(0);

  setLayout(layout);

  // Connect the button clicked signals to a slot
  connect(button1, &QPushButton::clicked, this, &CheckButtons::onButtonClicked);
  connect(button2, &QPushButton::clicked, this, &CheckButtons::onButtonClicked);
  connect(button3, &QPushButton::clicked, this, &CheckButtons::onButtonClicked);

  // Initialize the state
  updateButtonStates(button1);
}

void CheckButtons::setLabels(QString off, QString on) {
  button1->setText(off);
  button2->setText("");
  button3->setText(on);
  button2->setVisible(false);
}

void CheckButtons::setLabels(QString off, QString tri, QString on) {
  button1->setText(off);
  button2->setText(tri);
  button3->setText(on);
  button2->setVisible(true);
}

void CheckButtons::setChecked(bool checked) {
  if (checked)
    updateButtonStates(button3);
  else
    updateButtonStates(button1);
}

void CheckButtons::setCheckState(int state) {
  if (state == Qt::Unchecked) {
    updateButtonStates(button1);
  } else if (state == Qt::Checked) {
    updateButtonStates(button3);
  } else if (state == Qt::PartiallyChecked) {
    updateButtonStates(button2);
  }
}

void CheckButtons::onButtonClicked() {
  QPushButton *senderButton = qobject_cast<QPushButton *>(sender());
  updateButtonStates(senderButton);
}

void CheckButtons::updateButtonStates(QPushButton *checkedButton) {
  button1->setChecked(button1 == checkedButton);
  button2->setChecked(button2 == checkedButton);
  button3->setChecked(button3 == checkedButton);

  emit stateChanged(checkState());
}
