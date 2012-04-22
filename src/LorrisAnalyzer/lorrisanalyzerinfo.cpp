/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

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
