//  Copyright (C) 2020-2024  Jiří Maier

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "manualinputdialog.h"
#include "qdebug.h"
#include "ui_manualinputdialog.h"

void ManualInputDialog::initRollingTable() {
  initTable(*ui->tableWidgetRollingSetup);
  setExprRows(ui->tableWidgetRollingSetup, 3);
  ui->tableWidgetRollingSetup->setItem(0, 0, new QTableWidgetItem("5*sin(2*Pi*t)"));
  ui->tableWidgetRollingSetup->setItem(1, 0, new QTableWidgetItem("2*cos(2*Pi*t) + 0.1*random()*sin(100*t)"));

  initTable(*ui->tableWidgetOscSetup);
  setExprRows(ui->tableWidgetOscSetup, 3);
  ui->tableWidgetOscSetup->setItem(0, 0, new QTableWidgetItem("5*sin(2*Pi*1e3*t)"));
  ui->tableWidgetOscSetup->setItem(1, 0, new QTableWidgetItem("2*cos(2*Pi*2e3*t + time)"));
}

ManualInputDialog::ManualInputDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ManualInputDialog) {
  ui->setupUi(this);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  iconRun = QIcon(":/images/icons/run.png");
  iconPause = QIcon(":/images/icons/pause.png");
  ui->pushButtonRolling->setIcon(iconPause);
  connect(&rollingTimer, &QTimer::timeout, this, &ManualInputDialog::rollingDataTimerRoutine);
  ui->pushButtonOsc->setIcon(iconPause);
  connect(&oscTimer, &QTimer::timeout, this, &ManualInputDialog::oscDataTimerRoutine);

  rollingEngine.globalObject().setProperty("t", 0);
  oscEngine.globalObject().setProperty("t", 0);
  oscEngine.globalObject().setProperty("time", 0);

#if QT_VERSION > QT_VERSION_CHECK(5, 10, 0)
  ui->doubleSpinBoxRollingFreq->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
#endif

  initRollingTable();
}

ManualInputDialog::~ManualInputDialog() {
  rollingTimer.deleteLater();
  delete ui;
}

void ManualInputDialog::stopAll() {
  rollingTimer.stop();
  ui->pushButtonRolling->setIcon(iconPause);
}

Ui::ManualInputDialog *ManualInputDialog::getUi() const { return ui; }

void ManualInputDialog::on_doubleSpinBoxRollingFreq_valueChanged(double arg1) { rollingTimer.setInterval(1000 / arg1); }

void ManualInputDialog::on_pushButtonRolling_clicked() {
  if (rollingTimer.isActive())
    rollingTimer.stop();
  else
    rollingTimer.start(1000 / ui->doubleSpinBoxRollingFreq->value());
  ui->pushButtonRolling->setIcon(rollingTimer.isActive() ? iconRun : iconPause);
}

void ManualInputDialog::on_pushButtonOsc_clicked() {
  if (oscTimer.isActive())
    oscTimer.stop();
  else
    oscTimer.start(100);
  ui->pushButtonOsc->setIcon(oscTimer.isActive() ? iconRun : iconPause);
}

void ManualInputDialog::rollingDataTimerRoutine() {
  rollingTimestamp += static_cast<double>(rollingTimer.interval()) / 1000.0 * ui->doubleSpinBoxTimeScale->value();
  QString data = "$$P" + QString::number(rollingTimestamp, 'g', 10);
  rollingEngine.globalObject().setProperty("t", rollingTimestamp);
  for (auto &ch : rollingChannelEvaluators) {
    bool isOk;
    double val = ch->evaluate(rollingEngine, &isOk);
    if (isOk)
      data.append("," + QString::number(val));
    else
      data.append(",-");
  }
  data.append(";");
  emit sendManualInput(data.toLocal8Bit());
}

