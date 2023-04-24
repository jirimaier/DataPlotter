#ifndef DEVELOPEROPTIONS_H
#define DEVELOPEROPTIONS_H

#include "qlistwidget.h"
#include <QDialog>
#include <QUrl>
#include <QQuickWidget>

namespace Ui {
class DeveloperOptions;
}

class DeveloperOptions : public QDialog
{
    Q_OBJECT

public:
    explicit DeveloperOptions(QQuickWidget *qQuickWidget, QWidget *parent = nullptr);
    ~DeveloperOptions();

    Ui::DeveloperOptions *getUi() const;
    bool addColorToBlacklist(QByteArray code);
    void updateColorBlacklist();

    void addPathToList(QString fileName);

    void retranslate();
private slots:
    void on_pushButtonTerminalDebug_toggled(bool checked) { emit terminalDevToggled(checked); }
    void on_listWidgetTerminalCodeList_itemClicked(QListWidgetItem* item);
    void on_pushButtonTerminalDebugSend_clicked();
    void on_textEditTerminalDebug_cursorPositionChanged();
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
    void on_listWidgetQMLFiles_itemClicked(QListWidgetItem *item);

signals:
    void colorExceptionListChanged(QList<QColor> newlist, bool isBlacklist);
    void loadQmlFile(QUrl url);
    void terminalDevToggled(bool);
    void printToTerminal(QByteArray);

private:
    Ui::DeveloperOptions *ui;
    void insertInTerminalDebug(QString text, QColor textColor);
    void qmlReload();
    void qmlExport();

    const QQuickWidget *qQuickWidget;
    void quickWidget_statusChanged(const QQuickWidget::Status &arg1);
};

#endif // DEVELOPEROPTIONS_H
