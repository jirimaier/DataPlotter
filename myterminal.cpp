#include "myterminal.h"

MyTerminal::MyTerminal(QWidget *parent) : QTableWidget(parent) {
  QPalette p = this->palette();
  p.setColor(QPalette::Base, Qt::black);
  p.setColor(QPalette::Text, Qt::white);
  p.setColor(QPalette::Window, Qt::cyan);
  this->setPalette(p);
}

void MyTerminal::printText(QByteArray text) {
  for (uint16_t i = 0; i < text.length(); i++)
    printChar(text.at(i));
}

void MyTerminal::printChar(char text) {
  if (this->item(cursorY, cursorX))
    this->item(cursorY, cursorX)->setText(QString(text));
  else
    this->setItem(cursorY, cursorX, new QTableWidgetItem(QString(text)));
  this->item(cursorY, cursorX)->setBackground(backColor);
  this->item(cursorY, cursorX)->setForeground(fontColor);
  this->item(cursorY, cursorX)->setFont(font);
  moveCursorRelative(1, 0);
}

void MyTerminal::moveCursorAbsolute(int16_t x, int16_t y) {
  cursorX = (x > 0) ? x : 0;
  cursorY = (y > 0) ? y : 0;
  if (cursorY >= this->rowCount())
    this->setRowCount(cursorY + 1);
  if (cursorX >= this->columnCount())
    this->setColumnCount(cursorX + 1);

  this->setCurrentCell(cursorY, cursorX);
}

void MyTerminal::moveCursorRelative(int16_t x, int16_t y) { moveCursorAbsolute(cursorX + x, cursorY + y); }

void MyTerminal::clearTerminal() {
  for (uint16_t r = 0; r < this->rowCount(); r++)
    for (uint16_t c = 0; c < this->columnCount(); c++)
      if (this->item(r, c))
        delete this->item(r, c);
  this->setRowCount(1);
  this->setColumnCount(1);
  resetFont();
  moveCursorAbsolute(0, 0);
}

void MyTerminal::setUnderline(bool underlined) { font.setUnderline(underlined); }

void MyTerminal::setBold(bool bold) { font.setBold(bold); }

void MyTerminal::parseEscapeCode(QByteArray data) {
  // Reset
  if (data == "0")
    resetFont();

  // 8 Color Text
  if (data == "30")
    fontColor = Qt::black;
  if (data == "31")
    fontColor = Qt::darkRed;
  if (data == "32")
    fontColor = Qt::darkGreen;
  if (data == "33")
    fontColor = Qt::darkYellow;
  if (data == "34")
    fontColor = Qt::darkBlue;
  if (data == "35")
    fontColor = Qt::darkMagenta;
  if (data == "36")
    fontColor = Qt::darkCyan;
  if (data == "37")
    fontColor = Qt::lightGray;

  // 16 Color Text
  if (data == "30;1")
    fontColor = Qt::darkGray;
  if (data == "31;1")
    fontColor = Qt::red;
  if (data == "32;1")
    fontColor = Qt::green;
  if (data == "33;1")
    fontColor = Qt::yellow;
  if (data == "34;1")
    fontColor = Qt::blue;
  if (data == "35;1")
    fontColor = Qt::magenta;
  if (data == "36;1")
    fontColor = Qt::cyan;
  if (data == "37;1")
    fontColor = Qt::white;

  // 256 Colors Text
  if (data.contains("38;5;"))
    fontColor = QColor::fromRgb(colorCodes256[data.right(data.length() - 5).toUInt()]);

  // 8 Color Background
  if (data == "40")
    backColor = Qt::black;
  if (data == "41")
    backColor = Qt::darkRed;
  if (data == "42")
    backColor = Qt::darkGreen;
  if (data == "43")
    backColor = Qt::darkYellow;
  if (data == "44")
    backColor = Qt::darkBlue;
  if (data == "45")
    backColor = Qt::darkMagenta;
  if (data == "46")
    backColor = Qt::darkCyan;
  if (data == "47")
    backColor = Qt::lightGray;

  // 16 Color Background
  if (data == "40;1")
    backColor = Qt::darkGray;
  if (data == "41;1")
    backColor = Qt::red;
  if (data == "42;1")
    backColor = Qt::green;
  if (data == "43;1")
    backColor = Qt::yellow;
  if (data == "44;1")
    backColor = Qt::blue;
  if (data == "45;1")
    backColor = Qt::magenta;
  if (data == "46;1")
    backColor = Qt::cyan;
  if (data == "47;1")
    backColor = Qt::white;

  // 256 Colors Background
  if (data.contains("48;5;"))
    backColor = QColor::fromRgb(colorCodes256[data.right(data.length() - 5).toUInt()]);

  // Decorations
  if (data == "1")
    setBold(true);
  if (data == "4")
    setUnderline(true);
  if (data == "7") {
    QColor clr = fontColor;
    fontColor = backColor;
    backColor = clr;
  }
}

void MyTerminal::printToTerminal(QByteArray data) {
  for (uint16_t i = 0; i < data.length(); i++) {
    if (data.at(i) == '\u001b' && data.at(i + 1) == '[') {
      QByteArray code = data.right(data.length() - i - 2);
      uint8_t M = code.indexOf('m', 0);
      uint8_t A = code.indexOf('A', 0);
      uint8_t B = code.indexOf('B', 0);
      uint8_t C = code.indexOf('C', 0);
      uint8_t D = code.indexOf('D', 0);
      uint8_t J = code.indexOf('J', 0);
      uint8_t K = code.indexOf('K', 0);
      QVector<uint8_t> all = {M, A, B, C, D, J, K};
      uint16_t begin = i + 2;

      if (isSmallest(M, all)) {
        // kod s M
        i = data.indexOf('m', begin);
        parseEscapeCode(code.left(M));
        continue;
      }

      if (isSmallest(A, all)) {
        // kod s A
        i = data.indexOf('A', begin);
        moveCursorRelative(0, code.left(A).toUInt() * (-1));
        continue;
      }

      if (isSmallest(B, all)) {
        // kod s B
        i = data.indexOf('B', begin);
        moveCursorRelative(0, code.left(B).toUInt());
        continue;
      }

      if (isSmallest(C, all)) {
        // kod s C
        i = data.indexOf('C', begin);
        moveCursorRelative(code.left(C).toUInt(), 0);
        continue;
      }

      if (isSmallest(D, all)) {
        // kod s D
        i = data.indexOf('D', begin);
        moveCursorRelative(code.left(D).toUInt() * (-1), 0);
        continue;
      }

      if (isSmallest(J, all)) {
        // kod s J
        i = data.indexOf('J', begin);
        if (code.left(J) == "2") {
          clearTerminal();
        }
        if (code.left(J) == "0") {
          // TODO
        }
        if (code.left(J) == "1") {
          // TODO
        }
        continue;
      }

      if (isSmallest(K, all)) {
        // kod s K
        i = data.indexOf('K', begin);
        if (code.left(K) == "2")
          clearLine();
        if (code.left(K) == "0")
          clearLineRight();
        if (code.left(K) == "1")
          clearLineLeft();
        continue;
      }
    }
    if (data.at(i) == '\b') {
      moveCursorRelative(-1, 0);
      printChar(' ');
      moveCursorRelative(-1, 0);
      continue;
    }
    if (data.at(i) == '\r') {
      moveCursorAbsolute(0, cursorY);
      continue;
    }
    if (data.at(i) == '\n') {
      moveCursorRelative(0, 1);
      continue;
    }
    printChar(data.at(i));
  }
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
