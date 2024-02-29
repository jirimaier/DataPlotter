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

#include "developeroptions.h"
#include "communication/cobs.h"
#include "defaultpathmanager.h"
#include "qcheckbox.h"
#include "qclipboard.h"
#include "qdebug.h"
#include "qmessagebox.h"
#include "qml/ansiterminalmodel.h"
#include "qqmlerror.h"
#include "qscrollbar.h"
#include "ui_developeroptions.h"
#include <QColorDialog>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>

QString addSpacesToCamelCase(const QString &input) {
  QString output;
  for (int i = 0; i < input.length(); ++i) {
    const QChar currentChar = input.at(i);
    if (i > 0 && currentChar.isUpper()) {
      output += ' ';
    }
    output += currentChar;
  }
  return output;
}

DeveloperOptions::DeveloperOptions(QWidget *parent, QQuickWidget *qQuickWidget) : QDialog(parent), ui(new Ui::DeveloperOptions), qQuickWidget(qQuickWidget) {
  ui->setupUi(this);

  ui->tabWidget->setCurrentIndex(0);

  Q_ASSERT(qQuickWidget != nullptr);
  connect(qQuickWidget, &QQuickWidget::statusChanged, this, &DeveloperOptions::quickWidget_statusChanged);

  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  addColorToBlacklist("40");
  updateColorBlacklist();

  QDir dir(":/qml/");
  const QFileInfoList files = dir.entryInfoList(QStringList() << "*.qml", QDir::Files);

  for (const auto &file : files) {
    auto newItem = new QListWidgetItem();
    newItem->setText(addSpacesToCamelCase(file.fileName().mid(file.fileName().lastIndexOf('/') + 1).replace(".qml", "")).replace("qml", "QML", Qt::CaseInsensitive));
    newItem->setText(newItem->text().replace("Windows X P", "Windows XP"));
    newItem->setText(newItem->text().replace("Win X P", "WinXP"));
    newItem->setData(Qt::UserRole, file.absoluteFilePath());
    ui->listWidgetQMLFiles->addItem(newItem);
  }
}

DeveloperOptions::~DeveloperOptions() { delete ui; }

Ui::DeveloperOptions *DeveloperOptions::getUi() { return ui; }

void DeveloperOptions::on_lineEditTerminalBlacklist_returnPressed() {
  if (addColorToBlacklist(ui->lineEditTerminalBlacklist->text().toLocal8Bit().trimmed())) {
    ui->lineEditTerminalBlacklist->clear();
    ui->lineEditTerminalBlacklist->setStyleSheet("");
    updateColorBlacklist();
  } else
    ui->lineEditTerminalBlacklist->setStyleSheet("color: rgb(255, 0, 0);");
}

bool DeveloperOptions::addColorToBlacklist(QByteArray code) {
  QColor clr;
  bool valid = false;
  code.replace("\u001b", "");
  code.replace("\\u001b", "");
  code.replace("\\e", "");
  code.replace("[", "");
  code.replace("m", "");
  if (code.at(0) == '3')
    code.replace(0, 1, "4");

  valid = AnsiTerminalModel::colorFromSequence(code, clr);

  if (valid) {
    QPixmap colour = QPixmap(12, 12);
    colour.fill(clr);
    ui->listWidgetTerminalBlacklist->addItem(new QListWidgetItem(QIcon(colour), code, ui->listWidgetTerminalBlacklist));
  }
  return valid;
}

void DeveloperOptions::updateColorBlacklist() {
  QList<QColor> list;
  for (int i = 0; i < ui->listWidgetTerminalBlacklist->count(); i++) {
    QPixmap pixmap = ui->listWidgetTerminalBlacklist->item(i)->icon().pixmap(1, 1);
    list.append(pixmap.toImage().pixel(0, 0));
  }

  emit colorExceptionListChanged(list, ui->comboBoxTerminalColorListMode->currentIndex() == 0);
}

