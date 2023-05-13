
#ifndef VARIABLEEXPRESSIONPARSER_H
#define VARIABLEEXPRESSIONPARSER_H


#include "math/expressionparser.h"
#include <QObject>


class VariableExpressionParser : public ExpressionParser
{
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
