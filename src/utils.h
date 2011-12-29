#ifndef NUM_FUNC_H
#define NUM_FUNC_H

#include <QString>

class Utils
{
public:
    static QString hexToString(quint8 data, bool withZeroEx = false);
    static QString parseChar(char c);
};

#endif // NUM_FUNC_H
