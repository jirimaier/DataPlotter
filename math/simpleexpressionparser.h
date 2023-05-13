
#ifndef SIMPLEEXPRESSIONPARSER_H
#define SIMPLEEXPRESSIONPARSER_H


#include "math/expressionparser.h"
#include <QObject>


class SimpleExpressionParser : public ExpressionParser
{
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
