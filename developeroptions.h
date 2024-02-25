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

#ifndef DEVELOPEROPTIONS_H
#define DEVELOPEROPTIONS_H

#include "qlistwidget.h"
#include <QDialog>
#include <QQuickWidget>
#include <QUrl>

namespace Ui {
class DeveloperOptions;
}

class DeveloperOptions : public QDialog {
  Q_OBJECT

public:
  explicit DeveloperOptions(QWidget *parent, QQuickWidget *qQuickWidget);
  ~DeveloperOptions();

  Ui::DeveloperOptions *getUi();
  bool addColorToBlacklist(QByteArray code);
  void updateColorBlacklist();
  void addPathToList(QString fileName);

public slots:
  void addTerminalCursorPosCommand(int x, int y);

private slots:
  void on_pushButtonTerminalDebug_toggled(bool checked) { emit terminalDevToggled(checked); }
  void on_listWidgetTerminalCodeList_itemClicked(QListWidgetItem *item);
  void on_pushButtonTerminalDebugSend_clicked();
  void on_lineEditTerminalBlacklist_returnPressed();
  void on_pushButtonTerminalBlacklistClear_clicked();
  void on_lineEditTerminalBlacklist_textChanged(const QString &arg1);
  void on_pushButtonTerminalBlacklistCopy_clicked();
  void on_pushButtonTerminalBlacklisAddSelect_clicked();
  void on_pushButtonQmlReload_clicked() { qmlReload(); }
  void on_pushButtonQmlSave_clicked();
  void on_pushButtonQmlLoad_clicked();
  void on_pushButtonQmlExport_clicked() { qmlExport(); }
  void on_comboBoxTerminalColorListMode_currentIndexChanged(int index);
  void on_pushButtonTerminalDebugShift_clicked();
  void on_listWidgetQMLFiles_itemClicked(QListWidgetItem *item);
  void on_pushButtonClearBuffer_clicked() { emit requestSerialBufferClear(); }
  void on_pushButtonViewBuffer_clicked() { emit requestSerialBufferShow(); }
  void on_pushButtonClearAnsiTerminal_clicked();
  void on_pushButtonTerminalInputCopy_clicked();
  void on_textEditTerminalDebug_cursorPositionChanged();
  void on_lineEditManualInput_returnPressed();
  void on_pushButtonSendManual_clicked() { on_lineEditManualInput_returnPressed(); }
  void on_pushButtonClearBuffer_2_clicked() { emit requestManualBufferClear(); }
  void on_pushButtonViewBuffer_2_clicked() { emit requestManualBufferShow(); }
  void on_pushButtonScrollDown_2_clicked();
  void on_pushButtonOpenConfig_clicked();

signals:
  void colorExceptionListChanged(QList<QColor> newlist, bool isBlacklist);
  void loadQmlFile(QUrl url);
  void terminalDevToggled(bool);
  void printToTerminal(QByteArray);
  void requestSerialBufferClear();
  void requestSerialBufferShow();
  void requestManualBufferClear();
  void requestManualBufferShow();
  void sendManualInput(QByteArray bytes);

private:
  Ui::DeveloperOptions *ui;
  void insertInTerminalDebug(QString text, QColor textColor);
  void qmlReload();
  void qmlExport();

  const QQuickWidget *qQuickWidget;
  void quickWidget_statusChanged(const QQuickWidget::Status &arg1);
};

#endif // DEVELOPEROPTIONS_H