void ManualInputDialog::oscDataTimerRoutine() {
  oscTimestamp += static_cast<double>(oscTimer.interval()) / 1000.0;
  int len = ui->spinBoxOscLen->value();
  double fs = ui->doubleSpinBoxOscFs->value() * 1000;
  oscEngine.globalObject().setProperty("time", oscTimestamp);
  int chnum = 1;
  for (auto &ch : oscChannelEvaluators) {
    bool isOk;
    QByteArray data = "$$C";
    data.append(QString::number(chnum++).toLocal8Bit() + ",");
    data.append(QString::number(1.0 / fs).toLocal8Bit() + ",");
    data.append(QString::number(len).toLocal8Bit() + ";f4");

    for (int i = 0; i < len; i++) {
      oscEngine.globalObject().setProperty("t", static_cast<double>(i) / fs);
      float value = ch->evaluate(oscEngine, &isOk);
      data.append(reinterpret_cast<char *>(&value), 4);
    }

    data.append(";");
    if (isOk)
      emit sendManualInput(data);
  }
}

void ManualInputDialog::on_tableWidgetRollingSetup_cellChanged(int row, int column) {
  if (column == 0) {
    auto txt = ui->tableWidgetRollingSetup->item(row, column)->text();
    bool ok = rollingChannelEvaluators.at(row)->setExpression(rollingEngine, txt);
    QTableWidgetItem *item = new QTableWidgetItem(ok ? "OK" : txt.isEmpty() ? "Empty" : "Error");
    item->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    ui->tableWidgetRollingSetup->setItem(row, 1, item);
  }
}

void ManualInputDialog::on_tableWidgetOscSetup_cellChanged(int row, int column) {
  if (column == 0) {
    auto txt = ui->tableWidgetOscSetup->item(row, column)->text();
    bool ok = oscChannelEvaluators.at(row)->setExpression(oscEngine, txt);
    QTableWidgetItem *item = new QTableWidgetItem(ok ? "OK" : txt.isEmpty() ? "Empty" : "Error");
    item->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    ui->tableWidgetOscSetup->setItem(row, 1, item);
  }
}

void ManualInputDialog::setExprRows(QTableWidget *table, int rows) {
  auto *evaluators = &rollingChannelEvaluators;
  if (table == ui->tableWidgetOscSetup)
    evaluators = &oscChannelEvaluators;

  while (table->rowCount() != rows) {
    int count = table->rowCount();
    if (rows > count) {
      table->insertRow(count - 1);
      QTableWidgetItem *item = new QTableWidgetItem("Empty");
      item->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
      table->setItem(count - 1, 1, item);
      evaluators->append(new VariableExpressionParser());
    } else {
      table->removeRow(count - 2);
      delete evaluators->last();
      evaluators->removeLast();
    }
  }

  QStringList names;
  for (int i = 1; i < table->rowCount(); i++)
    names.append(QString("Ch%1").arg(i));
  names.append("");
  table->setVerticalHeaderLabels(names);
}

void ManualInputDialog::on_pushButtonRollingResetTime_clicked() { rollingTimestamp = 0; }

void ManualInputDialog::initTable(QTableWidget &table) {
  table.setRowCount(1);
  QHeaderView *headerView = table.horizontalHeader();
  headerView->setSectionResizeMode(QHeaderView::Stretch);
  headerView->setSectionResizeMode(1, QHeaderView::ResizeToContents);

  QPushButton *addRowButton = new QPushButton(QIcon(":/images/icons/plus.png"), "", &table);
  QPushButton *removeRowButton = new QPushButton(QIcon(":/images/icons/minus.png"), "", &table);

  QHBoxLayout *layout = new QHBoxLayout();
  layout->addWidget(addRowButton);
  layout->addWidget(removeRowButton);
  layout->setAlignment(Qt::AlignCenter);
  layout->setContentsMargins(0, 0, 0, 0);

  // Create a new QWidget object to contain the layout
  QWidget *widget = new QWidget();
  widget->setLayout(layout);
  widget->setFocusPolicy(Qt::NoFocus);

  // Set the widget as the cell's item
  table.setCellWidget(0, 0, widget);

  QTableWidgetItem *item = new QTableWidgetItem("");
  item->setFlags(Qt::NoItemFlags);
  table.setItem(0, 1, item);

  connect(addRowButton, &QPushButton::clicked, this, [this, &table]() { this->setExprRows(&table, table.rowCount() + 1); });
  connect(removeRowButton, &QPushButton::clicked, this, [this, &table]() {
    if (table.rowCount() > 2)
      this->setExprRows(&table, table.rowCount() - 1);
  });
}
