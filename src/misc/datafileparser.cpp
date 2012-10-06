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

#include "datafileparser.h"
#include "config.h"
#include "../ui/tooltipwarn.h"
#include "../connection/connection.h"

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
    "filterBlock",         // BLOCK_FILTERS

    "tabWidget",           // BLOCK_TABWIDGET
    "tabWidgetTab",        // BLOCK_WORKTAB
};

DataFileHeader::DataFileHeader(quint8 data_type)
{
    str[0] = 'L'; str[1] = 'D'; str[2] = 'T'; str[3] = 'A';
    version = 2;
    flags = 0;
    this->data_type = data_type;
    std::fill(md5, md5 + sizeof(md5), 0);
    header_size = 64;
    compressed_block = UINT_MAX;
    std::fill(unused, unused + sizeof(unused), 0xFF);
}

DataFileHeader::DataFileHeader(const DataFileHeader& other)
{
    std::copy(other.str, other.str+sizeof(str), str);
    version = other.version;
    flags = other.flags;
    data_type = other.data_type;
    std::copy(other.md5, other.md5 + sizeof(md5), md5);
    header_size = other.header_size;
    compressed_block = other.compressed_block;
    std::copy(other.unused, other.unused + sizeof(unused), unused);
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

void DataFileParser::writeConn(PortConnection *conn)
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

QFuture<QByteArray> DataFileBuilder::m_future;
QFutureWatcher<QByteArray> *DataFileBuilder::m_watcher = NULL;

QByteArray DataFileBuilder::readAndCheck(QFile &file, DataFileTypes expectedType, bool *legacy)
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

        if(header->data_type != expectedType)
            throw QObject::tr("This file is not of expected content type");

        compressed = (header->flags & DATAFLAG_COMPRESSED);
    }

    if(legacy)
        *legacy = header.isNull();

    QByteArray data = file.read(file.size());

    if(header && QByteArray::fromRawData(header->md5, sizeof(header->md5)) != MD5(data))
    {
        QMessageBox box(QMessageBox::Question, QObject::tr("Error"),
                        QObject::tr("Corrupted data file - MD5 checksum does not match"),
                        QMessageBox::Yes | QMessageBox::No);
        box.setInformativeText(QObject::tr("Load anyway?"));

        if(box.exec() == QMessageBox::No)
            throw QObject::tr("Corrupted data file - MD5 checksum does not match");
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

    file.read(header->str, 4);
    file.read((char*)&header->version, sizeof(header->version));
    file.read((char*)&header->flags, sizeof(header->flags));
    file.read((char*)&header->data_type, sizeof(header->data_type));
    file.read(header->md5, sizeof(header->md5));
    file.read((char*)&header->header_size, sizeof(header->header_size));
    file.read((char*)&header->compressed_block, sizeof(header->compressed_block));
    file.read(header->unused, sizeof(header->unused));

    if(header->version >= 2)
        file.seek(header->header_size);
}

void DataFileBuilder::writeHeader(QIODevice &file, DataFileHeader *header)
{
    file.seek(0);

    file.write(header->str, 4);
    file.write((char*)&header->version, sizeof(header->version));
    file.write((char*)&header->flags, sizeof(header->flags));
    file.write((char*)&header->data_type, sizeof(header->data_type));
    file.write(header->md5, sizeof(header->md5));
    file.write((char*)&header->header_size, sizeof(header->header_size));
    file.write((char*)&header->compressed_block, sizeof(header->compressed_block));
    file.write(header->unused, sizeof(header->unused));

    file.seek(header->header_size);
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
