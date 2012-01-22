#ifndef HEXFILE_H
#define HEXFILE_H

#include <QTypeInfo>
#include <map>
#include <vector>

class QFile;

class HexFile
{
public:
    typedef std::map<quint32, std::vector<quint8> > regionMap;

    HexFile();

    void clear()
    {
        m_data.clear();
    }

    void LoadFromFile(const QString& path);
    void SaveToFile(const QString& path);

    void addRegion(quint32 pos, quint8 const * first, quint8 const * last, int lineno);

    regionMap& getData() { return m_data; }
    void setData(const QByteArray& data);
    QByteArray getDataArray(quint32 len);

    std::vector<quint8>& operator[](quint32 i)
    {
        return m_data[i];
    }

private:
    void writeExtAddrLine(QFile *file, quint32 addr);

    regionMap m_data;
};

#endif // HEXFILE_H
