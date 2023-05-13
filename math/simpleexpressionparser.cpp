
#include "simpleexpressionparser.h"

SimpleExpressionParser::SimpleExpressionParser(QObject *parent)
    : ExpressionParser{parent}
{
    validchars = "0123456789+-*/ekMGmun., ";
    validchars.append(QString::fromUtf8("\xc2\xb5"));
}

double SimpleExpressionParser::evaluate(const QString &expression, bool *isOK)
{
    QString expr = expression.trimmed();
    expr.replace(",",".");
    expr.replace(QString::fromUtf8("\xc2\xb5"),"u");
    if(expr.left(1)=="/")
        expr.prepend("1");
    for(const QString &prefix : prefixes.keys())
        expr.replace(prefix,"*"+prefixes[prefix]);
    auto result = engine.evaluate(expr);

    if(isOK != nullptr)
        *isOK = (result.errorType()==0);
    if(result.isNumber())
        return result.toNumber();
    else
        return 0;
}

bool SimpleExpressionParser::validate(const QString &expression)
{
    for(QChar c : expression) {
        if(!validchars.contains(c))
            return false;
    }
    return true;
}

