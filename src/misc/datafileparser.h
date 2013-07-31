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
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QFileInfo>

#include "utils.h"

class QEventLoop;
class Connection;

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
    BLOCK_PACKET_LIMIT,
    BLOCK_FILTERS,

    BLOCK_TABWIDGET,
    BLOCK_WORKTAB,

    BLOCK_MAX
};

enum DataFileFlags
{
    DATAFLAG_COMPRESSED_OBSOLETE     = 0x01, // Obsolete
    DATAFLAG_COMPRESSED              = 0x02
};

enum DataFileTypes
{
    DATAFILE_NONE         = 0,
    DATAFILE_ANALYZER     = 1,
    DATAFILE_SESSION      = 2,

    DATAFILE_MAX
};

PACK_STRUCT(struct DataFileHeader
{
    DataFileHeader(quint8 data_type = DATAFILE_NONE);
    DataFileHeader(const DataFileHeader& other);

    char str[4];             // must be "LDTA" without null end
    quint16 version;
    quint32 flags;           // enum DataFileFlags
    quint8 data_type;        // enum DataFileTypes
    char md5[16];            // md5 hash of data which follows the header
    quint16 header_size;     // header size, for compatibility. Should be 64 for version >= 2
    quint32 compressed_block;
    quint32 lorris_rev;

    // SEE: align to 64 bytes
    char unused[27];
});

class DataFileParser : public QBuffer
{
    Q_OBJECT
public:
    typedef QScopedPointer<char, QScopedPointerArrayDeleter<char> > pStr;

    DataFileParser(QByteArray *data, QIODevice::OpenMode openMode, QString path = QString(),
                   QString name = QString(), QObject *parent = 0);
    ~DataFileParser();

    bool seekToNextBlock(DataBlocks block, qint32 maxDist);
    bool seekToNextBlock(const char *block, qint32 maxDist);
    bool seekToNextBlock(DataBlocks block, DataBlocks toMax);
    bool seekToNextBlock(const char *block, const char* toMax);
    bool seekToNextBlock(const char *block, DataBlocks toMax);
    bool seekToNextBlock(DataBlocks block, const char* toMax);

    void writeBlockIdentifier(DataBlocks block);
    void writeBlockIdentifier(const char* block);

    void writeString(const QString& str);
    QString readString();

    void writeColor(const QColor& color);
    QColor readColor();

    void writeConn(Connection *conn);
    bool readConn(quint8& type, QHash<QString, QVariant>& cfg);

    void writeVariantHash(const QHash<QString, QVariant>& hash);
    QHash<QString, QVariant> readVariantHash();

    QFileInfo getAttachmentFileInfo();
    QString getAttachmentsPath() const { return m_path; }

    template <typename T> void readVal(T& val);
    template <typename T> T readVal();
    template <typename T> void writeVal(T val);
    template <typename T> void writeVal(T val, quint64 pos);

    template <typename T> DataFileParser& operator <<(T data);
    DataFileParser& operator <<(const QString& str);
    DataFileParser& operator <<(const QColor& color);

    template <typename T> DataFileParser& operator >>(T &data);
    DataFileParser& operator >>(QString& str);
    DataFileParser& operator >>(QColor& color);

private:
    char *getBlockWithFormat(const char *block, quint8& lenght);

    int m_last_block;
    QString m_name;
    QString m_path;
    int m_attachmentId;
};

class DataFileBuilder
{
public:
    static QByteArray readAndCheck(QFile& file, DataFileTypes expectedType, bool *legacy = NULL, DataFileHeader *fillHeader = NULL);

    // Returns MD5 of written data. data is cleared!
    static QByteArray writeWithHeader(const QString& filename, QByteArray& data, bool compress, DataFileTypes type);

    static void dumpFileInfo(const QString& filename);

private:
    static void readHeader(QFile& file, DataFileHeader *header);
    static void writeHeader(QIODevice &file, DataFileHeader *header);
    static void dumpHeader(const DataFileHeader& header);

    static QByteArray writeWithHeader_private(const QString& filename, QByteArray& data, bool compress, DataFileTypes type);

    static QFuture<QByteArray> m_future;
    static QFutureWatcher<QByteArray> *m_watcher;
};

class ProgressReporter : public QObject
{
    Q_OBJECT

    friend class DataFileBuilder;

protected:
    ProgressReporter();
    ~ProgressReporter();

private slots:
    void showSavingNotice();

private:
    QTimer m_timer;
    bool m_showDone;
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

template <typename T>
void DataFileParser::writeVal(T val, quint64 pos)
{
    quint64 origin = this->pos();
    if(seek(pos))
        write((char*)&val, sizeof(T));
    seek(origin);
}

// NOTE: Operators << and >> have specialized methods
// for QString, QColor and Connection (and possibly some
// other, depends if this comment is outdated or not)
template <typename T>
DataFileParser& DataFileParser::operator <<(T data)
{
    writeVal<T>(data);
    return *this;
}

template <typename T>
DataFileParser& DataFileParser::operator >>(T &data)
{
    read((char*)&data, sizeof(T));
    return *this;
}

#endif // DATAFILEPARSER_H
