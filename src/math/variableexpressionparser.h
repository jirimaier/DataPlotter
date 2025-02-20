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

#ifndef VARIABLEEXPRESSIONPARSER_H
#define VARIABLEEXPRESSIONPARSER_H

#include "math/expressionparser.h"
#include <QObject>

class VariableExpressionParser : public ExpressionParser {
  Q_OBJECT

  QString expression;

public:
  explicit VariableExpressionParser(QObject *parent = nullptr);

signals:

  // ExpressionParser interface
public:
  bool setExpression(QJSEngine &engine, QString expr);
  double evaluate(QJSEngine &engine, bool *isOK = nullptr);
};

#endif // VARIABLEEXPRESSIONPARSER_H
