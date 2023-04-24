//  Copyright (C) 2020-2021  Jiří Maier

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

#ifndef SERIALSETTINGSDIALOG_H
#define SERIALSETTINGSDIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QTranslator>

namespace Ui {
class SerialSettingsDialog;
}

class SerialSettingsDialog : public QDialog {
  Q_OBJECT

 public:
  Ui::SerialSettingsDialog* ui;

 public:
  explicit SerialSettingsDialog(QWidget* parent = nullptr);
  ~SerialSettingsDialog();

  struct Settings {
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
  };

  Settings settings() const;

  void retranslate();

 private slots:
  void updateSettings();
  void apply();
  void defaults();

 private:
  Settings currentSettings;

 signals:
  void settingChanged();
};

#endif // SERIALSETTINGSDIALOG_H
