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

#include "myterminal.h"

MyTerminal::MyTerminal(QWidget* parent) : QTableWidget(parent) {
  colorCodes["0"] = Qt::black;
  colorCodes["1"] = Qt::darkRed;
  colorCodes["2"] = Qt::darkGreen;
  colorCodes["3"] = Qt::darkYellow;
  colorCodes["4"] = Qt::darkBlue;
  colorCodes["5"] = Qt::darkMagenta;
  colorCodes["6"] = Qt::darkCyan;
  colorCodes["7"] = Qt::lightGray;

  colorCodes["0;1"] = Qt::darkGray;
  colorCodes["1;1"] = Qt::red;
  colorCodes["2;1"] = Qt::green;
  colorCodes["3;1"] = Qt::yellow;
  colorCodes["4;1"] = Qt::blue;
  colorCodes["5;1"] = Qt::magenta;
  colorCodes["6;1"] = Qt::cyan;
  colorCodes["7;1"] = Qt::white;

  connect(this, &QTableWidget::cellClicked, this, &MyTerminal::characterClicked);
  connect(this, &QTableWidget::cellDoubleClicked, this, &MyTerminal::characterDoubleClicked);
  connect(&clickBlinkTimer, &QTimer::timeout, this, &MyTerminal::resetBlinkedItem);
  setMode(mode);
  clickBlinkTimer.setSingleShot(true);
  clickBlinkTimer.setInterval(TERMINAL_CLICK_BLINK_TIME);

  changeFont(false);
}

MyTerminal::~MyTerminal() {
  for (uint16_t r = 0; r < this->rowCount(); r++)
    for (uint16_t c = 0; c < this->columnCount(); c++)
      if (this->item(r, c) != NULL)
        delete this->item(r, c);
}

void MyTerminal::setColorExceptionList(QList<QColor> newlist, bool isBlacklist) {
  exceptionList = newlist;
  exeptionListIsBlacklist = isBlacklist;
}

bool MyTerminal::colorFromSequence(QByteArray code, QColor& clr) {
  code.remove(0, 1);

  // 256 barev
  if (code.left(4) == "8;5;") {
    bool isOK;
    unsigned int index = code.mid(4).toUInt(&isOK);
    if (!isOK || index >= 256)
      return false;
    clr = colorCodes256[index];
    return true;
  }

  // Základní a rozšířené (16) barvy
  else if (colorCodes.contains(code)) {
    clr = colorCodes[code];
    return true;
  }

  return false;
}

void MyTerminal::printText(QByteArray bytes) {
  QString text = QString::fromUtf8(bytes);
  for (uint16_t i = 0; i < text.length(); i++) {
    if (text.at(i) == '\b' || text.at(i) == QChar::fromLatin1((char)0x7f)) {
      moveCursorRelative(-1, 0);
      printChar(' ');
      moveCursorRelative(-1, 0);
      continue;
    }
    if (text.at(i) == '\r') {
      moveCursorAbsolute(0, cursorY);
      continue;
    }
    if (text.at(i) == '\n') {
      moveCursorRelative(0, 1);
      continue;
    }
    if (text.at(i) == '\a') {
      QApplication::beep();
      continue;
    }
    if (text.at(i) == '\t') { // Tabulátor posune kursor na další sloupec který je násobkem 8 (počítáno od 0)
      do
        moveCursorRelative(1, 0);
      while (cursorX % 8);
      continue;
    }

    if (text.at(i) == '_') { // Podtržítko je moc hluboko a nevejde se do pole, nahradí se podtržením.
      bool wasUnderlined = font.underline();
      font.setUnderline(true);
      printChar(' ');
      font.setUnderline(wasUnderlined);
      continue;
    }
    printChar(text.at(i));
  }
}

