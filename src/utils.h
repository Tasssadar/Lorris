#ifndef NUM_FUNC_H
#define NUM_FUNC_H

#include <QString>

class Utils
{
public:
    static QString hexToString(quint8 data, bool withZeroEx = false);
    static QString parseChar(char c);
    static quint16 swapEndian16(quint16 x);
    static quint32 swapEndian32(quint32 x);
    static quint64 swapEndian64(quint64 x);
};

#endif // NUM_FUNC_H
