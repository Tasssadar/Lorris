#include "programmer.h"

void Programmer::executeText(QByteArray const &, quint8, chip_definition &)
{
    Q_ASSERT(0);
}

bool Programmer::setPwmFreq(uint32_t, float)
{
    return false;
}
