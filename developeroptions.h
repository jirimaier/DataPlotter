#ifndef DEVELOPEROPTIONS_H
#define DEVELOPEROPTIONS_H

#include <QDialog>
#include <QQuickWidget>
#include <QUrl>
#include "qlistwidget.h"

namespace Ui {
class DeveloperOptions;
}

class DeveloperOptions : public QDialog {
  Q_OBJECT

 public:
  explicit DeveloperOptions(QWidget* parent, QQuickWidget* qQuickWidget);
  ~DeveloperOptions();

  Ui::DeveloperOptions* getUi();
  bool addColorToBlacklist(QByteArray code);
  void updateColorBlacklist();
  void addPathToList(QString fileName);

 public slots:
  void addTerminalCursorPosCommand(int x, int y);

 private slots:
  void on_pushButtonTerminalDebug_toggled(bool checked) {
    emit terminalDevToggled(checked);
  }
  void on_listWidgetTerminalCodeList_itemClicked(QListWidgetItem* item);
  void on_pushButtonTerminalDebugSend_clicked();
  void on_lineEditTerminalBlacklist_returnPressed();
  void on_pushButtonTerminalBlacklistClear_clicked();
  void on_lineEditTerminalBlacklist_textChanged(const QString& arg1);
  void on_pushButtonTerminalBlacklistCopy_clicked();
  void on_pushButtonTerminalBlacklisAddSelect_clicked();
  void on_pushButtonQmlReload_clicked() { emit qmlReload(); }
  void on_pushButtonQmlSave_clicked();
  void on_pushButtonQmlLoad_clicked();
  void on_pushButtonQmlExport_clicked() { emit qmlExport(); }
  void on_comboBoxTerminalColorListMode_currentIndexChanged(int index);
  void on_pushButtonTerminalDebugShift_clicked();
  void on_listWidgetQMLFiles_itemClicked(QListWidgetItem* item);
  void on_pushButtonClearBuffer_clicked() { emit requestSerialBufferClear(); }
  void on_pushButtonViewBuffer_clicked() { emit requestSerialBufferShow(); }
  void on_pushButtonClearAnsiTerminal_clicked();
  void on_pushButtonTerminalInputCopy_clicked();
  void on_textEditTerminalDebug_cursorPositionChanged();
  void on_lineEditManualInput_returnPressed();
  void on_pushButtonSendManual_clicked() {
    on_lineEditManualInput_returnPressed();
  }
  void on_pushButtonClearBuffer_2_clicked() { emit requestManualBufferClear(); }
  void on_pushButtonViewBuffer_2_clicked() { emit requestManualBufferShow(); }
  void on_pushButtonScrollDown_2_clicked();

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
  Ui::DeveloperOptions* ui;
  void insertInTerminalDebug(QString text, QColor textColor);
  void qmlReload();
  void qmlExport();

  const QQuickWidget* qQuickWidget;
  void quickWidget_statusChanged(const QQuickWidget::Status& arg1);
};

#endif  // DEVELOPEROPTIONS_H
