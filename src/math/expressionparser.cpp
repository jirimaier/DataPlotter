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

#include "expressionparser.h"
#include "qdebug.h"

ExpressionParser::ExpressionParser(QObject *parent) : QObject{parent} {
  siPrefixes.setPattern("(\\d+\\.?\\d*)\\s*([TMkmGun]?)");
  jsFunctions.setPattern("(\\b(acos|asin|atan2|atan|cbrt|ceil|cos|cosh|exp|floor|log10|log2|log|max|min|pow|random|round|sin|sinh|sqrt|tan|tanh|PI)\\b)");
}

QString ExpressionParser::replaceUnitPrefixes(QString &expr) const {
  expr.replace(QString::fromUtf8("\xc2\xb5"), "u"); // mu
  int cumulativeLength = 0;                         // cumulative length of replacements so far
  QRegularExpressionMatchIterator matchIterator = siPrefixes.globalMatch(expr);
  while (matchIterator.hasNext()) {
    QRegularExpressionMatch match = matchIterator.next();
    QString number = match.captured(1);
    QString prefix = match.captured(2);
    QString value = "";
    if (prefixes.contains(prefix))
      value = "*" + prefixes[prefix];
    QString replacement = number + value;
    int capturedStart = match.capturedStart() - cumulativeLength; // adjust start index
    int capturedLength = match.capturedLength();
    expr.replace(capturedStart, capturedLength, replacement);
    cumulativeLength += capturedLength - replacement.length(); // update cumulative length
  }
  return expr;
}

QString ExpressionParser::replaceFunctionNames(QString &expr) const {
  // Replace function names with "Math." + function name
  int offset = 0;
  QRegularExpressionMatchIterator i = jsFunctions.globalMatch(expr);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    QString mathFunction = "Math." + match.captured();
    int position = match.capturedStart() + offset;
    int length = match.capturedLength();
    expr.replace(position, length, mathFunction);
    offset += mathFunction.length() - length;
  }

  return expr;
}