void MyTerminal::printChar(QChar letter) {
  if (blinkedItem.blinkInProggres) {
    if (blinkedItem.r == cursorY && blinkedItem.c == cursorX)
      resetBlinkedItem();
  }

  if (cursorY >= this->rowCount())
    addRows(cursorY + 1);
  if (cursorX >= this->columnCount())
    addColumns(cursorX + 1);

  clearCell(cursorX, cursorY);
  this->setItem(cursorY, cursorX, new QTableWidgetItem(letter));
  this->item(cursorY, cursorX)->setBackground(backColor);
  this->item(cursorY, cursorX)->setForeground(fontColor);
  this->item(cursorY, cursorX)->setTextAlignment(Qt::AlignCenter);
  this->item(cursorY, cursorX)->setFont(font);
  moveCursorRelative(1, 0);
}

void MyTerminal::moveCursorAbsolute(int16_t x, int16_t y) {
  if (mode == debug)
    this->setCurrentCell(cursorY, cursorX, QItemSelectionModel::Deselect);
  cursorX = (x > 0) ? x : 0;
  cursorY = (y > 0) ? y : 0;
  if (mode == debug) {
    if (cursorY >= this->rowCount())
      addRows(cursorY + 1);
    if (cursorX >= this->columnCount())
      addColumns(cursorX + 1);
    this->setCurrentCell(cursorY, cursorX, QItemSelectionModel::Select);
  }
}

void MyTerminal::clearTerminal() {
  for (uint16_t r = 0; r < this->rowCount(); r++)
    for (uint16_t c = 0; c < this->columnCount(); c++)
      if (this->item(r, c) != NULL)
        delete this->item(r, c);
  this->setRowCount(1);
  this->setColumnCount(1);
  setRowHeight(0, cellHeight);
  setColumnWidth(0, cellWidth);
  resetFont();
  moveCursorAbsolute(0, 0);
  if (mode == debug) {
    addRows(100);
    addColumns(50);
  }
}

void MyTerminal::highLightField(QTableWidgetItem* field) {
  QColor clr = field->background().color();
  if (clr.lightness() < 127) // Tmavou barvu zesvětlá
    clr.setHsl(clr.hue(), clr.saturation(), clr.lightness() + 50);
  else // Světlo stmaví
    clr.setHsl(clr.hue(), clr.saturation(), clr.lightness() - 50);
  if (field->background().color() == field->foreground().color())
    field->setForeground(clr); // Pokud byl text schovaný stejnou barvou, bude mít novou barvu
  field->setBackground(clr);
}

void MyTerminal::addRows(int newCount) {
  int oldcount = rowCount();
  setRowCount(newCount);
  for (int r = oldcount; r < newCount; r++)
    setRowHeight(r, cellHeight);
}

void MyTerminal::addColumns(int newCount) {
  int oldcount = columnCount();
  setColumnCount(newCount);
  for (int c = oldcount; c < newCount; c++)
    setColumnWidth(c, cellWidth);
}

QByteArray MyTerminal::nearestColorCode(QColor color) {
  auto colors = colorCodes.values();
  foreach (QColor clr, colors)
    if (color == clr)
      return colorCodes.key(color).toLocal8Bit();

  int nearestIndex = 0;
  int nearestDistance = INT_MAX;
  for (int i = 0; i < 256; i++) {
    QColor clr = QColor::fromRgb(colorCodes256[i]);
    int distance = (color.red() - clr.red()) * (color.red() - clr.red());
    distance += (color.green() - clr.green()) * (color.green() - clr.green());
    distance += (color.blue() - clr.blue()) * (color.blue() - clr.blue());
    if (distance < nearestDistance) {
      nearestDistance = distance;
      nearestIndex = i;
    }
  }
  return (QString("8;5;" + QString::number(nearestIndex)).toUtf8());
}

