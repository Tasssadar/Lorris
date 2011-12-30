#include "packet.h"
#include "common.h"

analyzer_packet::analyzer_packet()
{
    Reset();
}

void analyzer_packet::Reset()
{
    data.clear();
    big_endian = true;
}

quint8 analyzer_packet::getUInt8(quint32 pos)
{
    if(pos >= data.length())
        return 0;
    return (quint8)data[pos];
}

qint8 analyzer_packet::getInt8(quint32 pos)
{
    if(pos >= data.length())
        return 0;
    return (qint8)data[pos];
}

quint16 analyzer_packet::getUInt16(quint32 pos)
{
    if(pos+2 > data.length())
        return 0;
    quint16 val = ((quint8)data[pos] << 8) | (quint8)data[pos+1];
    if(!big_endian)
        val = Utils::swapEndian16(val);
    return val;
}

qint16 analyzer_packet::getInt16(quint32 pos)
{
    if(pos+2 > data.length())
        return 0;
    qint16 val = ((quint8)data[pos] << 8) | (quint8)data[pos+1];
    if(!big_endian)
        val = Utils::swapEndian16(val);
    return val;
}

quint32 analyzer_packet::getUInt32(quint32 pos)
{
    if(pos+4 > data.length())
        return 0;
    quint32 val = ((quint8)data[pos++] << 24) | ((quint8)data[pos++] << 16) |
                  ((quint8)data[pos++] << 8)  | (quint8)data[pos];
    if(!big_endian)
        val = Utils::swapEndian32(val);
    return val;
}

qint32 analyzer_packet::getInt32(quint32 pos)
{
    if(pos+4 > data.length())
        return 0;
    qint32 val = ((quint8)data[pos++] << 24) | ((quint8)data[pos++] << 16) |
                  ((quint8)data[pos++] << 8)  | (quint8)data[pos];
    if(!big_endian)
        val = Utils::swapEndian32(val);
    return val;
}

quint64 analyzer_packet::getUInt64(quint32 pos)
{
    if(pos+8 > data.length())
        return 0;
    quint64 val = ((quint8)data[pos++] << 56) | ((quint8)data[pos++] << 48) |
                  ((quint8)data[pos++] << 40) | ((quint8)data[pos++] << 32) |
                  ((quint8)data[pos++] << 24) | ((quint8)data[pos++] << 16) |
                  ((quint8)data[pos++] << 8)  | (quint8)data[pos];
    if(!big_endian)
        val = Utils::swapEndian64(val);
    return val;
}

qint64 analyzer_packet::getInt64(quint32 pos)
{
    if(pos+8 > data.length())
        return 0;
    qint64 val = ((quint8)data[pos++] << 56) | ((quint8)data[pos++] << 48) |
                 ((quint8)data[pos++] << 40) | ((quint8)data[pos++] << 32) |
                 ((quint8)data[pos++] << 24) | ((quint8)data[pos++] << 16) |
                 ((quint8)data[pos++] << 8)  | (quint8)data[pos];
    if(!big_endian)
        val = Utils::swapEndian64(val);
    return val;
}

QString analyzer_packet::getString(quint32 pos)
{
    QString str = "";
    if(pos >= data.length())
        return str;
    for(; pos < data.length() && QChar(data[pos]) != 0; ++pos)
        str.append(QChar(data[pos]));
    return str;
}
