
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
