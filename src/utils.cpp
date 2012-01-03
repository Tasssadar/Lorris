#include "utils.h"

QString Utils::hexToString(quint8 data, bool withZeroEx)
{
    QString result = QString::number(data, 16).toUpper();
    if(result.count()%2)
        result.push_front('0');
    if(withZeroEx)
        result.prepend("0x");
    return result;
}

QString Utils::parseChar(char c)
{
    switch(c)
    {
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\f': return "\\f";
        default:   return QString((QChar)c);
    }
}
