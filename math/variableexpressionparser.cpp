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

#include "variableexpressionparser.h"
#include "qdebug.h"

VariableExpressionParser::VariableExpressionParser(QObject *parent) : ExpressionParser{parent} {}

bool VariableExpressionParser::setExpression(QJSEngine &engine, QString expr) {
  expr.replace("pi", "PI");
  expr.replace("Pi", "PI");
  expr = replaceUnitPrefixes(expr);
  expr = replaceFunctionNames(expr);
  auto result = engine.evaluate(expr);
  if (result.isNumber()) {
    expression = expr;
    return true;
  }
  expression = "";
  return false;
}

double VariableExpressionParser::evaluate(QJSEngine &engine, bool *isOK) {
  auto result = engine.evaluate(expression);
  *isOK = result.isNumber();
  return result.toNumber();
}
