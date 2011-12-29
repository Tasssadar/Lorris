#include <QFileDialog>
#include <QMessageBox>

#include "eeprom.h"
#include "common.h"

EEPROM::EEPROM(QWidget *parent, DeviceInfo *info)
{
    m_deviceInfo = info;
    m_parent = parent;
}

EEPROM::~EEPROM()
{
    HexFile::deleteAllPages(pages);
    delete m_deviceInfo;
}

void EEPROM::Export()
{
    QString filters = QObject::tr("Intel hex file (*.hex);;Data file (*.dta)");
    QString filename = QFileDialog::getSaveFileName(m_parent, QObject::tr("Export EEPROM"), "", filters);

    QFile *file = new QFile(filename);
    if(!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if(filename != "")
        {
            QMessageBox *box = new QMessageBox(m_parent);
            box->setWindowTitle(QObject::tr("Error!"));
            box->setText(QObject::tr("Can't create/open file!"));
            box->setIcon(QMessageBox::Critical);
            box->exec();
            delete box;
        }
        delete file;
        return;
    }

    if(filename.endsWith(".hex", Qt::CaseInsensitive))
    {
        QString line;
        quint8 checksum = 0;
        for(quint16 address = 0; address < m_deviceInfo->eeprom_size; address += 16)
        {
            line = ":10"; // data size (16)

            // Address
            line += Utils::hexToString(address >> 8);
            line += Utils::hexToString(quint8(address));

            line += "00"; // record type, 0 = data

            checksum = 0x10 + (address >> 8) + quint8(address) + 0x00;
            for(quint8 i = 0; i < 16; ++i)
            {
                line += Utils::hexToString(data[address+i]);
                checksum += data[address+i];
            }
            line += Utils::hexToString(256 - checksum);
            line += "\r\n";
            file->write(line.toAscii());
        }
        line = ":00000001FF";
        file->write(line.toAscii());
    }
    else
        file->write(data);
    file->close();
    delete file;
}

bool EEPROM::Import()
{
    QString filters = QObject::tr("Intel hex file (*.hex);;Data file (*.dta)");
    QString filename = QFileDialog::getOpenFileName(m_parent, QObject::tr("Import EEPROM"), "", filters);

    QFile *file = new QFile(filename);
    if(!file->open(QIODevice::ReadOnly))
    {
        QMessageBox *box = new QMessageBox(m_parent);
        box->setWindowTitle(QObject::tr("Error!"));
        box->setText(QObject::tr("Can't open file!"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete file;
        return false;
    }

    Page *curPage = NULL;
    quint16 baseAddress = 0;
    if(filename.endsWith(".hex", Qt::CaseInsensitive))
    {
        quint8 dataSize;
        for(QString line = QString(file->readLine()); line.size(); line = QString(file->readLine()))
        {
            dataSize = line.mid(1, 2).toInt(0, 16);
            baseAddress = (line.mid(3, 2).toInt(0, 16) << 8);
            baseAddress |= line.mid(5, 2).toInt(0, 16);

            for(quint8 i = 0; i < dataSize; i+=2)
            {
                curPage = new Page();
                curPage->address = baseAddress + i;
                for(quint8 y = 0; y < 2; ++y)
                    curPage->data.push_back(line.mid(9+i*2+y*2, 2).toInt(0, 16));
                pages.push_back(curPage);
            }
        }
    }
    else
    {
        data = file->readAll();
        for(quint16 i = 0; i < data.size(); i+=2)
        {
            curPage = new Page();
            curPage->address = i;
            for(quint8 y = 0; y < 2; ++y)
                curPage->data.push_back(data[y+i]);
            pages.push_back(curPage);
        }
        data.clear();
    }
    pageItr = 0;
    file->close();
    delete file;
    return true;
}


