/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QScopedPointer>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QEventLoop>
#include <QtConcurrentRun>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QString>

#include "datafileparser.h"
#include "config.h"
#include "../ui/tooltipwarn.h"
#include "../connection/connection.h"
#include "../revision.h"

#define MD5(x) QCryptographicHash::hash(x, QCryptographicHash::Md5)

static const char *blockNames[] = {
    "staticDataBlock",     // BLOCK_STATIC_DATA
    "collapseWStatus",     // BLOCK_COLLAPSE_STATUS
    "collapseWStatus2",    // BLOCK_COLLAPSE_STATUS2
    "deviceTabsBlock",     // BLOCK_DEVICE_TABS
    "deviceTabBlock",      // BLOCK_DEVICE_TAB
    "cmdTabsBlock",        // BLOCK_CMD_TABS
    "cmdTabBlock",         // BLOCK_CMD_TAB
    "dataBlock",           // BLOCK_DATA
    "widgetsBlock",        // BLOCK_WIDGETS
    "widgetBlock",         // BLOCK_WIDGET
    "dataIndexBlock",      // BLOCK_DATA_INDEX
    "packetLimits",        // BLOCK_PACKET_LIMIT
    "filterBlock",         // BLOCK_FILTERS

    "tabWidget",           // BLOCK_TABWIDGET
    "tabWidgetTab",        // BLOCK_WORKTAB
};

DataFileHeader::DataFileHeader(quint8 data_type)
{
    memset(&str[0], 0, sizeof(DataFileHeader));

    str[0] = 'L'; str[1] = 'D'; str[2] = 'T'; str[3] = 'A';
    version = 2;
    this->data_type = data_type;
    header_size = 64;
    compressed_block = UINT_MAX;
    lorris_rev = REVISION;

    memset(&unused[0], 0xFF, sizeof(unused));
}

DataFileHeader::DataFileHeader(const DataFileHeader& other)
{
    memcpy(&str[0], &other.str[0], sizeof(DataFileHeader));
}

DataFileParser::DataFileParser(QByteArray *data, QIODevice::OpenMode openMode, QString path, QString name, QObject *parent) :
    QBuffer(data, parent)
{
    Q_ASSERT(sizeof_array(blockNames) == BLOCK_MAX);

    m_last_block = 0;
    m_name = name;
    m_path = path;
    m_attachmentId = 0;
    open(openMode);
}

DataFileParser::~DataFileParser()
{

}

bool DataFileParser::seekToNextBlock(DataBlocks block, qint32 maxDist)
{
    Q_ASSERT(block < BLOCK_MAX);
    return seekToNextBlock(blockNames[block], maxDist);
}

bool DataFileParser::seekToNextBlock(const char *block, qint32 maxDist)
{
    quint8 lenght = 0;

    pStr name(getBlockWithFormat(block, lenght));
    int index = data().indexOf(QByteArray(name.data(), lenght), m_last_block);

    if(index == -1 || (maxDist != 0 && (index - m_last_block) > maxDist))
        return false;
    m_last_block = index + lenght;
    seek(m_last_block);
    return true;
}

