#ifndef NUM_FUNC_H
#define NUM_FUNC_H

#include <QString>

namespace Nums
{
    QString hexToString(quint8 data, bool withZeroEx = false)
    {
        QString result = QString::number(data, 16).toUpper();
        if(result.count()%2)
            result.push_front('0');
        if(withZeroEx)
            result.prepend("Ox");
        return result;
    }
}

#endif // NUM_FUNC_H
