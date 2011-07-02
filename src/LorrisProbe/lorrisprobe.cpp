#include "lorrisprobe.h"


LorrisProbe::LorrisProbe()
{
}

LorrisProbe::~LorrisProbe()
{

}

QWidget *LorrisProbe::GetTab(QWidget *parent)
{
    return new QWidget(parent);
}
