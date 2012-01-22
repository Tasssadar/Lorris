#include <QFile>
#include <QObject>

#include "hexfile.h"
#include "common.h"

// Most of this file is ported from avr232client, file program.hpp

HexFile::HexFile()
{
}

void HexFile::LoadFromFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        throw QString(QObject::tr("Can't open file \"%1\"!")).arg(path);

    clear();

    int base = 0;
    std::vector<quint8> rec_nums;
    bool ok;

    QByteArray line = file.readLine();
    for(int lineno = 0; !line.isNull(); line = file.readLine(), ++lineno)
    {
        line = line.trimmed();
        if(line.isEmpty())
            continue;

        if(line[0] != ':' || line.size()%2 != 1)
            throw QString(QObject::tr("Invalid line format (line %1)")).arg(lineno);

        rec_nums.clear();
        quint8 checksum = 0;
        for(quint8 i = 1; true;)
        {
            quint8 num = line.mid(i, 2).toInt(&ok, 16);
            if(!ok)
                throw QString(QObject::tr("Failed to parse hex num (line %1)")).arg(lineno);

            rec_nums.push_back(num);

            i += 2;

            if(i < line.length())
                checksum += num;
            else
                break;
        }
        checksum = 256 - checksum;

        if(checksum != rec_nums[rec_nums.size()-1])
            throw QString(QObject::tr("Checksums does not match (line %1)")).arg(lineno);

        int length = rec_nums[0];
        int address = rec_nums[1] * 0x100 + rec_nums[2];
        int rectype = rec_nums[3];

        if (length != (int)rec_nums.size() - 5)
            throw QString(QObject::tr("Invalid record lenght specified (line %1)")).arg(lineno);

        if (rectype == 4)
        {
            if (length != 2)
                throw QString(QObject::tr("Invalid type 4 record (line %1)")).arg(lineno);
            base = (rec_nums[4] * 0x100 + rec_nums[5]) << 16;
            continue;
        }

        if (rectype == 3)
            continue;

        if (rectype == 2)
        {
            if (length != 2)
                throw QString(QObject::tr("Invalid type 2 record (line %1)")).arg(lineno);
            base = (rec_nums[4] * 0x100 + rec_nums[5]) * 16;
            continue;
        }

        if (rectype == 1)
            break;

        if (rectype != 0)
            throw QString(QObject::tr("Invalid record type (line %1)")).arg(lineno);

        addRegion(base + address, rec_nums.data() + 4, rec_nums.data() + rec_nums.size() - 1, lineno);
    }
}

//void add_region(std::size_t pos, byte_type const * first, byte_type const * last, int lineno)
//program.hpp
void HexFile::addRegion(quint32 pos, quint8 const * first, quint8 const * last, int lineno)
{
    regionMap::iterator itr = m_data.upper_bound(pos);
    if(itr != m_data.begin())
    {
        regionMap::iterator itr2 = itr;
        --itr2;

        if(itr2->first + itr2->second.size() == pos)
        {
            itr2->second.insert(itr2->second.end(), first, last);
            return;
        }

        if(itr2->first + itr2->second.size() > pos)
            throw QString(QObject::tr("Memory location was defined twice (line %1)")).arg(lineno);
    }

    if(itr != m_data.end() && itr->first < pos + (last - first))
        throw QString(QObject::tr("Memory location was defined twice (line %1)")).arg(lineno);

    m_data[pos] = std::vector<quint8>(first, last);
}

void HexFile::SaveToFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))
        throw QString(QObject::tr("Can't open file \"%1\"!")).arg(path);

    quint32 base = 0;
    for(regionMap::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
    {
        quint32 offset = itr->first;
        quint32 address = offset;
        std::vector<quint8>& data = itr->second;

        if((base & 0xFFFF0000) != (offset & 0xFFFF0000))
        {
            writeExtAddrLine(&file, offset);
            base = offset;
        }

        quint8 write = 0;
        for(quint32 i = 0; i != data.size(); i += write)
        {
            QString line(":");
            write = (data.size() - i >= 0x10) ? 0x10 : data.size() - i;

            line += Utils::hexToString(write);       // recordn len
            line += Utils::hexToString(address >> 8); // address
            line += Utils::hexToString(address);
            line += "00";                            // record type

            quint8 checksum = write + (quint8)(address >> 8) + (quint8)address;
            for(quint8 x = 0; x < write; ++x)
            {
                line += Utils::hexToString(data[i+x]);
                checksum += data[i+x];
            }
            line += Utils::hexToString(0x100 - checksum);
            line += "\r\n";

            address += write;

            file.write(line.toAscii());
        }
    }

    static const QString endFile = ":00000001FF\r\n";
    file.write(endFile.toAscii());
    file.close();
}

void HexFile::writeExtAddrLine(QFile *file, quint32 addr)
{
    QString line = ":02" "00" "00" "04";
    line += Utils::hexToString(addr >> 24);
    line += Utils::hexToString(addr >> 16);

    quint8 checksum = 0x100 - (quint8)(0x02 + 0x04 + (quint8)(addr >> 24) + (quint8)(addr >> 16));
    line += Utils::hexToString(checksum);

    line += "\r\n";
    file->write(line.toAscii());
}

void HexFile::setData(const QByteArray &data)
{
    clear();

    quint32 base = 0;
    quint32 size = data.size();
    quint32 itr = 0;

    std::vector<quint8> bytes;

    do
    {
        bytes.clear();

        for(;itr < size && itr < 0xFFFF; ++itr)
            bytes.push_back(data[itr]);

        m_data[base] = bytes;
        base = itr;
    }while((base & 0xFFFF0000) != (size & 0xFFFF0000));
}

QByteArray HexFile::getDataArray(quint32 len)
{
    QByteArray res(len, 0xFF);
    for(regionMap::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
    {
        quint32 offset = itr->first;
        std::vector<quint8>& data = itr->second;

        for(quint32 i = 0; i < data.size(); ++i)
        {
            //FIXME: is it right?
            //if(offset + i > res.size())
            //    return res;
            res[offset+i] = data[i];
        }
    }
    return res;
}
