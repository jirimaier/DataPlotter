#include "myterminal.h"

MyTerminal::MyTerminal(QWidget *parent) : QTableWidget(parent) { colorCodes = ColorCodes::colorCodes(); }

MyTerminal::~MyTerminal() {
  for (uint16_t r = 0; r < this->rowCount(); r++)
    for (uint16_t c = 0; c < this->columnCount(); c++)
      if (this->item(r, c) != NULL)
        delete this->item(r, c);
}

void MyTerminal::printText(QByteArray text) {
  for (uint16_t i = 0; i < text.length(); i++) {
    if (text.at(i) == '\b' || text.at(i) == 0x7f) {
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
    printChar(text.at(i));
  }
}

void MyTerminal::printChar(char letter) {
  if (this->item(cursorY, cursorX) != NULL)
    delete this->item(cursorY, cursorX);
  this->setItem(cursorY, cursorX, new QTableWidgetItem(QChar(letter)));
  this->item(cursorY, cursorX)->setBackground(backColor);
  this->item(cursorY, cursorX)->setForeground(fontColor);
  this->item(cursorY, cursorX)->setFont(font);
  this->item(cursorY, cursorX)->setTextAlignment(Qt::AlignLeft);
  moveCursorRelative(1, 0);
}

void MyTerminal::moveCursorAbsolute(int16_t x, int16_t y) {
  if (debug)
    this->setCurrentCell(cursorY, cursorX, QItemSelectionModel::Deselect);
  cursorX = (x > 0) ? x : 0;
  cursorY = (y > 0) ? y : 0;
  if (cursorY >= this->rowCount())
    this->setRowCount(cursorY + 1);
  if (cursorX >= this->columnCount())
    this->setColumnCount(cursorX + 1);
  if (debug)
    this->setCurrentCell(cursorY, cursorX, QItemSelectionModel::Select);
  else
    this->setCurrentCell(cursorY, cursorX);
}

void MyTerminal::clearTerminal() {
  for (uint16_t r = 0; r < this->rowCount(); r++)
    for (uint16_t c = 0; c < this->columnCount(); c++)
      if (this->item(r, c) != NULL)
        delete this->item(r, c);
  this->setRowCount(1);
  this->setColumnCount(1);
  resetFont();
  moveCursorAbsolute(0, 0);
  if (!buffer.isEmpty())
    buffer.clear();
}

void MyTerminal::setUnderline(bool underlined) { font.setUnderline(underlined); }

void MyTerminal::setBold(bool bold) { font.setBold(bold); }

void MyTerminal::parseFontEscapeCode(QByteArray data) {
  // Reset
  if (data == "0")
    resetFont();

  // Font color
  else if (*data.begin() == '3') {
    QString code = data.mid(1);
    if (colorCodes.contains(code))
      fontColor = colorCodes[code];
  }

  // 256 Colors Text
  else if (data.left(5) == "38;5;")
    fontColor = QColor::fromRgb(ColorCodes::colorCodes256[data.mid(6).toUInt()]);

  // 8 Color Background
  else if (data == "40")
    backColor = Qt::black;
  else if (data == "41")
    backColor = Qt::darkRed;
  else if (data == "42")
    backColor = Qt::darkGreen;
  else if (data == "43")
    backColor = Qt::darkYellow;
  else if (data == "44")
    backColor = Qt::darkBlue;
  else if (data == "45")
    backColor = Qt::darkMagenta;
  else if (data == "46")
    backColor = Qt::darkCyan;
  else if (data == "47")
    backColor = Qt::lightGray;

  // 16 Color Background
  else if (data == "40;1")
    backColor = Qt::darkGray;
  else if (data == "41;1")
    backColor = Qt::red;
  else if (data == "42;1")
    backColor = Qt::green;
  else if (data == "43;1")
    backColor = Qt::yellow;
  else if (data == "44;1")
    backColor = Qt::blue;
  else if (data == "45;1")
    backColor = Qt::magenta;
  else if (data == "46;1")
    backColor = Qt::cyan;
  else if (data == "47;1")
    backColor = Qt::white;

  // 256 Colors Background
  else if (data.contains("48;5;"))
    backColor = QColor::fromRgb(ColorCodes::colorCodes256[data.right(data.length() - 5).toUInt()]);

  // Decorations
  else if (data == "1")
    setBold(true);
  else if (data == "4")
    setUnderline(true);
  else if (data == "7") {
    QColor clr = fontColor;
    fontColor = backColor;
    backColor = clr;
  } else
    emit sendMessage(tr("Invalid escape sequence").toUtf8(), data, MessageLevel::error);
}

void MyTerminal::parseEscapeCode(QByteArray data) {
  if (data == "s") {
    cursorX_saved = cursorX;
    cursorY_saved = cursorY;
    return;
  }
  if (data == "u") {
    moveCursorAbsolute(cursorX_saved, cursorY_saved);
    return;
  }
  if (data.right(1) == "m") {
    parseFontEscapeCode(data.left(data.length() - 1));
    return;
  }
  if (data.right(1) == "A" || data.right(1) == "B" || data.right(1) == "C" || data.right(1) == "D") {
    bool isok;
    int value = data.left(data.length() - 1).toUInt(&isok, 10);
    if (isok) {
      if (data.right(1) == "A")
        moveCursorRelative(0, -value);
      else if (data.right(1) == "B")
        moveCursorRelative(0, value);
      else if (data.right(1) == "C")
        moveCursorRelative(value, 0);
      else if (data.right(1) == "D")
        moveCursorRelative(-value, 0);
    } else
      emit sendMessage(tr("Invalid escape sequence").toUtf8(), data, MessageLevel::error);
    return;
  }
  if (data == "2J") {
    clearTerminal();
    return;
  }

  emit sendMessage(tr("Invalid escape sequence").toUtf8(), data, MessageLevel::error);
}

void MyTerminal::printToTerminal(QByteArray data) {
  buffer.push_back(data);
  while (!buffer.isEmpty()) {
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

void MyTerminal::setDebug(bool en) {
  debug = en;
  if (en)
    this->setCurrentCell(cursorY, cursorX, QItemSelectionModel::Select);
  else
    this->setCurrentCell(cursorY, cursorX, QItemSelectionModel::Deselect);
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
  uint16_t originalCursorX = cursorX;
  cursorX = 0;
  for (uint16_t i = 0; i < this->columnCount(); i++)
    printChar(' ');
  cursorX = originalCursorX;
}

void MyTerminal::clearLine(uint16_t line) {
  uint16_t originalCursorX = cursorX;
  uint16_t originalCursorY = cursorY;
  moveCursorAbsolute(0, line);
  for (uint16_t i = 0; i < this->columnCount(); i++)
    printChar(' ');
  cursorX = originalCursorX;
  cursorY = originalCursorY;
}

void MyTerminal::clearLineLeft() {
  uint16_t originalCursorX = cursorX;
  cursorX = 0;
  for (uint16_t i = 0; i < cursorX; i++)
    printChar(' ');
  cursorX = originalCursorX;
}

void MyTerminal::clearLineRight() {
  uint16_t originalCursorX = cursorX;
  for (uint16_t i = cursorX + 1; i < this->columnCount() - cursorX - 1; i++)
    printChar(' ');
  cursorX = originalCursorX;
}
