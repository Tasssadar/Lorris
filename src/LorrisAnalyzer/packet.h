#ifndef PACKET_H
#define PACKET_H

#include <QTypeInfo>
#include <QByteArray>

enum DataType
{
    DATA_BODY      = 0x01,
    DATA_HEADER    = 0x02,
    DATA_DEVICE_ID = 0x04,
    DATA_OPCODE    = 0x08,
    DATA_LEN       = 0x10,
    DATA_STATIC    = 0x20
};

struct analyzer_header
{
    analyzer_header()
    {
        Reset();
    }

    void Reset()
    {
        length = 0;
        data_mask = 0;
        static_len = 1;
        len_fmt = 0;
        for(quint8 i = 0; i < 4; ++i)
            order[i] = 0;
    }

    void AddOrder(quint8 mask)
    {
        quint8 i = 0;
        while(order[i] != 0) { ++i; }
        order[i] = mask;
    }

    void RmOrder(quint8 mask)
    {
        quint8 i = 0;
        while(order[i] != mask) { ++i; }
        order[i] = 0;
    }

    quint32 length;
    quint8 data_mask; // from enum DataType
    quint8 static_len;
    quint8 len_fmt;
    quint8 order[4];
};


class analyzer_packet
{
public:
    analyzer_packet();

    void Reset();

    quint8  getUInt8 (quint32 pos);
    qint8   getInt8  (quint32 pos);
    quint16 getUInt16(quint32 pos);
    qint16  getInt16 (quint32 pos);
    quint32 getUInt32(quint32 pos);
    qint32  getInt32 (quint32 pos);
    quint64 getUInt64(quint32 pos);
    qint64  getInt64 (quint32 pos);
    QString getString(quint32 pos);

private:
    analyzer_header header;
    QByteArray data;
    bool big_endian;
};

#endif // PACKET_H
