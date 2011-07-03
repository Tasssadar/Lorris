#include "lorrisprobe.h"


LorrisProbe::LorrisProbe() : WorkTab()
{
}

LorrisProbe::~LorrisProbe()
{

}

QWidget *LorrisProbe::GetTab(QWidget *parent)
{
    return new QWidget(parent);
}

void LorrisProbe::readData(QByteArray data)
{

}