void MyTerminal::parseFontEscapeCode(QByteArray data) {
  // Reset
  if (data == "0")
    resetFont();

  // Decorations
  else if (data == "1")
    setBold(true);
  else if (data == "4")
    setUnderline(true);
  else if (data == "7") {
    QColor clr = fontColor;
    fontColor = backColor;
    backColor = clr;
  }

  // Font color
  else if (*data.begin() == '3') {
    if (!colorFromSequence(data, fontColor))
      goto INVALID;
  }

  // Bacground color
  else if (*data.begin() == '4') {
    if (!colorFromSequence(data, backColor))
      goto INVALID;
  }

  else {
INVALID:
    emit sendMessage(tr("Invalid escape sequence").toUtf8(), data, MessageLevel::error);
  }
}

void MyTerminal::parseEscapeCode(QByteArray data) {
  // Uložit pozici kursoru
  if (data == "s") {
    cursorX_saved = cursorX;
    cursorY_saved = cursorY;
  }
  // Vrátit kursor na uloženou posici
  else if (data == "u") {
    moveCursorAbsolute(cursorX_saved, cursorY_saved);
  }
  // Nastavení barev a stylů
  else if (data.right(1) == "m") {
    parseFontEscapeCode(data.left(data.length() - 1));
  }
  // Pohyb kursoru
  else if (data.right(1) == "A" || data.right(1) == "B" || data.right(1) == "C" || data.right(1) == "D" || data.right(1) == "E" || data.right(1) == "F") {
    int value = 1;
    if (data.length() > 1) {
      bool isok;
      value = data.left(data.length() - 1).toUInt(&isok, 10);
      if (!isok) {
        emit sendMessage(tr("Invalid escape sequence").toUtf8(), data, MessageLevel::error);
        return;
      }
    }
    if (data.right(1) == "A") // Nahoru
      moveCursorRelative(0, -value);
    else if (data.right(1) == "B") // Dolů
      moveCursorRelative(0, value);
    else if (data.right(1) == "C") // Vpravo
      moveCursorRelative(value, 0);
    else if (data.right(1) == "D") // Vlevo
      moveCursorRelative(-value, 0);
    else if (data.right(1) == "E") // Řádek dolů
      moveCursorAbsolute(0, cursorY + value);
    else if (data.right(1) == "F") // Řádek nahoru
      moveCursorAbsolute(0, cursorY - value);
    else if (data.right(1) == "G") // Sloupec
      moveCursorAbsolute(value - 1, cursorY);
  }

  // Posunout kursor na pozici (v příkazu se čísluje od 1)
  else if (data.right(1) == "H" || data.right(1) == "f") {
    int n = 1;
    int m = 1;
    if (data.length() == 1) {
      moveCursorAbsolute(0, 0);
      return;
    }
    if (data.contains(';')) {
      bool isok = true;
      QByteArrayList coord = data.left(data.length() - 1).split(';');
      if (coord.length() == 2) {
        if (!coord.at(0).trimmed().isEmpty())
          n = coord.at(0).toUInt(&isok);
        if (!coord.at(1).trimmed().isEmpty())
          m = coord.at(1).toUInt(&isok);
      } else
        isok = false;
      if (isok) {
        moveCursorAbsolute(m - 1, n - 1);
        return;
      }
    } else {
      bool isok;
      n = data.left(data.length() - 1).toUInt(&isok);
      if (isok) {
        moveCursorAbsolute(0, n - 1);
        return;
      }
    }
    emit sendMessage(tr("Invalid escape sequence").toUtf8(), data, MessageLevel::error);
  }

  // Vymazat od kursoru na konec všeho
  else if (data == "0J" || data == "J") {
    clearLineRight();
    clearDown();
  }
  // Vymazet od kursoru po začátek všeho
  else if (data == "1J") {
    clearLineLeft();
    clearUp();
  }
  // Vymazat všechno
  else if (data == "2J")
    clearTerminal();

  // Vymazat od kursoru na konec řádku
  else if (data == "0K" || data == "K")
    clearLineRight();
  // Vymazet od kursoru po začátek řádku
  else if (data == "1K")
    clearLineLeft();
  // Vymazat řádek
  else if (data == "2K")
    clearLine();

  else
    emit sendMessage(tr("Invalid escape sequence").toUtf8(), data, MessageLevel::error);
}

