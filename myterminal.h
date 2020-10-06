#ifndef MYTERMINAL_H
#define MYTERMINAL_H

#include <QTableWidget>
#include <QTableWidgetItem>

#include "colorCodes256.h"
#include "settings.h"

class MyTerminal : public QTableWidget {
  Q_OBJECT
public:
  explicit MyTerminal(QWidget *parent = nullptr);

private:
  void printChar(char text);
  void moveCursorAbsolute(int16_t x, int16_t y);
  void moveCursorRelative(int16_t x, int16_t y);
  void clearLine();
  void clearLine(uint16_t line);
  void clearLineLeft();
  void clearLineRight();
  void setUnderline(bool underlined);
  void setBold(bool bold);
  void printText(QByteArray text);
  void parseEscapeCode(QByteArray data);

private:
  uint16_t cursorX = 0;
  uint16_t cursorY = 0;
  QColor fontColor = Qt::white;
  QColor backColor = Qt::black;
  QFont font = QFont("Courier New", 18, QFont::Normal);
  void resetFont();
  bool isSmallest(uint8_t number, QVector<uint8_t> list);

public slots:
  void clearTerminal();
  void printToTerminal(QByteArray data);
};

#endif // MYTERMINAL_H