bool DataFileParser::seekToNextBlock(const char *block, const char *toMax)
{
    quint8 lenght = 0;
    pStr name(getBlockWithFormat(toMax, lenght));
    int index = data().indexOf(QByteArray(name.data(), lenght), m_last_block);

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

bool DataFileParser::seekToNextBlock(const char *block, DataBlocks toMax)
{
    Q_ASSERT(toMax < BLOCK_MAX);

    quint8 len;
    pStr formatted(getBlockWithFormat(blockNames[toMax], len));
    int index = data().indexOf(QByteArray(formatted.data(), len), m_last_block);

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

bool DataFileParser::seekToNextBlock(DataBlocks block, const char *toMax)
{
    quint8 lenght = 0;
    pStr name(getBlockWithFormat(toMax, lenght));
    int index = data().indexOf(QByteArray(name.data(), lenght), m_last_block);

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

bool DataFileParser::seekToNextBlock(DataBlocks block, DataBlocks toMax)
{
    Q_ASSERT(toMax < BLOCK_MAX);

    quint8 len;
    pStr formatted(getBlockWithFormat(blockNames[toMax], len));

    int index = data().indexOf(QByteArray(formatted.data(), len), m_last_block);

    quint32 dist = 0;
    if(index != -1)
        dist = index - m_last_block;

    return seekToNextBlock(block, dist);
}

void DataFileParser::writeBlockIdentifier(DataBlocks block)
{
    Q_ASSERT(block < BLOCK_MAX);
    writeBlockIdentifier(blockNames[block]);
}

void DataFileParser::writeBlockIdentifier(const char *block)
{
    quint8 lenght = 0;

    pStr name(getBlockWithFormat(block, lenght));
    write(name.data(), lenght);
}

char *DataFileParser::getBlockWithFormat(const char *block, quint8& lenght)
{
    lenght = strlen(block) + 3;
    char* name = new char[lenght+1];
    sprintf(name, "%c%s%c%c", 0x80, block, 0, 0x80);
    return name;
}

void DataFileParser::writeString(const QString &str)
{
    QByteArray utf = str.toUtf8();
    quint32 size = utf.size();

    write((char*)&size, sizeof(size));
    write(utf);
}

QString DataFileParser::readString()
{
    quint32 size = 0;
    read((char*)&size, sizeof(size));

    QByteArray raw = read(size);
    return QString::fromUtf8(raw.data(), size);
}

void DataFileParser::writeColor(const QColor &color)
{
    writeVal(color.isValid());

    if(!color.isValid())
        return;

    writeVal(color.rgb());
}

QColor DataFileParser::readColor()
{
    bool valid = readVal<bool>();
    if(!valid)
        return QColor();

    return QColor(readVal<QRgb>());
}

void DataFileParser::writeConn(Connection *conn)
{
    if(!conn || !conn->canSaveToSession())
    {
        writeVal(false);
        return;
    }

    writeVal(true);
    writeVal(conn->getType());

    QHash<QString, QVariant> cfg = conn->config();
    writeVal((int)cfg.count());
    for(QHash<QString, QVariant>::iterator itr = cfg.begin(); itr != cfg.end(); ++itr)
    {
        writeString(itr.key());
        writeVal((int)(*itr).type());

        switch((*itr).type())
        {
            case QVariant::String:
                writeString((*itr).toString());
                break;
            case QVariant::Int:
            case QVariant::UInt:
                writeVal((*itr).value<int>());
                break;
            case QVariant::LongLong:
                writeVal((*itr).value<qint64>());
                break;
            default:
                break;
        }
    }
}

bool DataFileParser::readConn(quint8 &type, QHash<QString, QVariant> &cfg)
{
    if(!readVal<bool>())
        return false;

    type = readVal<quint8>();
    int count = readVal<int>();

    for(int i = 0; i < count; ++i)
    {
        QString key = readString();
        int val_type = readVal<int>();

        QVariant val;
        switch(val_type)
        {
            case QVariant::String:
                val = readString();
                break;
            case QVariant::Int:
            case QVariant::UInt:
                val = readVal<int>();
                val.convert((QVariant::Type)val_type);
                break;
            case QVariant::LongLong:
                val = readVal<qint64>();
                break;
            default:
                break;
        }
        cfg.insert(key, val);
    }
    return true;
}

QString DataFileParser::getAttachmentFilename()
{
    if(m_name.isEmpty() || m_path.isEmpty())
        return QString();

    return QString("%1_%2_at%3.cldta").arg(m_path).arg(m_name).arg(m_attachmentId++);
}

DataFileParser& DataFileParser::operator <<(const QString& str)
{
    writeString(str);
    return *this;
}

DataFileParser& DataFileParser::operator <<(const QColor& color)
{
    writeColor(color);
    return *this;
}

DataFileParser& DataFileParser::operator >>(QString& str)
{
    str = readString();
    return *this;
}

DataFileParser& DataFileParser::operator >>(QColor& color)
{
    color = readColor();
    return *this;
}

QFuture<QByteArray> DataFileBuilder::m_future;
QFutureWatcher<QByteArray> *DataFileBuilder::m_watcher = NULL;

QByteArray DataFileBuilder::readAndCheck(QFile &file, DataFileTypes expectedType, bool *legacy, DataFileHeader *fillHeader)
{
    if(!file.isOpen() && !file.open(QIODevice::ReadOnly))
        throw QObject::tr("Cannot open file \"%1\"!").arg(file.fileName());

    char str[5] = { 0, 0, 0, 0, 0 };
    if(file.read(str, 4) != 4)
        throw QObject::tr("Corrupted data file");

    file.seek(0);

    bool compressed = file.fileName().contains(".cldta", Qt::CaseInsensitive);
    QScopedPointer<DataFileHeader> header;

    // with header
    if(strcmp(str, "LDTA") == 0)
    {
        if(file.size() < (qint64)sizeof(DataFileHeader))
            throw QObject::tr("Corrupted data file");

        header.reset(new DataFileHeader);
        readHeader(file, header.data());

        if(expectedType != DATAFILE_NONE && header->data_type != expectedType)
            throw QObject::tr("This file is not of expected content type");

        compressed = (header->flags & DATAFLAG_COMPRESSED);

        if(fillHeader)
            memcpy(fillHeader, header.data(), sizeof(DataFileHeader));
    }

    if(legacy)
        *legacy = header.isNull();

    QByteArray data = file.read(file.size());

    if(header && QByteArray::fromRawData(header->md5, sizeof(header->md5)) != MD5(data))
    {
        if(qApp)
        {
            QMessageBox box(QMessageBox::Question, QObject::tr("Error"),
                        QObject::tr("Corrupted data file - MD5 checksum does not match"),
                        QMessageBox::Yes | QMessageBox::No);
            box.setInformativeText(QObject::tr("Load anyway?"));

            if(box.exec() == QMessageBox::No)
                throw QObject::tr("Corrupted data file - MD5 checksum does not match");
        }
        else
            printf("MD5 checksums do not match!\n");
    }


    if(compressed && header && (header->flags & DATAFLAG_COMPRESSED))
    {
        char *end = data.data() + data.size();
        quint32 blocks = *( (quint32*) (end-sizeof(quint32)) );
        data.chop(sizeof(blocks));
        end = data.data() + data.size();

        std::vector<QByteArray> uncompressed(blocks);
        quint32 resSize = 0;
        for(quint32 i = 0; i < blocks; ++i)
        {
            end -= sizeof(quint32);
            quint32 size = *((quint32*)end);

            uncompressed[i] = qUncompress((const uchar*)(end-size), size);
            resSize += uncompressed[i].size();

            data.chop(size + sizeof(quint32));
            end = data.data() + data.size();
        }

        data.clear();
        data.reserve(resSize);
        while(!uncompressed.empty())
        {
            data.append(uncompressed.back());
            uncompressed.pop_back();
        }
    }
    else if(compressed) // Obsolete
        data = qUncompress(data);

    return data;
}

QByteArray DataFileBuilder::writeWithHeader(const QString& filename, QByteArray &data, bool compress, DataFileTypes type)
{
    QFile testFile(filename);
    if(!testFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        throw QObject::tr("Cannot open file \"%1\"!").arg(filename);
    testFile.close();

    // wait for last save to finish
    if(m_watcher)
        throw QObject::tr("Another file is currently saving!");

    m_watcher = new QFutureWatcher<QByteArray>();
    QEventLoop ev;
    QObject::connect(m_watcher, SIGNAL(finished()), &ev, SLOT(quit()));

    m_future = QtConcurrent::run(&DataFileBuilder::writeWithHeader_private, filename, data, compress, type);
    m_watcher->setFuture(m_future);

    ProgressReporter reporter;
    ev.exec();

    QByteArray res = m_watcher->result();
    delete m_watcher;
    m_watcher = NULL;
    return res;
}

QByteArray DataFileBuilder::writeWithHeader_private(const QString& filename, QByteArray& data, bool compress, DataFileTypes type)
{
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return QByteArray();

    DataFileHeader header(type);
    if(compress)
    {
        header.flags |= DATAFLAG_COMPRESSED;
        header.compressed_block = sConfig.get(CFG_QUINT32_COMPRESS_BLOCK);

        std::vector<QByteArray> compressed(data.size()/header.compressed_block + 1);

        quint32 chunk = (std::min)(header.compressed_block, (quint32)data.size());
        char *end = data.data() + data.size();
        quint32 size = 0;
        for(quint32 i = 0; !data.isEmpty(); ++i)
        {
            compressed[i] = qCompress((const uchar*)(end-chunk), chunk);
            data.chop(chunk);
            data.squeeze();

            size += compressed[i].size() + 4;

            chunk = (std::min)(header.compressed_block, (quint32)data.size());
            end = data.data() + data.size();
        }

        data.reserve(size + 4);
        quint32 blocks = compressed.size();
        while(!compressed.empty())
        {
            const QByteArray& ar = compressed.back();
            data.append(ar);

            size = ar.size();
            data.append((char*)&size, sizeof(size));

            compressed.pop_back();
        }
        data.append((char*)&blocks, sizeof(blocks));
    }

    QByteArray md5 = MD5(data);
    std::copy(md5.data(), md5.data()+sizeof(header.md5), header.md5);

    {
        QBuffer buff;
        buff.open(QIODevice::WriteOnly);
        writeHeader(buff, &header);
        data.prepend(buff.data());
    }

    file.write(data);
    data = MD5(data);
    return data;
}

void DataFileBuilder::readHeader(QFile &file, DataFileHeader *header)
{
    file.seek(0);

    file.read((char*)&header->str[0], sizeof(DataFileHeader));

    if(header->version >= 2)
        file.seek(header->header_size);
}

void DataFileBuilder::writeHeader(QIODevice &file, DataFileHeader *header)
{
    Q_ASSERT(sizeof(DataFileHeader) == header->header_size);

    file.seek(0);
    file.write((char*)header->str, sizeof(DataFileHeader));
    file.seek(header->header_size);
}

void DataFileBuilder::dumpFileInfo(const QString& filename)
{
    QFile f(filename);
    try
    {
        bool legacy = false;
        DataFileHeader header;
        QByteArray data = readAndCheck(f, DATAFILE_NONE, &legacy, &header);
        f.close();

        if(legacy) printf("This file does not have header\n");
        else       dumpHeader(header);

        printf("\nData blocks:\n");
        int cur = 0;
        char *st, *itr;
        int offset = legacy ? 0 : sizeof(DataFileHeader);
        while((cur = data.indexOf((char)0x80, cur)) != -1)
        {
            st = itr = data.data() + cur + 1;
            for(; *itr >= 0; ++itr)
            {
                if(*itr == 0)
                {
                    if(*(itr+1) == (char)0x80 && itr != st)
                        printf("%08X: %s\n", cur+offset, st);
                    break;
                }
            }
            ++cur;
        }
    }
    catch (const QString& str)
    {
        printf("ERROR: %s\n", str.toStdString().c_str());
    }
}

void DataFileBuilder::dumpHeader(const DataFileHeader& header)
{
    QString flags;
    if(header.flags != 0)
    {
        if(header.flags & DATAFLAG_COMPRESSED_OBSOLETE)
            flags += "DATAFLAG_COMPRESSED_OBSOLETE ";
        if(header.flags & DATAFLAG_COMPRESSED)
            flags += "DATAFLAG_COMPRESSED ";
    } else flags = "none ";

    QString type;
    switch(header.data_type)
    {
        case DATAFILE_NONE:     type = "DATAFILE_NONE"; break;
        case DATAFILE_ANALYZER: type = "DATAFILE_ANALYZER"; break;
        case DATAFILE_SESSION:  type = "DATAFILE_SESSION"; break;
        default:                type = "UNKNOWN"; break;
    }

    QString md5 = Utils::toBase16((quint8*)header.md5, (quint8*)header.md5+16);
    printf("Header:\n");
    printf("             str: %c%c%c%c\n", header.str[0], header.str[1], header.str[2], header.str[3]);
    printf("         version: %u\n", header.version);
    printf("           flags: 0x%X - %s\n", header.flags, flags.toStdString().c_str());
    printf("       data_type: %u (%s)\n", header.data_type, type.toStdString().c_str());
    printf("             md5: %s\n", md5.toStdString().c_str());
    printf("     header_size: %u\n", header.header_size);
    printf("compressed_block: %u (%.1f MB)\n", header.compressed_block, float(header.compressed_block)/1024/1024);
    printf("      lorris_rev: %u\n", header.lorris_rev);
}

ProgressReporter::ProgressReporter() : QObject()
{
    m_showDone = false;
    m_timer.start(500);
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), SLOT(showSavingNotice()));
}

ProgressReporter::~ProgressReporter()
{
    if(m_showDone)
    {
        ToolTipWarn *w = new ToolTipWarn(tr("Data file saved"), NULL, NULL, 3000, ":/actions/info");
        w->toRightBottom();
    }
}

void ProgressReporter::showSavingNotice()
{
    ToolTipWarn *w = new ToolTipWarn(tr("Saving data file..."), NULL, NULL, -1);
    connect(this, SIGNAL(destroyed()), w, SLOT(deleteLater()));

    w->showSpinner();
    w->toRightBottom();

    m_showDone = true;
}
