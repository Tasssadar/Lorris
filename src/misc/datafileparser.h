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

    BLOCK_TABWIDGET,
    BLOCK_WORKTAB
};

enum DataFileFlags
{
    DATAFLAG_COMPRESSED = 0x01
};

enum DataFileTypes
{
    DATAFILE_NONE         = 0,
    DATAFILE_ANALYZER     = 1,
    DATAFILE_SESSION      = 2,

    DATAFILE_MAX
};

struct DataFileHeader
{
    DataFileHeader(quint8 data_type = DATAFILE_NONE);
    DataFileHeader(const DataFileHeader& other);

    char str[4];        // must be "LDTA" without null end
    quint16 version;
    quint32 flags;      // enum DataFileFlags
    quint8 data_type;   // enum DataFileTypes
    char md5[16];       // md5 hash of data which follows the header

    // SEE: align to 64 bytes
    char unused[37];
};

class DataFileParser : public QBuffer
{
    Q_OBJECT
public:
    explicit DataFileParser(QByteArray *data, QIODevice::OpenMode openMode, QObject *parent = 0);
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
    template <typename T> T readVal();
    template <typename T> void writeVal(T val);

private:
    char *getBlockWithFormat(const char *block, quint8& lenght);

    int m_last_block;
};

class DataFileBuilder
{
public:
    static QByteArray readAndCheck(QFile& file, DataFileTypes expectedType, bool *legacy = NULL);

    // Returns MD5 of written data
    static QByteArray writeWithHeader(QFile& file, QByteArray data, bool compress, DataFileTypes type);

private:
    static void readHeader(QFile& file, DataFileHeader *header);
    static void writeHeader(QIODevice &file, DataFileHeader *header);
};

template <typename T>
void DataFileParser::readVal(T& val)
{
    read((char*)&val, sizeof(T));
}

template <typename T>
T DataFileParser::readVal()
{
    T t;
    read((char*)&t, sizeof(t));
    return t;
}

template <typename T>
void DataFileParser::writeVal(T val)
{
    write((char*)&val, sizeof(T));
}

#endif // DATAFILEPARSER_H
