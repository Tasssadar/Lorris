#include "lorrisanalyzerinfo.h"
#include "lorrisanalyzer.h"

LorrisAnalyzerInfo::LorrisAnalyzerInfo()
{

}

LorrisAnalyzerInfo::~LorrisAnalyzerInfo()
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
    return QObject::tr("Analyzer can parse any data you give it and show them as anything you like."
                       "You can mark packet in data source, mark its header or bottom, mark individual data blocks, "
                       "select their data type and the way they will be showed to you.");
}
