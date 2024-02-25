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

#include "communication/cobs.h"
#include "mainwindow.h"
#include "qml/ansiterminalmodel.h"
#include "qml/messagemodel.h"
#include "ui_developeroptions.h"

void MainWindow::initQmlTerminal() {
  qmlTerminalInterface = new QmlTerminalInterface();

  QQmlContext *context = ui->quickWidget->rootContext();
  context->setContextProperty("dataPlotter", qmlTerminalInterface);
  context->setContextProperty("messageModel", &messageModel);
  context->setContextProperty("ansiTerminalModel", &ansiTerminalModel);

  // Get the QQmlEngine instance from the widget
  QQmlEngine *engine = ui->quickWidget->engine();

  // Add the import path to the directory where the QML file is located
  engine->addImportPath(":/qml/GenericComponents");

  qmlRegisterType<MessageModel>("DataPlotter", 1, 0, "MessageModel");
  qmlRegisterType<AnsiTerminalModel>("DataPlotter", 1, 0, "ANSITerminalModel");

  engine->rootContext()->setContextProperty("_locale", locale());

  resetQmlTerminal();
}

void MainWindow::resetQmlTerminal() { loadQmlFile(QUrl::fromLocalFile(":/qml/DefaultQmlTerminal.qml")); }

void MainWindow::loadCompressedQml(QByteArray data) {
  data = COBS::decode(data);
  data = qUncompress(data);

  if (data.isEmpty()) {
    QMessageBox msgBox(this);
    msgBox.setText(tr("Cant load QML terminal."));
    msgBox.setInformativeText(tr("Data is corrupted or empty."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    return;
  }

  if (currentQmlFile.open()) {
    currentQmlFile.write(data);
    currentQmlFile.close();
  } else {
    QMessageBox msgBox(this);
    msgBox.setText(tr("QML terminal fatal error"));
    msgBox.setInformativeText(tr("Could not create temporary file."));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    return;
  }

  loadQmlFile(QUrl::fromLocalFile(currentQmlFile.fileName()));
}

void MainWindow::loadQmlFile(QUrl url) {
  developerOptions->getUi()->plainTextEditQmlLog->clear();
  developerOptions->getUi()->plainTextEditQmlLog->appendPlainText(url.toString());
  ui->quickWidget->setSource(QUrl());
  ui->quickWidget->engine()->clearComponentCache();
  ui->quickWidget->setSource(url);
}

void MainWindow::qmlDirectInput(QByteArray data) { qmlTerminalInterface->directInput(data); }

void MainWindow::setQmlProperty(QByteArray data) {
  QByteArrayList l = data.split(':');
  if (l.length() == 2) {
    auto ro = ui->quickWidget->rootObject();
    if (ro != nullptr)
      ro->setProperty(l.at(0), l.at(1));
  }
}
