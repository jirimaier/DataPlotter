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

#ifndef SIMPLEEXPRESSIONPARSER_H
#define SIMPLEEXPRESSIONPARSER_H

#include "math/expressionparser.h"
#include <QObject>

class SimpleExpressionParser : public ExpressionParser {
  Q_OBJECT
public:
  explicit SimpleExpressionParser(QObject *parent = nullptr);

signals:

  // ExpressionParser interface
public:
  double evaluate(const QString &expression, bool *isOK);
  QString validchars;
  bool validate(const QString &expression);
  QJSEngine engine;
};

#endif // SIMPLEEXPRESSIONPARSER_H
