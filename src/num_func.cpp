#include "num_func.h"

QString Nums::hexToString(quint8 data, bool withZeroEx)
{
    QString result = QString::number(data, 16).toUpper();
    if(result.count()%2)
        result.push_front('0');
    if(withZeroEx)
        result.prepend("0x");
    return result;
}