void DeveloperOptions::on_pushButtonTerminalBlacklistClear_clicked() {
  auto selection = ui->listWidgetTerminalBlacklist->selectedItems();
  if (selection.isEmpty())
    ui->listWidgetTerminalBlacklist->clear();
  else {
    foreach (QListWidgetItem *item, ui->listWidgetTerminalBlacklist->selectedItems()) {
      delete ui->listWidgetTerminalBlacklist->takeItem(ui->listWidgetTerminalBlacklist->row(item));
    }
  }
  updateColorBlacklist();
}

void DeveloperOptions::on_lineEditTerminalBlacklist_textChanged(const QString &arg1) {
  if (arg1.isEmpty())
    ui->lineEditTerminalBlacklist->setStyleSheet("");
}

void DeveloperOptions::on_pushButtonTerminalBlacklistCopy_clicked() {
  QClipboard *clipboard = QGuiApplication ::clipboard();
  QString settingsEntry;
  settingsEntry.append(ui->comboBoxTerminalColorListMode->currentIndex() == 0 ? "noclickclr:" : "clickclr:");
  for (int i = 0; i < ui->listWidgetTerminalBlacklist->count(); i++)
    settingsEntry.append(ui->listWidgetTerminalBlacklist->item(i)->text().toLocal8Bit().replace(';', '.') + ',');
  settingsEntry.remove(settingsEntry.length() - 1, 1);
  settingsEntry.append(";\n");
  clipboard->setText(settingsEntry);
}

void DeveloperOptions::on_pushButtonTerminalDebugShift_clicked() {
  bool ok;
  int i = QInputDialog::getInt(this, "", tr("Shift content verticaly"), 0, -1000, 1000, 1, &ok, Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
  if (!ok)
    return;

  QString input = ui->textEditTerminalDebug->toPlainText();
  QString output = "";
  static QRegularExpression re("\\\\e\\[\\d+;\\d+H");
  while (true) {
    QRegularExpressionMatch match = re.match(input);
    if (match.hasMatch()) {
      output.append(input.leftRef(match.capturedStart()));
      QString a = match.captured();
      a = a.mid(3, a.length() - 4);
      auto b = a.split(';');
      unsigned int c = b.first().toUInt(&ok);
      unsigned int d = b.last().toUInt(&ok);
      if (!ok)
        return;
      output.append(QString("\\e[%1;%2H").arg(c + i).arg(d));
      input.remove(0, match.capturedStart() + match.capturedLength());
    } else {
      output.append(input);
      break;
    }
  }
  ui->textEditTerminalDebug->setPlainText(output);
  on_pushButtonTerminalDebugSend_clicked();
}

void DeveloperOptions::on_comboBoxTerminalColorListMode_currentIndexChanged(int index) {
  Q_UNUSED(index);
  updateColorBlacklist();
}

void DeveloperOptions::on_pushButtonQmlLoad_clicked() {
  QString fileName = DefaultPathManager::getInstance().requestOpenFile(this, tr("Load file"), "path_lqml", tr("Qml file (*.qml);;Any file (*.*)"));
  if (fileName.isEmpty())
    return;

  addPathToList(fileName);

  emit loadQmlFile(QUrl::fromLocalFile(fileName));
}

void DeveloperOptions::on_pushButtonTerminalDebugSend_clicked() {
  QByteArray data = ui->textEditTerminalDebug->toPlainText().toUtf8();
  data.replace("\n", "\r\n"); // Odřádkování v textovém poli
  data.replace("\\n", "\n");
  data.replace("\\e", "\u001b");
  data.replace("\\r", "\r");
  data.replace("\\t", "\t");
  data.replace("\\b", "\b");
  data.replace("\\a", "\a");

  emit printToTerminal(data);
}

void DeveloperOptions::on_textEditTerminalDebug_cursorPositionChanged() {
  if (ui->textEditTerminalDebug->textCursor().selectedText().isEmpty())
    ui->textEditTerminalDebug->setTextColor(ui->textEditTerminalDebug->palette().color(QPalette::Text));
}

void DeveloperOptions::on_pushButtonTerminalBlacklisAddSelect_clicked() {
  QColor color = QColorDialog::getColor(Qt::black);
  if (!color.isValid())
    return;
  QByteArray colorCode = AnsiTerminalModel::nearestColorCode(color);
  addColorToBlacklist(QString("\\e[3" + colorCode + "m").toUtf8());
  updateColorBlacklist();
}

void DeveloperOptions::addPathToList(QString fileName) {
  auto toremove = ui->listWidgetQMLFiles->findItems(fileName, Qt::MatchExactly);
  for (auto item : qAsConst(toremove)) {
    delete ui->listWidgetQMLFiles->takeItem(ui->listWidgetQMLFiles->row(item));
  }

  auto newItem = new QListWidgetItem();
  newItem->setText(fileName);
  newItem->setData(Qt::UserRole, fileName);
  ui->listWidgetQMLFiles->addItem(newItem);
}

void DeveloperOptions::addTerminalCursorPosCommand(int x, int y) { insertInTerminalDebug(QString("\\e[%1;%2H").arg(y + 1).arg(x + 1), Qt::green); }

void DeveloperOptions::on_pushButtonQmlSave_clicked() {
  QString defaultName = QString("terminal.qml");
  QString fileName = DefaultPathManager::getInstance().requestSaveFile(this, tr("Save QML terminal"), "path_sqml", defaultName, tr("QML file (*.qml)"));
  if (fileName.isEmpty())
    return;
  QFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Truncate)) {
    QFile qmlFile(qQuickWidget->source().toLocalFile());
    if (qmlFile.open(QFile::ReadOnly)) {
      QByteArray data = qmlFile.readAll();
      qmlFile.close();
      file.write(data);
      file.close();
    } else {
      qCritical() << "Unable to read current qml file" << qQuickWidget->source().toLocalFile();
    }
  } else
    qCritical() << "Cannot save qml file" << fileName;
}

