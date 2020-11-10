#ifndef MYTERMINAL_H
#define MYTERMINAL_H

#include <QDebug>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "colorCodes.h"
#include "enums_defines_constants.h"

class MyTerminal : public QTableWidget {
  Q_OBJECT
public:
  explicit MyTerminal(QWidget *parent = nullptr);
  ~MyTerminal();

private:
  bool debug = false;
  QByteArray buffer;
  QMap<QString, QColor> colorCodes;
  void printChar(char text);
  void moveCursorAbsolute(int16_t x, int16_t y);
  void moveCursorRelative(int16_t x, int16_t y) { moveCursorAbsolute(cursorX + x, cursorY + y); }
  void clearLine();
  void clearLine(uint16_t line);
  void clearLineLeft();
  void clearLineRight();
  void clearUp();
  void clearDown();
  void setUnderline(bool underlined) { font.setUnderline(underlined); }
  void setBold(bool bold) { font.setBold(bold); }
  void printText(QByteArray text);
  void parseFontEscapeCode(QByteArray data);
  void parseEscapeCode(QByteArray data);
  uint16_t cursorX = 0;
  uint16_t cursorY = 0;
  uint16_t cursorX_saved = 0;
  uint16_t cursorY_saved = 0;
  QColor fontColor = Qt::white;
  QColor backColor = Qt::black;
  QFont font = QFont("Courier New", 18, QFont::Normal);
  void resetFont();
  bool isSmallest(uint8_t number, QVector<uint8_t> list);
  void clearTerminal();

  void clearCell(int x, int y);
public slots:
  void printToTerminal(QByteArray data);
  void setDebug(bool en);
  void resetTerminal();

signals:
  /// Pošle zprávu do výpisu
  void sendMessage(QByteArray header, QByteArray message, MessageLevel::enumerator type, MessageTarget::enumerator target = MessageTarget::serial1);
};

#endif // MYTERMINAL_H
