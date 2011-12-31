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

// analyzer_header & analyzer_packet contain only structure
struct analyzer_header
{
    analyzer_header()
    {
        Reset();
    }

    analyzer_header(analyzer_header *h)
    {
        Copy(h);
    }

    void Reset()
    {
        length = 0;
        data_mask = 0;
        static_len = 1;
        len_fmt = 0;
        packet_length = 0;
        for(quint8 i = 0; i < 4; ++i)
            order[i] = 0;
    }

    void Copy(analyzer_header *h)
    {
        length = h->length;
        data_mask = h->data_mask;
        static_len = h->static_len;
        len_fmt = h->len_fmt;
        packet_length = h->packet_length;
        for(quint8 i = 0; i < 4; ++i)
            order[i] = h->order[i];
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
    quint32 packet_length; // if packet has static size, it is here
    quint8 data_mask; // from enum DataType
    quint8 static_len; // static data length
    quint8 len_fmt;
    quint8 order[4];
};

struct analyzer_packet
{
    analyzer_packet()
    {
        Reset();
    }

    analyzer_packet(analyzer_header *h, bool b_e)
    {
        header = h;
        big_endian = b_e;
    }

    void Reset()
    {
        header = NULL;
        big_endian = true;
    }

    analyzer_header *header;
    bool big_endian;
};

// Real data
class analyzer_data
{
public:
    analyzer_data(analyzer_packet *packet);

    void setData(QByteArray data)
    {
        m_data = data;
    }
    QByteArray getData()
    {
        return m_data;
    }


    bool getDeviceId(quint8& id);
    bool getCmd(quint8& cmd);

    quint8  getUInt8 (quint32 pos);
    qint8   getInt8  (quint32 pos);
    quint16 getUInt16(quint32 pos);
    qint16  getInt16 (quint32 pos);
    quint32 getUInt32(quint32 pos);
    qint32  getInt32 (quint32 pos);
    quint64 getUInt64(quint32 pos);
    qint64  getInt64 (quint32 pos);
    QString getString(quint32 pos);

    template<class T> void getInt(T& val, quint32 pos);

private:
    analyzer_packet *m_packet;
    QByteArray m_data;
};

#endif // PACKET_H
