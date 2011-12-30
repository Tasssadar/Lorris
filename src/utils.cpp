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

quint16 Utils::swapEndian16(quint16 x)
{
    x = quint16((x >> 8) | (x << 8));
}


quint32 Utils::swapEndian32(quint32 x)
{
    return ((x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24));
}

quint64 Utils::swapEndian64(quint64 x)
{
    return  ((x>>56) |
            ((x<<40) & 0x00FF000000000000) |
            ((x<<24) & 0x0000FF0000000000) |
            ((x<<8)  & 0x000000FF00000000) |
            ((x>>8)  & 0x00000000FF000000) |
            ((x>>24) & 0x0000000000FF0000) |
            ((x>>40) & 0x000000000000FF00) |
            (x<<56));
}

