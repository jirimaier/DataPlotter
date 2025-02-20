
#ifndef ANSITERMINALMODEL_H
#define ANSITERMINALMODEL_H

#include "global.h"
#include <QAbstractListModel>
#include <QApplication>
#include <QColor>

class AnsiTerminalModel : public QAbstractListModel {
  Q_OBJECT
public:
  enum GridRoles { CharacterRole = Qt::UserRole + 1, BackgroundColorRole, FontColorRole, BoldRole, UnderlineRole, ClickableRole, ClickHighLightColorRole };

  struct GridItem {
    QChar character;
    QColor backgroundColor;
    QColor fontColor;
    bool bold;
    bool underline;
    bool clickable;

    GridItem() {
      character = ' ';
      backgroundColor = Qt::black;
      fontColor = Qt::white;
      bold = false;
      underline = false;
      clickable = false;
    }

    GridItem(QChar c, QColor b, QColor f, bool bold, bool u, bool clickable) {
      character = c;
      backgroundColor = b;
      fontColor = f;
      this->bold = bold;
      underline = u;
      this->clickable = clickable;
    }
  };

  explicit AnsiTerminalModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;
  void addItem(const GridItem &item);

  int columns() const;
  void setColumns(int newColumns);

  inline int rows();

  void appendColumn();
  void appendRow();

  static QByteArray nearestColorCode(QColor color);

  void clear();
  void clearCell(int column, int row);

  void printChar(QChar text);
  void moveCursorAbsolute(int16_t x, int16_t y);
  void moveCursorRelative(int16_t x, int16_t y) { moveCursorAbsolute(cursorX + x, cursorY + y); }
  void clearLine();
  void clearLine(uint16_t line);
  void clearLineLeft();
  void clearLineRight();
  void clearUp();
  void clearDown();
  void setUnderline(bool underlined) { this->underlined = underlined; }
  void setBold(bool bold) { this->bold = bold; }
  void printText(QByteArray text);
  void parseFontEscapeCode(QByteArray data);
  void parseEscapeCode(QByteArray data);

  void resetFont();
  int itemIndex(int row, int column);

  /// Převede kód na barvu, kód musí obsahovat 3/4 na začátku a nesmí mít m na konci
  static bool colorFromSequence(QByteArray code, QColor &clr);

public slots:
  void setColorExceptionList(QList<QColor> newlist, bool isBlacklist);

signals:
  void columnsChanged();

private:
  uint16_t cursorX = 0;
  uint16_t cursorY = 0;
  uint16_t cursorX_saved = 0;
  uint16_t cursorY_saved = 0;

  QByteArray buffer;

  QColor fontColor = Qt::white;
  QColor backColor = Qt::black;
  bool bold = false;
  bool underlined = false;

  bool m_showGrid = false;
  QList<GridItem> m_items;
  int m_columns = TERMINAL_DEFAULT_WIDTH;

  int cursorIndex = 0;
  Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)

  bool m_active = false;

  bool exeptionListIsBlacklist = false;
  QList<QColor> exceptionList;

  Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
  Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
  Q_PROPERTY(int cursorIndex READ getCursorIndex NOTIFY cursorIndexChanged)

public slots:
  void printToTerminal(QByteArray data);
  void setShowGrid(bool newShowGrid);
  Q_INVOKABLE void gridClicked(int index);

signals:
  /// Pošle zprávu do výpisu
  void sendMessage(QByteArray header, QByteArray message, MessageLevel::enumMessageLevel type, MessageTarget::enumMessageTarget target = MessageTarget::serial1);
  void activeChanged();
  void showGridChanged();
  void cursorIndexChanged();
  void gridClickedSignal(int x, int y);

public:
  bool active() const;
  void setActive(bool newActive);
  bool showGrid() const;

  static uint32_t colorCodes256(int code);
  static QMap<QString, QColor> colorCodes();
  int getCursorIndex() const;
  void setCursorIndex(int newCursorIndex);
};

#endif // ANSITERMINALMODEL_H
