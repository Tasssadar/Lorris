/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QScopedPointer>
#include <QCryptographicHash>
#include <QMessageBox>

#include "datafileparser.h"

#define MD5(x) QCryptographicHash::hash(x, QCryptographicHash::Md5)

DataFileHeader::DataFileHeader(quint8 data_type)
{
    str[0] = 'L'; str[1] = 'D'; str[2] = 'T'; str[3] = 'A';
    version = 1;
    flags = 0;
    this->data_type = data_type;
    std::fill(md5, md5 + sizeof(md5), 0);
    std::fill(unused, unused + sizeof(unused), 0xFF);
}

DataFileHeader::DataFileHeader(const DataFileHeader& other)
{
    std::copy(other.str, other.str+sizeof(str), str);
    version = other.version;
    flags = other.flags;
    data_type = other.data_type;
    std::copy(other.md5, other.md5 + sizeof(md5), md5);
    std::copy(other.unused, other.unused + sizeof(unused), unused);
}

DataFileParser::DataFileParser(QByteArray *data, OpenMode openMode, QObject *parent) :
    QBuffer(data, parent)
{
    m_last_block = 0;
    open(openMode);
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

        case BLOCK_TABWIDGET:        strcpy(res, "tabWidget");       break;
        case BLOCK_WORKTAB:          strcpy(res, "tabWidgetTab");    break;
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

    if(compressed)
        data = qUncompress(data);

    return data;
}

QByteArray DataFileBuilder::writeWithHeader(QFile &file, QByteArray data, bool compress, DataFileTypes type)
{
    if(!file.isOpen() && !file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        throw QObject::tr("Cannot open file \"%1\"!").arg(file.fileName());

    DataFileHeader header(type);
    if(compress)
    {
        header.flags |= DATAFLAG_COMPRESSED;
        data = qCompress(data);
    }

    QByteArray md5 = MD5(data);
    std::copy(md5.data(), md5.data()+sizeof(header.md5), header.md5);

    QBuffer buff;
    buff.open(QIODevice::WriteOnly);
    writeHeader(buff, &header);
    buff.write(data);

    file.write(buff.data());

    return MD5(buff.data());
}

void DataFileBuilder::readHeader(QFile &file, DataFileHeader *header)
{
    file.seek(0);

    file.read(header->str, 4);
    file.read((char*)&header->version, sizeof(header->version));
    file.read((char*)&header->flags, sizeof(header->flags));
    file.read((char*)&header->data_type, sizeof(header->data_type));
    file.read(header->md5, sizeof(header->md5));
    file.read(header->unused, sizeof(header->unused));
}

void DataFileBuilder::writeHeader(QIODevice &file, DataFileHeader *header)
{
    file.seek(0);

    file.write(header->str, 4);
    file.write((char*)&header->version, sizeof(header->version));
    file.write((char*)&header->flags, sizeof(header->flags));
    file.write((char*)&header->data_type, sizeof(header->data_type));
    file.write(header->md5, sizeof(header->md5));
    file.write(header->unused, sizeof(header->unused));
}
