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

#include "serialsettingsdialog.h"
#include "ui_serialsettingsdialog.h"

SerialSettingsDialog::SerialSettingsDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::SerialSettingsDialog) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->pushButtonApply, &QPushButton::clicked, this, &SerialSettingsDialog::apply);
    connect(ui->pushButtonDefault, &QPushButton::clicked, this, &SerialSettingsDialog::defaults);

    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);

    ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);

    defaults();
    updateSettings();
}

void SerialSettingsDialog::defaults() {
    ui->dataBitsBox->setCurrentIndex(3);
    ui->parityBox->setCurrentIndex(0);
    ui->stopBitsBox->setCurrentIndex(0);
    ui->flowControlBox->setCurrentIndex(0);
}

SerialSettingsDialog::~SerialSettingsDialog() {
    delete ui;
}

Ui::SerialSettingsDialog *SerialSettingsDialog::getUi()
{
    return ui;
}

SerialSettingsDialog::Settings SerialSettingsDialog::settings() const {
    return currentSettings;
}

void SerialSettingsDialog::updateSettings() {
    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    currentSettings.parity = static_cast<QSerialPort::Parity>(ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
}

void SerialSettingsDialog::apply() {
    updateSettings();
    emit settingChanged();
    hide();
}
