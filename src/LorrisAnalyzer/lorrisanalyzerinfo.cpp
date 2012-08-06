/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "lorrisanalyzerinfo.h"
#include "lorrisanalyzer.h"

static const LorrisAnalyzerInfo info;

LorrisAnalyzerInfo::LorrisAnalyzerInfo() : WorkTabInfo()
{

}

WorkTab *LorrisAnalyzerInfo::GetNewTab()
{
    return new LorrisAnalyzer();
}

QString LorrisAnalyzerInfo::GetName()
{
    return QObject::tr("Analyzer");
}

QString LorrisAnalyzerInfo::GetDescription()
{
    return QObject::tr("Analyzer can parse any data you give it and display it in various ways. "
                       "You can mark packets in the data source, mark their headers or tails, mark individual data blocks, "
                       "select their data type and the way they will be shown to you.");
}

QStringList LorrisAnalyzerInfo::GetHandledFiles()
{
    return (QStringList() << "cldta" << "ldta");
}

QString LorrisAnalyzerInfo::GetIdString()
{
    return "LorrisAnalyzer";
}