void DeveloperOptions::qmlExport() {
  QMessageBox msgBox(this);
  msgBox.setText(tr("Export qml in compressed format"));
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.setButtonText(QMessageBox::Yes, tr("To clipboard"));
  msgBox.setButtonText(QMessageBox::Ok, tr("To file"));
  auto checkBox = new QCheckBox(&msgBox);
  checkBox->setText(tr("Export as C array"));
  checkBox->setChecked(true);
  msgBox.setCheckBox(checkBox);
  int returnValue = msgBox.exec();

  QFile qmlFile(qQuickWidget->source().toLocalFile());
  qmlFile.open(QFile::ReadOnly);
  QByteArray data = qmlFile.readAll();
  qmlFile.close();
  while (data.contains("\n "))
    data.replace("\n ", "\n");
  data = qCompress(data, 9);
  data = COBS::encode(data);

  if (checkBox->isChecked()) {
    QByteArray newData;
    newData.append(QString("const unsigned char terminalQml[%1] = {'$','$','Q',").arg(data.length() + 3).toLocal8Bit());
    for (auto it = data.begin(); it != data.end(); it++) {
      newData.append(QString::number((quint8)*it).toLocal8Bit());
      newData.append(",");
    }
    newData.remove(newData.length() - 1, 1);
    newData.append("};");
    data = newData;
  }

  if (returnValue == QMessageBox::Yes) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(data);
  } else if (returnValue == QMessageBox::Ok) {
    QString defaultName = QString("terminal.txt");
    QString fileName = DefaultPathManager::getInstance().requestSaveFile(this, tr("Export qml in compressed format"), "path_eqml", defaultName, tr("Text file (*.*)"));
    if (fileName.isEmpty())
      return;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
      file.write(data);
      file.close();
    }
    qDebug() << "Cannot write to file" << fileName;
  }
}

