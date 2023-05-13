
#include "variableexpressionparser.h"
#include "qdebug.h"

VariableExpressionParser::VariableExpressionParser(QObject *parent)
    : ExpressionParser{parent}
{

}

bool VariableExpressionParser::setExpression(QJSEngine &engine,QString expr)
{
    expr.replace("pi","PI");
    expr.replace("Pi","PI");
    expr = replaceUnitPrefixes(expr);
    expr = replaceFunctionNames(expr);
    auto result = engine.evaluate(expr);
    qDebug() << expr;
    if(result.isNumber()) {
        expression = expr;
        return true;
    }
    expression = "";
    return false;
}

double VariableExpressionParser::evaluate(QJSEngine &engine, bool *isOK)
{
    auto result = engine.evaluate(expression);
    *isOK = result.isNumber();
    return result.toNumber();
}

