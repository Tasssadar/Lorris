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

EEPROM::EEPROM(QWidget *parent, chip_definition& chip)
{
    m_chip = chip;
    m_parent = parent;
}

EEPROM::~EEPROM()
{

}

void EEPROM::Export()
{
    static const QString filters = QObject::tr("Intel hex file (*.hex);;Data file (*.dta)");
    QString filename = QFileDialog::getSaveFileName(m_parent, QObject::tr("Export EEPROM"), "", filters);

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
    {
        QMessageBox box(m_parent);
        box.setWindowTitle(QObject::tr("Error!"));
        box.setText(error);
        box.setIcon(QMessageBox::Critical);
        box.exec();
    }
}

bool EEPROM::Import()
{
    QString filters = QObject::tr("Intel hex file (*.hex);;Data file (*.dta)");
    QString filename = QFileDialog::getOpenFileName(m_parent, QObject::tr("Import EEPROM"), "", filters);

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
        QMessageBox box(m_parent);
        box.setWindowTitle(QObject::tr("Error!"));
        box.setText(error);
        box.setIcon(QMessageBox::Critical);
        box.exec();
        return false;
    }
    pageItr = 0;
    return true;
}


