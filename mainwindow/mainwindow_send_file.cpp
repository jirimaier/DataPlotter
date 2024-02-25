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

#include "global.h"
#include "mainwindow.h"

void MainWindow::fileRequest(QByteArray message, MessageTarget::enumMessageTarget source) {
  QByteArrayList list = message.split(',');

  if (!list.isEmpty() && list.first().toLower() == "new") {
    QByteArray text = {};
    QString fileName = DefaultPathManager::getInstance().requestOpenFile(this, tr("Send file"), "filepath_rq", "Any file (*.*)");
    if (fileName.isEmpty())
      return;
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
      text = file.readAll();
    } else
      return;

    fileSender.newFile(text);

    list.removeFirst();

    if (list.isEmpty())
      return;
  }

  if (list.length() == 0) {
    QByteArray result = fileSender.nextPart("", "");
    if (!result.isEmpty())
      printMessage(tr("Invalid file request").toUtf8(), result, MessageLevel::error, source);
  } else if (list.length() == 1) {
    QByteArray result = fileSender.nextPart(list.first(), "");
    if (!result.isEmpty())
      printMessage(tr("Invalid file request").toUtf8(), result, MessageLevel::error, source);
  } else if (list.length() == 2) {
    QByteArray result = fileSender.nextPart(list.first(), list.last());
    if (!result.isEmpty())
      printMessage(tr("Invalid file request").toUtf8(), result, MessageLevel::error, source);
  } else
    printMessage(tr("Invalid file request").toUtf8(), tr("Too many arguments in header").toUtf8(), MessageLevel::error, source);
}