void MyTerminal::printToTerminal(QByteArray data) {
  buffer.push_back(data);
  while (!buffer.isEmpty()) {
    if ((((uint8_t)buffer.at(buffer.length() - 1)) & 0b10000000) == 0b10000000) { //1xxxxxxx
      if ((((uint8_t)buffer.at(buffer.length() - 1)) & 0b11000000) == 0b10000000) { //10xxxxxx
        //Poslední znak je UTF-8, ale ne první bajt, je potřeba zjistit, jestli jde o poslední z bajtů (kompletní znak)
        bool complete = false;
        if ((buffer.length() >= 2) && (((uint8_t)buffer.at(buffer.length() - 2)) & 0b11100000) == 0b11000000) //110xxxxx 10xxxxxx
          complete = true;

        else if ((buffer.length() >= 3) && (((uint8_t)buffer.at(buffer.length() - 3)) & 0b11110000) == 0b11100000) //1110xxxx 10xxxxxx 10xxxxxx
          complete = true;

        else if ((buffer.length() >= 4) && (((uint8_t)buffer.at(buffer.length() - 4)) & 0b11111000) == 0b11110000) //11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
          complete = true;

        if (!complete)
          return;
      } else
        return; // Pozlední znak je začátek UTF-8 znaku, je potřeba počkat na zbytek
    }

    if (buffer.at(0) != '\u001b') {
      if (!buffer.contains('\u001b')) {
        printText(buffer);
        buffer.clear();
        break;
      }
      printText(buffer.left(buffer.indexOf('\u001b')));
      buffer.remove(0, buffer.indexOf('\u001b'));
      continue;
    }
    if (buffer.length() == 1)
      break;
    if (buffer.at(1) != '[') {
      buffer.remove(0, 2);
      continue;
    }
    if (buffer.length() < 3)
      break;
    for (int i = 2; true; i++) {
      if (!isdigit(buffer.at(i)) && buffer.at(i) != ';') {
        parseEscapeCode(buffer.mid(2, i - 1));
        buffer.remove(0, i + 1);
        break;
      }
      if (i >= buffer.length() - 1)
        return;
    }
  }
}

void MyTerminal::setMode(TerminalMode::enumTerminalMode mode) {
  this->clearSelection();
  this->mode = mode;
  if (mode == TerminalMode::debug) {
    this->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    this->setCurrentCell(cursorY, cursorX, QItemSelectionModel::Select);
    this->addRows(MAX(100, rowCount()));
    this->addColumns(MAX(smallFont?SMALL_COLS:LARGE_COLS, columnCount()));
  } else if (mode == TerminalMode::select) {
    this->setSelectionMode(QAbstractItemView::SelectionMode::ContiguousSelection);
  } else if (mode == TerminalMode::clicksend) {
    this->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
  } else if (mode == TerminalMode::none) {
    this->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
  }
  this->setShowGrid(mode == TerminalMode::debug);

  emit modeChanged(mode);
}

void MyTerminal::resetTerminal() {
  clearTerminal();
  if (!buffer.isEmpty())
    buffer.clear();
}

void MyTerminal::copyToClipboard() {
  QClipboard* clipboard = QGuiApplication::clipboard();
  QString text = "";
  QModelIndexList selection = this->selectionModel()->selectedIndexes();
  if (selection.isEmpty())
    return;
  int lastRow = selection.at(0).row();
  for (QModelIndexList::iterator it = selection.begin(); it != selection.end(); it++) {
    if (it->row() != lastRow) {
      text.append('\n');
      lastRow = it->row();
    }
    text.append(it->data().toString());
  }
  clipboard->setText(text);
  clearSelection();
}

