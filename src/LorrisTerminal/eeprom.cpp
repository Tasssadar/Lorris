/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QFileDialog>
#include <QMessageBox>

#include "eeprom.h"
#include "../common.h"

EEPROM::EEPROM()
{
    pageItr = 0;
}

EEPROM::~EEPROM()
{

}

void EEPROM::reset(chip_definition &chip)
{
    data.clear();
    m_chip = chip;
    pageItr = 0;
    pages.clear();
}

void EEPROM::Export()
{
    static const QString filters = QObject::tr("Intel hex file (*.hex);;Data file (*.dta)");
    QString filename = QFileDialog::getSaveFileName(NULL, QObject::tr("Export EEPROM"), "", filters);

    QString error;
    if(filename.endsWith(".hex", Qt::CaseInsensitive))
    {
        try
        {
            HexFile hex;
            hex.setData(data);
            hex.SaveToFile(filename);
        }
        catch(QString ex)
        {
            error = ex;
        }
    }
    else
    {
        QFile file(filename);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate) && filename != "")
            error = QObject::tr("Can't create/open file!");
        else
        {
            file.write(data);
            file.close();
        }
    }

    if(!error.isEmpty())
        Utils::ThrowException(error);
}

bool EEPROM::Import()
{
    QString filters = QObject::tr("Intel hex file (*.hex);;Data file (*.dta)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import EEPROM"), "", filters);

    QString error;
    if(filename.endsWith(".hex", Qt::CaseInsensitive))
    {
        try
        {
            HexFile hex;
            hex.LoadFromFile(filename);
            hex.makePages(pages, MEM_EEPROM, m_chip, NULL);
        }
        catch(QString ex)
        {
            error = ex;
        }
    }
    else
    {
        QFile file(filename);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate) && filename != "")
            error = QObject::tr("Can't open file!");
        else
        {
            data = file.readAll();
            page curPage;
            for(quint16 i = 0; i < data.size(); i+=2)
            {
                curPage.address = i;
                for(quint8 y = 0; y < 2; ++y)
                    curPage.data.push_back(data[y+i]);
                pages.push_back(curPage);
            }
            data.clear();
            file.close();
        }
    }

    if(!error.isEmpty())
    {
        Utils::ThrowException(error);
        return false;
    }
    pageItr = 0;
    return true;
}
