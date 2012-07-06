/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef DATAFILEPARSER_H
#define DATAFILEPARSER_H

#include <QFile>
#include <QBuffer>
#include <vector>

enum DataBlocks
{
    BLOCK_STATIC_DATA,
    BLOCK_COLLAPSE_STATUS,
    BLOCK_COLLAPSE_STATUS2,
    BLOCK_DEVICE_TABS,
    BLOCK_DEVICE_TAB,
    BLOCK_CMD_TABS,
    BLOCK_CMD_TAB,
    BLOCK_DATA,
    BLOCK_WIDGETS,
    BLOCK_WIDGET,
    BLOCK_DATA_INDEX,

    BLOCK_SESSIONS_INFO,
    BLOCK_SESSION
};

class DataFileParser : public QBuffer
{
    Q_OBJECT
public:
    explicit DataFileParser(QByteArray *data, QObject *parent = 0);
    ~DataFileParser();

    bool seekToNextBlock(DataBlocks block, qint32 maxDist);
    bool seekToNextBlock(const char *block, qint32 maxDist);
    bool seekToNextBlock(DataBlocks block, DataBlocks toMax);
    bool seekToNextBlock(const char *block, const char* toMax);
    bool seekToNextBlock(const char *block, DataBlocks toMax);
    bool seekToNextBlock(DataBlocks block, const char* toMax);

    void writeBlockIdentifier(DataBlocks block);
    void writeBlockIdentifier(const char* block);

    char* getBlockName(DataBlocks block);

    void writeString(const QString& str);
    QString readString();

    template <typename T> void readVal(T& val);
    template <typename T> void writeVal(T val);

private:
    char *getBlockWithFormat(const char *block, quint8& lenght);

    int m_last_block;
};

template <typename T>
void DataFileParser::readVal(T& val)
{
    read((char*)&val, sizeof(T));
}

template <typename T>
void DataFileParser::writeVal(T val)
{
    write((char*)&val, sizeof(T));
}


#endif // DATAFILEPARSER_H