void DeveloperOptions::on_listWidgetTerminalCodeList_itemClicked(QListWidgetItem *item) {
  QString code = "";
  if (item->text().contains(" "))
    code = item->text().left(item->text().indexOf(" "));
  else
    code = item->text();

  if (code.at(0) == '\\')
    insertInTerminalDebug(QString(code.left(2)), Qt::blue);

  else if (code == "3?m") {
    QColor color = QColorDialog::getColor(Qt::white);
    if (!color.isValid())
      return;
    QByteArray colorCode = AnsiTerminalModel::nearestColorCode(color);
    insertInTerminalDebug(QString("\\e[3" + colorCode + "m").toUtf8(), Qt::red);
  }

  else if (code == "4?m") {
    QColor color = QColorDialog::getColor(Qt::black);
    if (!color.isValid())
      return;
    QByteArray colorCode = AnsiTerminalModel::nearestColorCode(color);
    insertInTerminalDebug(QString("\\e[4" + colorCode + "m").toUtf8(), Qt::red);
  }

  else
    insertInTerminalDebug(QString("\\e[" + code).toUtf8(), Qt::red);
}

void DeveloperOptions::insertInTerminalDebug(QString text, QColor textColor) {
  ui->textEditTerminalDebug->setTextColor(textColor);
  ui->textEditTerminalDebug->textCursor().insertText(text);
  ui->textEditTerminalDebug->setTextColor(ui->textEditTerminalDebug->palette().color(QPalette::Text));
}

void DeveloperOptions::on_listWidgetQMLFiles_itemClicked(QListWidgetItem *item) { emit loadQmlFile(QUrl::fromLocalFile(item->data(Qt::UserRole).toString())); }

void DeveloperOptions::qmlReload() {
  if (!ui->listWidgetQMLFiles->selectedItems().isEmpty())
    emit loadQmlFile(QUrl::fromLocalFile(ui->listWidgetQMLFiles->selectedItems().first()->data(Qt::UserRole).toString()));
  else {
    QUrl url = qQuickWidget->source();
    emit loadQmlFile(url);
  }
}

void DeveloperOptions::quickWidget_statusChanged(const QQuickWidget::Status &arg1) {
  if (arg1 == QQuickWidget::Error) {
    auto errors = qQuickWidget->errors();
    for (const auto &error : qAsConst(errors))
      ui->plainTextEditQmlLog->appendHtml("<font color=red>" + error.toString() + "<\font color>");
  } else if (arg1 == QQuickWidget::Ready) {
    ui->plainTextEditQmlLog->appendHtml("<font color=green>" + tr("File loaded") + "<\font color>");
  }
}

void DeveloperOptions::on_pushButtonClearAnsiTerminal_clicked() { emit printToTerminal("\u001b[2J\u001b[0m"); }

void DeveloperOptions::on_pushButtonTerminalInputCopy_clicked() {
  QByteArray bytes = ui->textEditTerminalDebug->toPlainText().replace('\n', "\\r\\n").toUtf8();
  for (unsigned char ch = 0;; ch++) {
    if (ch == 32)
      ch = 127;
    bytes.replace(ch, ("\\x" + QString::number(ch, 16)).toLocal8Bit() + "\"\"");
    if (ch == 255)
      break;
  }

  QGuiApplication::clipboard()->setText(bytes);
}

void DeveloperOptions::on_lineEditManualInput_returnPressed() {
  QByteArray bytes;
  bytes.append(ui->lineEditManualInput->text().toLocal8Bit());
  emit sendManualInput(bytes);
  ui->lineEditManualInput->clear();
  on_pushButtonScrollDown_2_clicked();
}

void DeveloperOptions::on_pushButtonScrollDown_2_clicked() {
  QScrollBar *scroll = ui->plainTextEditConsole_2->verticalScrollBar();
  scroll->setValue(scroll->maximum());
  scroll = ui->plainTextEditConsole_2->horizontalScrollBar();
  scroll->setValue(scroll->minimum());
}

void DeveloperOptions::on_pushButtonOpenConfig_clicked() { QDesktopServices::openUrl(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)); }
