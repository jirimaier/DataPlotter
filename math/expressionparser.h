
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
