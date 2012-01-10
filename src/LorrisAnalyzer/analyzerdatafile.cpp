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

#include "analyzerdatafile.h"

AnalyzerDataFile::AnalyzerDataFile(const QString &filename, QObject *parent) :
    QFile(filename, parent)
{
    m_last_block = 0;
}

bool AnalyzerDataFile::open ( OpenMode mode )
{
    bool res = QFile::open(mode);
    if(!res)
        return res;
    if(mode & QIODevice::ReadOnly)
    {
        m_data = readAll();
        seek(0);
    }
    return res;
}

bool AnalyzerDataFile::seekToNextBlock(DataBlocks block, qint32 maxDist)
{
    char* block_name = getBlockName(block);
    if(!block_name)
        return false;
    bool res = seekToNextBlock(block_name, maxDist);
    delete[] block_name;
    return res;
}

bool AnalyzerDataFile::seekToNextBlock(const char *block, qint32 maxDist)
{
    quint8 lenght = 0;
    char* name = getBlockWithFormat(block, lenght);

    int index = m_data.indexOf(QByteArray(name, lenght), m_last_block);
    delete name;

    if(index == -1 || (maxDist != 0 && (index - m_last_block) > maxDist))
        return false;
    m_last_block = index + lenght;
    seek(m_last_block);
    return true;
}

bool AnalyzerDataFile::seekToNextBlock(const char *block, const char *toMax)
{
    quint8 lenght = 0;
    char* name = getBlockWithFormat(toMax, lenght);

    int index = m_data.indexOf(QByteArray(name, lenght), m_last_block);

    delete[] name;

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

bool AnalyzerDataFile::seekToNextBlock(const char *block, DataBlocks toMax)
{
    char* block_name = getBlockName(toMax);
    if(!block_name)
        return false;

    quint8 len;
    char *formatted = getBlockWithFormat(block_name, len);

    int index = m_data.indexOf(QByteArray(formatted, len), m_last_block);

    delete[] block_name;
    delete[] formatted;

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

bool AnalyzerDataFile::seekToNextBlock(DataBlocks block, const char *toMax)
{
    quint8 lenght = 0;
    char* name = getBlockWithFormat(toMax, lenght);

    int index = m_data.indexOf(QByteArray(name, lenght), m_last_block);

    delete[] name;

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}


bool AnalyzerDataFile::seekToNextBlock(DataBlocks block, DataBlocks toMax)
{
    char* block_name = getBlockName(toMax);
    if(!block_name)
        return false;

    quint8 len;
    char *formatted = getBlockWithFormat(block_name, len);

    int index = m_data.indexOf(QByteArray(formatted, len), m_last_block);

    delete[] block_name;
    delete[] formatted;

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

char* AnalyzerDataFile::getBlockName(DataBlocks block)
{
    char* res = new char[100];

    switch(block)
    {
        case BLOCK_STATIC_DATA: strcpy(res, "staticDataBlock"); break;
        case BLOCK_DEVICE_TABS: strcpy(res, "deviceTabsBlock"); break;
        case BLOCK_DEVICE_TAB:  strcpy(res, "deviceTabBlock");  break;
        case BLOCK_CMD_TABS:    strcpy(res, "cmdTabsBlock");    break;
        case BLOCK_CMD_TAB:     strcpy(res, "cmdTabBlock");     break;
        case BLOCK_DATA:        strcpy(res, "dataBlock");       break;
        case BLOCK_WIDGETS:     strcpy(res, "widgetsBlock");    break;
        case BLOCK_WIDGET:      strcpy(res, "widgetBlock");     break;
        default: return NULL;
    }
    return res;
}

void AnalyzerDataFile::writeBlockIdentifier(DataBlocks block)
{
    char* block_name = getBlockName(block);
    if(block_name)
        writeBlockIdentifier(block_name);
    delete block_name;
}

void AnalyzerDataFile::writeBlockIdentifier(const char *block)
{
    quint8 lenght = 0;

    char* name = getBlockWithFormat(block, lenght);
    write(name, lenght);
    delete[] name;
}

char *AnalyzerDataFile::getBlockWithFormat(const char *block, quint8& lenght)
{
    char* name = new char[100];
    name[0] = 0x80;
    ++name;
    strcpy(name, block);
    lenght = strlen(name) + 3;
    name[strlen(name) + 1] = 0x80;
    --name;
    return name;
}
