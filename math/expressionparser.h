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

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "qjsengine.h"
#include "qregularexpression.h"
#include <QObject>

class ExpressionParser : public QObject {
  Q_OBJECT

protected:
  QRegularExpression siPrefixes;
  QRegularExpression jsFunctions;
  QRegularExpression decimalSeparatorRegex;
  QString replaceUnitPrefixes(QString &expr) const;
  QString replaceFunctionNames(QString &expr) const;
  QMap<QString, QString> prefixes{{"E", "1e18"}, {"P", "1e15"}, {"T", "1e12"}, {"G", "1e9"}, {"M", "1e6"}, {"k", "1e3"}, {"m", "1e-3"}, {"u", "1e-6"}, {"n", "1e-9"}, {"p", "1e-12"}, {"f", "1e-15"}, {"a", "1e-18"}};

public:
  explicit ExpressionParser(QObject *parent = nullptr);
};

#endif // EXPRESSIONPARSER_H
