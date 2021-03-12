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

 private:
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
