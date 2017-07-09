/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PACKET_H
#define PACKET_H

#include <QTypeInfo>
#include <QByteArray>
#include <algorithm>
#include <vector>

#include "../common.h"

enum DataType
{
    DATA_BODY      = 0x01,
    DATA_HEADER    = 0x02,
    DATA_DEVICE_ID = 0x04,
    DATA_OPCODE    = 0x08,
    DATA_LEN       = 0x10,
    DATA_STATIC    = 0x20,
    DATA_AVAKAR    = 0x40
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
        static_len = 0;
        len_fmt = 0;
        packet_length = 0;
        len_offset = 0;
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
        len_offset = h->len_offset;
        for(quint8 i = 0; i < 4; ++i)
            order[i] = h->order[i];
    }

    void AddOrder(quint8 mask)
    {
        for(quint8 i = 0; i < 4; ++i)
            if(order[i] == mask)
                return;

        quint8 i = 0;
        while(order[i] != 0) { ++i; }
        order[i] = mask;
    }

    void RmOrder(quint8 mask)
    {
        quint8 tmpOrder[4];
        std::copy(order, order + 4, tmpOrder);
        std::fill(order, order + 4, 0);

        for(quint8 i = 0, y = 0; i < 4; ++i)
        {
            if(tmpOrder[i] == mask || tmpOrder[i] == 0)
                continue;
            order[y++] = tmpOrder[i];
        }
    }

    int findDataPos(quint8 dataType)
    {
        int pos_h = 0;
        for(int i = 0; i < 4; ++i)
        {
            if(order[i] == dataType)
                return pos_h;

            switch(order[i])
            {
                case DATA_STATIC:
                    pos_h += static_len;
                    break;
                case DATA_LEN:
                    pos_h += (1 << len_fmt);
                    break;
                case DATA_DEVICE_ID:
                case DATA_OPCODE:
                case DATA_AVAKAR:
                    ++pos_h;
                    break;
            }
        }
        return -1;
    }

    bool hasLen() const
    {
        return (data_mask & (DATA_AVAKAR | DATA_LEN));
    }

    bool hasOpcode() const
    {
        return (data_mask & (DATA_AVAKAR | DATA_OPCODE));
    }

    quint32 length;
    quint32 packet_length; // if packet has static size, it is here
    quint8 data_mask; // from enum DataType
    quint8 static_len; // static data length
    quint8 len_fmt;
    quint8 order[4];
    qint8 len_offset;
};


// for old version header, used in data file load
struct analyzer_header_v1
{
    void copyToNew(analyzer_header *h)
    {
        h->length = length;
        h->packet_length = packet_length;
        h->data_mask = data_mask;
        h->static_len = static_len;
        h->len_fmt = len_fmt;
        h->len_offset = 0;

        for(quint8 i = 0; i < 4; ++i)
            h->order[i] = order[i];
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

    analyzer_packet(analyzer_packet *p)
    {
        copy(p);
    }

    void copy(analyzer_packet *p)
    {
        header = new analyzer_header(p->header);
        big_endian = p->big_endian;
        static_data.assign(p->static_data.begin(), p->static_data.end());
    }

    void Reset()
    {
        static_data.clear();
        header = NULL;
        big_endian = true;
    }

    QByteArray getStaticData()
    {
        if(header)
            return QByteArray::fromRawData((char*)static_data.data(), header->static_len);
        return QByteArray();
    }

    quint32 getStaticDataOffset() {
        if(header && header->static_len != 0) {
            return header->findDataPos(DATA_STATIC);
        }
        return 0;
    }

    analyzer_header *header;
    bool big_endian;
    std::vector<quint8> static_data;
};

// Real data
class analyzer_data
{
public:
    analyzer_data(QByteArray *data = NULL, analyzer_packet *packet = NULL);
    void clear();
    void copy(analyzer_data *other);

    void setPacket(analyzer_packet *packet) { m_packet = packet; }
    analyzer_packet *getPacket() const { return m_packet; }

    quint32 addData(char *d_itr, char *d_end, quint32& itr);
    const QByteArray& getData() { return *m_data; }
    QByteArray *getDataPtr() { return m_data; }
    bool hasData() const { return m_data != NULL; }
    void setData(QByteArray *data)
    {
        m_data = data;
    }

    bool isValid(quint32 itr);

    bool getDeviceId(quint8& id);
    bool getCmd(quint8& cmd);
    bool getLenFromHeader(quint32& len);
    quint32 getLenght(bool *readFromHeader = NULL);

    quint8   getUInt8  (quint32 pos) const { return read<quint8>(pos); }
    qint8    getInt8   (quint32 pos) const { return read<qint8>(pos); }
    quint16  getUInt16 (quint32 pos) const { return read<quint16>(pos); }
    qint16   getInt16  (quint32 pos) const { return read<qint16> (pos); }
    quint32  getUInt32 (quint32 pos) const { return read<quint32>(pos); }
    qint32   getInt32  (quint32 pos) const { return read<qint32> (pos); }
    quint64  getUInt64 (quint32 pos) const { return read<quint64>(pos); }
    qint64   getInt64  (quint32 pos) const { return read<qint64> (pos); }
    float    getFloat  (quint32 pos) const { return read<float>  (pos); }
    double   getDouble (quint32 pos) const { return read<double> (pos); }

    QString getString(quint32 pos);

    template <typename T> T read(quint32 pos) const;

private:
    analyzer_packet *m_packet;
    QByteArray *m_data;
};

template <typename T>
T analyzer_data::read(quint32 pos) const
{
    T val = 0;
    if(int(pos + sizeof(T)) > m_data->size())
        return val;

    val = *((T const*)&m_data->data()[pos]);
    if(sizeof(T) > 1 && m_packet->big_endian)
        Utils::swapEndian(val);
    return val;
}

#endif // PACKET_H
