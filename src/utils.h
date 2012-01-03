#ifndef NUM_FUNC_H
#define NUM_FUNC_H

#include <QString>

class Utils
{
public:
    static QString hexToString(quint8 data, bool withZeroEx = false);
    static QString parseChar(char c);
    template <typename T> static inline void swapEndian(char *val);
};

template <typename T>
void Utils::swapEndian(char *val)
{
    for(qint8 i = sizeof(T); i > 0; i -= 2, ++val)
        std::swap(*val, *(val + i - 1));
}


#endif // NUM_FUNC_H
