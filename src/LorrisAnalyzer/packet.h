/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef PACKET_H
#define PACKET_H

#include <QTypeInfo>
#include <QByteArray>
#include <algorithm>

#include "common.h"

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

    analyzer_packet(analyzer_header *h, bool b_e, quint8 *s_d)
    {
        header = h;
        big_endian = b_e;
        static_data = s_d;
    }

    ~analyzer_packet()
    {
        delete[] static_data;
    }

    void Reset()
    {
        static_data = NULL;
        header = NULL;
        big_endian = true;
    }

    analyzer_header *header;
    bool big_endian;
    quint8 *static_data;
};

// Real data
class analyzer_data
{
public:
    analyzer_data(analyzer_packet *packet);

    void setData(QByteArray data)
    {
        m_data = data;
        m_forceValid = true;
    }
    quint32 addData(char *d_itr, char *d_end);

    const QByteArray& getData() { return m_data; }
    const QByteArray& getStaticData() { return m_static_data; }

    bool isValid();

    bool getDeviceId(quint8& id);
    bool getCmd(quint8& cmd);
    bool getLenFromHeader(quint32& len);
    quint32 getLenght(bool *readFromHeader = NULL);
    quint16 getHeaderDataPos(quint8 type);

    quint8   getUInt8  (quint32 pos) { return read<quint8> (pos); }
    qint8    getInt8   (quint32 pos) { return read<qint8>  (pos); }
    quint16  getUInt16 (quint32 pos) { return read<quint16>(pos); }
    qint16   getInt16  (quint32 pos) { return read<qint16> (pos); }
    quint32  getUInt32 (quint32 pos) { return read<quint32>(pos); }
    qint32   getInt32  (quint32 pos) { return read<qint32> (pos); }
    quint64  getUInt64 (quint32 pos) { return read<quint64>(pos); }
    qint64   getInt64  (quint32 pos) { return read<qint64> (pos); }
    float    getFloat  (quint32 pos) { return read<float>  (pos); }
    double   getDouble (quint32 pos) { return read<double> (pos); }

    QString getString(quint32 pos);

    template <typename T> T read(quint32 pos);

private:
    analyzer_packet *m_packet;
    QByteArray m_data;
    QByteArray m_static_data;
    quint32 itr;
    bool m_forceValid;
};

template <typename T>
T analyzer_data::read(quint32 pos)
{
    if(pos+sizeof(T) > (quint32)m_data.length())
        throw "Cannot read beyond data size!";

    T val = *((T const*)&m_data.data()[pos]);
    if(m_packet->big_endian)
        Utils::swapEndian<T>((char*)&val);
    return val;
}



#endif // PACKET_H
