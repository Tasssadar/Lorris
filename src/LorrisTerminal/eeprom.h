#ifndef EEPROM_H
#define EEPROM_H

#include <QByteArray>
#include <QWidget>

#include "deviceinfo.h"
#include "hexfile.h"

class EEPROM
{
public:
    EEPROM(QWidget *parent, DeviceInfo *info);
    ~EEPROM();

    void AddData(QByteArray newData)
    {
        data.append(newData);
    }

    quint16 GetEEPROMSize()
    {
        return m_deviceInfo->eeprom_size;
    }

    void Export();
    bool Import();
    Page *getNextPage()
    {
        if(pageItr >= pages.size() || pages[pageItr]->address >= m_deviceInfo->eeprom_size)
            return NULL;
        return pages[pageItr++];
    }

    QString prepareHexByte(quint8 data);

private:
    QByteArray data;
    DeviceInfo *m_deviceInfo;
    QWidget *m_parent;
    quint16 pageItr;
    std::vector<Page*> pages;
};

#endif // EEPROM_H