void MyTerminal::changeFont(bool smallFont) {
  font.setPointSize(smallFont ? 10 : 16);
  setFont(font);
  for (int r = 0; r < rowCount(); r++) {
    for (int c = 0; c < columnCount(); c++) {
      if (this->item(r, c) != nullptr) {
        auto newfont =  this->item(r, c)->font();
        newfont.setPointSize(smallFont ? 10 : 16);
        this->item(r, c)->setFont(newfont);
      }
    }
  }

  this->smallFont = smallFont;

  if(mode==debug)
      this->addColumns(MAX(smallFont?SMALL_COLS:LARGE_COLS, columnCount()));

  cellWidth = smallFont ? (geometry().width()-6)/SMALL_COLS : (geometry().width()-6)/LARGE_COLS;
  cellHeight =  cellWidth * (24.0/18.0);

  for (int r = 0; r < rowCount(); r++)
    setRowHeight(r, cellHeight);

  for (int c = 0; c < columnCount(); c++)
    setColumnWidth(c, cellWidth);

}

void MyTerminal::setVScrollBar(bool show) {
  this->setVerticalScrollBarPolicy(show ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);

  // Po zapnutí scrollbaru se změna neprojeví (scrollbar se neobjeví), dokud nedojde ke změně (asi bug v Qt)
  if (show)
    addRows(rowCount() + 1);

}

void MyTerminal::resizeEvent(QResizeEvent *event)
{
    changeFont(smallFont);
}

void MyTerminal::resetFont() {
  backColor = Qt::black;
  fontColor = Qt::white;
  setBold(false);
  setUnderline(false);
}

bool MyTerminal::isSmallest(uint8_t number, QVector<uint8_t> list) {
  for (uint8_t i = 0; i < list.length(); i++)
    if (number > list.at(i))
      return false;
  return true;
}

void MyTerminal::clearLine() {
  for (uint16_t i = 0; i < this->columnCount(); i++)
    clearCell(i, cursorY);
}

void MyTerminal::clearLine(uint16_t line) {
  for (uint16_t i = 0; i < this->columnCount(); i++)
    clearCell(i, line);
}

void MyTerminal::clearLineLeft() {
  for (uint16_t i = 0; i <= cursorX; i++)
    clearCell(i, cursorY);
}

void MyTerminal::clearLineRight() {
  for (uint16_t i = cursorX; i < this->columnCount(); i++)
    clearCell(i, cursorY);
}

void MyTerminal::clearDown() {
  clearLineRight();
  for (uint16_t i = cursorY + 1; i < this->rowCount(); i++)
    clearLine(i);
}

void MyTerminal::clearUp() {
  clearLineLeft();
  for (uint16_t i = 0; i < cursorX; i++)
    clearLine(i);
}

void MyTerminal::clearCell(int x, int y) {
  if (this->item(y, x) != NULL)
    delete this->item(y, x);
}

void MyTerminal::resetBlinkedItem() {
  clickBlinkTimer.stop();
  if (blinkedItem.blinkInProggres) {
    auto field = item(blinkedItem.r, blinkedItem.c);
    if (field) {
      field->setBackground(blinkedItem.originalBack);
      field->setForeground(blinkedItem.originalFront);
    }
  }

  blinkedItem.blinkInProggres = false;
}

void MyTerminal::characterClicked(int r, int c) {
  if (mode == clicksend) {
    QTableWidgetItem* it = item(r, c);
    if (it) {
      if (exeptionListIsBlacklist && exceptionList.contains(it->background().color()))
        return;
      if (!exeptionListIsBlacklist && !exceptionList.contains(it->background().color()))
        return;

      emit writeToSerial(it->text().toUtf8());
      if (blinkedItem.blinkInProggres)
        resetBlinkedItem();
      blinkedItem.blinkInProggres = true;
      blinkedItem.originalBack = it->background().color();
      blinkedItem.originalFront = it->foreground().color();
      blinkedItem.r = r;
      blinkedItem.c = c;
      highLightField(it);
      clickBlinkTimer.start();
    }
  } else if (mode == debug) {
    moveCursorAbsolute(c, r);
    if (r > this->rowCount() / 2)
      this->addRows(this->rowCount() * 1.5);
  }
}
