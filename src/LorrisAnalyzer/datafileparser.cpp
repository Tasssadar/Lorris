/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "datafileparser.h"

DataFileParser::DataFileParser(QByteArray *data, QObject *parent) :
    QBuffer(data, parent)
{
    m_last_block = 0;
}

DataFileParser::~DataFileParser()
{

}

bool DataFileParser::seekToNextBlock(DataBlocks block, qint32 maxDist)
{
    char* block_name = getBlockName(block);
    if(!block_name)
        return false;
    bool res = seekToNextBlock(block_name, maxDist);
    delete[] block_name;
    return res;
}

bool DataFileParser::seekToNextBlock(const char *block, qint32 maxDist)
{
    quint8 lenght = 0;
    char* name = getBlockWithFormat(block, lenght);

    int index = data().indexOf(QByteArray(name, lenght), m_last_block);
    delete[] name;

    if(index == -1 || (maxDist != 0 && (index - m_last_block) > maxDist))
        return false;
    m_last_block = index + lenght;
    seek(m_last_block);
    return true;
}

bool DataFileParser::seekToNextBlock(const char *block, const char *toMax)
{
    quint8 lenght = 0;
    char* name = getBlockWithFormat(toMax, lenght);

    int index = data().indexOf(QByteArray(name, lenght), m_last_block);

    delete[] name;

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

bool DataFileParser::seekToNextBlock(const char *block, DataBlocks toMax)
{
    char* block_name = getBlockName(toMax);
    if(!block_name)
        return false;

    quint8 len;
    char *formatted = getBlockWithFormat(block_name, len);

    int index = data().indexOf(QByteArray(formatted, len), m_last_block);

    delete[] block_name;
    delete[] formatted;

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

bool DataFileParser::seekToNextBlock(DataBlocks block, const char *toMax)
{
    quint8 lenght = 0;
    char* name = getBlockWithFormat(toMax, lenght);

    int index = data().indexOf(QByteArray(name, lenght), m_last_block);

    delete[] name;

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

bool DataFileParser::seekToNextBlock(DataBlocks block, DataBlocks toMax)
{
    char* block_name = getBlockName(toMax);
    if(!block_name)
        return false;

    quint8 len;
    char *formatted = getBlockWithFormat(block_name, len);

    int index = data().indexOf(QByteArray(formatted, len), m_last_block);

    delete[] block_name;
    delete[] formatted;

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

char* DataFileParser::getBlockName(DataBlocks block)
{
    char* res = new char[100];

    switch(block)
    {
        case BLOCK_STATIC_DATA:      strcpy(res, "staticDataBlock"); break;
        case BLOCK_COLLAPSE_STATUS:  strcpy(res, "collapseWStatus"); break;
        case BLOCK_COLLAPSE_STATUS2: strcpy(res, "collapseWStatus2");break;
        case BLOCK_DEVICE_TABS:      strcpy(res, "deviceTabsBlock"); break;
        case BLOCK_DEVICE_TAB:       strcpy(res, "deviceTabBlock");  break;
        case BLOCK_CMD_TABS:         strcpy(res, "cmdTabsBlock");    break;
        case BLOCK_CMD_TAB:          strcpy(res, "cmdTabBlock");     break;
        case BLOCK_DATA:             strcpy(res, "dataBlock");       break;
        case BLOCK_WIDGETS:          strcpy(res, "widgetsBlock");    break;
        case BLOCK_WIDGET:           strcpy(res, "widgetBlock");     break;
        case BLOCK_DATA_INDEX:       strcpy(res, "dataIndexBlock");  break;
        default: return NULL;
    }
    return res;
}

void DataFileParser::writeBlockIdentifier(DataBlocks block)
{
    char* block_name = getBlockName(block);
    if(block_name)
        writeBlockIdentifier(block_name);
    delete[] block_name;
}

void DataFileParser::writeBlockIdentifier(const char *block)
{
    quint8 lenght = 0;

    char* name = getBlockWithFormat(block, lenght);
    write(name, lenght);
    delete[] name;
}

char *DataFileParser::getBlockWithFormat(const char *block, quint8& lenght)
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
