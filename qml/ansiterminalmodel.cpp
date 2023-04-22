#include "ansiterminalmodel.h"

AnsiTerminalModel::AnsiTerminalModel(QObject *parent) : QAbstractListModel(parent)
{
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
    clear();
}

int AnsiTerminalModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_items.count();
}

QVariant AnsiTerminalModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const GridItem &item = m_items[index.row()];

    switch (role) {
    case CharacterRole:
        return QVariant(item.character);
    case BackgroundColorRole:
        return QVariant(item.backgroundColor);
    case FontColorRole:
        return QVariant(item.fontColor);
    case BoldRole:
        return QVariant(item.bold);
    case UnderlineRole:
        return QVariant(item.underline);
    case ClickableRole:
        return QVariant(item.clickable);
    case ClickHighLightColorRole:
        return QVariant(item.backgroundColor.lightness()>127 ? "black" : "white");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AnsiTerminalModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CharacterRole] = "character";
    roles[BackgroundColorRole] = "backgroundColor";
    roles[FontColorRole] = "fontColor";
    roles[BoldRole] = "bold";
    roles[UnderlineRole] = "underline";
    roles[ClickableRole] = "clickable";
    roles[ClickHighLightColorRole] = "clickHighLightColor";
    return roles;
}

void AnsiTerminalModel::addItem(const GridItem &item)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
}

int AnsiTerminalModel::columns() const
{
    return m_columns;
}

void AnsiTerminalModel::setColumns(int newColumns)
{
    if (m_columns == newColumns)
        return;
    m_columns = newColumns;
    emit columnsChanged();
}

void AnsiTerminalModel::appendColumn()
{
    int index = columns();
    while(index<=m_items.length()) {
        beginInsertRows(QModelIndex(), index, index);
        m_items.insert(index,GridItem());
        index += columns()+1;
        endInsertRows();
    }

    setColumns(m_columns+1);
}

void AnsiTerminalModel::appendRow()
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+columns()-1);
    int count = m_items.length();
    while(m_items.length()<count+columns()) {
        m_items.append(GridItem());
    }
    endInsertRows();
}

QByteArray AnsiTerminalModel::nearestColorCode(QColor color)
{
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

void AnsiTerminalModel::clear()
{
    resetFont();
    beginResetModel();
    m_items.clear();
    endResetModel();

    for(int i = 0; i<14*10;i++)
        addItem(GridItem());
    setColumns(14);

    moveCursorAbsolute(0, 0);
}

void AnsiTerminalModel::clearCell(int column, int row)
{
    int index = itemIndex(row,column);
    beginRemoveRows(QModelIndex(),index,index);
    m_items.removeAt(index);
    endRemoveRows();

    beginInsertRows(QModelIndex(),index,index);
    m_items.insert(index,GridItem());
    endInsertRows();
}

void AnsiTerminalModel::printChar(QChar text)
{
    int index = itemIndex(cursorY, cursorX);
    beginRemoveRows(QModelIndex(),index,index);
    m_items.removeAt(index);
    endRemoveRows();

    bool clickable = !(exceptionList.contains(backColor) == exeptionListIsBlacklist);

    beginInsertRows(QModelIndex(),index,index);
    m_items.insert(index,GridItem(text,backColor,fontColor,bold,underlined,clickable));
    endInsertRows();

    moveCursorRelative(1, 0);
}

void AnsiTerminalModel::moveCursorAbsolute(int16_t x, int16_t y)
{  
    cursorX = (x > 0) ? x : 0;
    cursorY = (y > 0) ? y : 0;

    while(cursorX>=columns())
        appendColumn();
    while(cursorY>=rowCount()/columns())
        appendRow();

    Q_ASSERT(rowCount()%columns() == 0);
}

void AnsiTerminalModel::clearLine() {
    for (uint16_t i = 0; i < this->columns(); i++)
        clearCell(i, cursorY);
}

void AnsiTerminalModel::clearLine(uint16_t line) {
    for (uint16_t i = 0; i < this->columns(); i++)
        clearCell(i, line);
}

void AnsiTerminalModel::clearLineLeft() {
    for (uint16_t i = 0; i <= cursorX; i++)
        clearCell(i, cursorY);
}

void AnsiTerminalModel::clearLineRight() {
    for (uint16_t i = cursorX; i < this->columns(); i++)
        clearCell(i, cursorY);
}

void AnsiTerminalModel::clearDown() {
    clearLineRight();
    for (uint16_t i = cursorY + 1; i < (cursorY<=rowCount()/columns()); i++)
        clearLine(i);
}

void AnsiTerminalModel::printText(QByteArray bytes) {
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

        printChar(text.at(i));
    }
}

void AnsiTerminalModel::parseFontEscapeCode(QByteArray data) {
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

void AnsiTerminalModel::parseEscapeCode(QByteArray data) {
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
        clear();

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

void AnsiTerminalModel::resetFont()
{
    backColor = Qt::black;
    fontColor = Qt::white;
    bold = false;
    underlined = false;
}

int AnsiTerminalModel::itemIndex(int row, int column)
{
    return row*columns()+column;
}

void AnsiTerminalModel::setColorExceptionList(QList<QColor> newlist, bool isBlacklist)
{
    exceptionList = newlist;
    exeptionListIsBlacklist = isBlacklist;

    beginResetModel();
    for(auto &item : m_items) {
        item.clickable = !(exceptionList.contains(item.backgroundColor) == exeptionListIsBlacklist);
    }
    endResetModel();
}

bool AnsiTerminalModel::colorFromSequence(QByteArray code, QColor& clr) {
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

bool AnsiTerminalModel::active() const
{
    return m_active;
}

void AnsiTerminalModel::setActive(bool newActive)
{
    if (m_active == newActive)
        return;
    m_active = newActive;
    emit activeChanged();
}

void AnsiTerminalModel::clearUp() {
    clearLineLeft();
    for (uint16_t i = 0; i < cursorX; i++)
        clearLine(i);
}

void AnsiTerminalModel::printToTerminal(QByteArray data) {
    setActive(true);
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


