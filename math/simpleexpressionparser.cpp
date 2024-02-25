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

#include "simpleexpressionparser.h"

SimpleExpressionParser::SimpleExpressionParser(QObject *parent) : ExpressionParser{parent} {
  validchars = "0123456789+-*/ekMGTPEmunpfa., ";
  validchars.append(QString::fromUtf8("\xc2\xb5"));
}

double SimpleExpressionParser::evaluate(const QString &expression, bool *isOK) {
  QString expr = expression.trimmed();
  expr.replace(",", ".");
  expr.replace(QString::fromUtf8("\xc2\xb5"), "u");
  if (expr.at(0) == '/')
    expr.prepend("1");
  for (auto it = prefixes.begin(); it != prefixes.end(); it++)
    expr.replace(it.key(), "*" + prefixes[it.key()]);
  auto result = engine.evaluate(expr);

  if (isOK != nullptr)
    *isOK = !result.isError();
  if (result.isNumber())
    return result.toNumber();
  else
    return 0;
}

bool SimpleExpressionParser::validate(const QString &expression) {
  for (QChar c : expression) {
    if (!validchars.contains(c))
      return false;
  }
  return true;
}
