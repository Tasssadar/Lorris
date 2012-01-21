#ifndef CHIPDEFS_H
#define CHIPDEFS_H

#include <QString>
#include <map>
#include <vector>

class chip_definition
{
public:

    static void parse_chipdefs(QString const & strdefs, std::vector<chip_definition> & res);
    static void update_chipdef(std::vector<chip_definition> &templates, chip_definition & cd);
    static void parse_default_chipsets(std::vector<chip_definition> & res);

    struct memorydef
    {
        quint32 size;
        quint16 pagesize;
        quint8 memid;
    };

    struct fuse
    {
        QString name;
        std::vector<int> bits;
        std::vector<int> values;
    };

    chip_definition();

    const QString& getName() { return m_name; }
    const QString& getSign() { return m_signature; }
    void setName(const QString& name) { m_name = name; }
    void setSign(const QString& sign) { m_signature = sign; }

    std::map<QString, memorydef> &getMems() { return m_memories; }
    std::vector<fuse> &getFuses() { return m_fuses; }
    std::map<QString, QString> &getOptions() { return m_options; }

    memorydef *getMemDef(const QString& name)
    {
        std::map<QString, memorydef>::iterator itr = m_memories.find(name);
        if(itr != m_memories.end())
            return &itr->second;
        return NULL;
    }

private:
    QString m_name;
    QString m_signature;

    std::map<QString, memorydef> m_memories;
    std::map<QString, QString> m_options;

    std::vector<fuse> m_fuses;
};

#endif // CHIPDEFS_H
