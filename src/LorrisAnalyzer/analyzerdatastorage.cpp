#include <QFileDialog>
#include <QMessageBox>

#include "analyzerdatastorage.h"
#include "analyzerdataarea.h"
#include "devicetabwidget.h"
#include "analyzerdatafile.h"

static const char *ANALYZER_DATA_FORMAT = "v5";
static const char ANALYZER_DATA_MAGIC[] = { 0xFF, 0x80, 0x68 };

AnalyzerDataStorage::AnalyzerDataStorage()
{
    m_packet = NULL;
    m_size = 0;
}

AnalyzerDataStorage::~AnalyzerDataStorage()
{
    Clear();
}

void AnalyzerDataStorage::Clear()
{
    m_size = 0;
    for(quint32 i = 0; i < m_data.size(); ++i)
        delete m_data[i];
    m_data.clear();
}

void AnalyzerDataStorage::addData(analyzer_data *data)
{
    if(!m_packet)
        return;
    m_data.push_back(data);
    ++m_size;
}

void AnalyzerDataStorage::SaveToFile(AnalyzerDataArea *area, DeviceTabWidget *devices)
{
    if(!m_packet)
        return;

    QString filters = QObject::tr("Lorris data file (*.ldta)");
    QString filename = QFileDialog::getSaveFileName(NULL, QObject::tr("Export Data"), "", filters);

    AnalyzerDataFile *file = new AnalyzerDataFile(filename);
    if(!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if(filename != "")
        {
            QMessageBox *box = new QMessageBox();
            box->setWindowTitle(QObject::tr("Error!"));
            box->setText(QObject::tr("Can't create/open file!"));
            box->setIcon(QMessageBox::Critical);
            box->exec();
            delete box;
        }
        delete file;
        return;
    }

    //Magic
    file->write(ANALYZER_DATA_FORMAT);
    file->write(ANALYZER_DATA_MAGIC, 3);

    //Header
    char *itr = (char*)&m_packet->header->length;
    file->write(itr, sizeof(analyzer_header));

    //Packet
    itr = (char*)&m_packet->big_endian;
    file->write(itr, sizeof(bool));

    //header static data
    file->writeBlockIdentifier(BLOCK_STATIC_DATA);
    if(m_packet->static_data)
    {
        file->write((char*)&m_packet->header->static_len, sizeof(m_packet->header->static_len));
        file->write((char*)m_packet->static_data, m_packet->header->static_len);
    }
    else
    {
        char i = 0;
        file->write(&i, 1);
    }

    //Devices and commands
    file->writeBlockIdentifier(BLOCK_DEVICE_TABS);
    devices->Save(file);

    //Data
    file->writeBlockIdentifier(BLOCK_DATA);

    quint32 packetCount = m_data.size();
    file->write((char*)&packetCount, sizeof(quint32));

    for(quint32 i = 0; i < m_data.size(); ++i)
    {
        quint32 len = m_data[i]->getData().length();
        file->write((char*)&len, sizeof(len));
        file->write(m_data[i]->getData());
    }

    //Widgets
    file->writeBlockIdentifier(BLOCK_WIDGETS);
    area->SaveWidgets(file);

    file->close();
    delete file;
}

analyzer_packet *AnalyzerDataStorage::loadFromFile(QString *name, quint8 load, AnalyzerDataArea *area, DeviceTabWidget *devices)
{
    QString filename;
    if(name)
        filename = *name;
    else
    {
        QString filters = QObject::tr("Lorris data file (*.ldta)");
        filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import Data"), "", filters);
    }

    AnalyzerDataFile *file = new AnalyzerDataFile(filename);
    if(!file->open(QIODevice::ReadOnly))
    {
        if(filename != "")
        {
            QMessageBox *box = new QMessageBox();
            box->setWindowTitle(QObject::tr("Error!"));
            box->setText(QObject::tr("Can't open file!"));
            box->setIcon(QMessageBox::Critical);
            box->exec();
            delete box;
        }
        delete file;
        return NULL;
    }

    //Magic
    char *itr = new char[3];
    file->read(itr, 2);

    if(itr[0] != ANALYZER_DATA_FORMAT[0] || itr[1] != ANALYZER_DATA_FORMAT[1])
    {
        QMessageBox *box = new QMessageBox();
        box->setWindowTitle(QObject::tr("Warning!"));
        box->setText(QObject::tr("You are opening file with old structure format, some things may be messed up!"));
        box->setIcon(QMessageBox::Warning);
        box->exec();
        delete box;
    }
    delete[] itr;

    if(!checkMagic(file))
    {
        QMessageBox *box = new QMessageBox();
        box->setWindowTitle(QObject::tr("Error!"));
        box->setText(QObject::tr("Data file has wrong magic!"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        file->close();
        delete file;
        return NULL;
    }

    Clear();

    analyzer_header *header = new analyzer_header();
    analyzer_packet *packet = new analyzer_packet(header, true, NULL);

    //Header
    itr = (char*)&header->length;
    file->read(itr, sizeof(analyzer_header));

    //Packet
    itr = (char*)&packet->big_endian;
    file->read(itr, sizeof(bool));

    if(load & STORAGE_STRUCTURE)
    {
        if(m_packet)
        {
            delete m_packet->header;
            delete m_packet;
        }
        m_packet = packet;
    }

    //header static data
    if(file->seekToNextBlock(BLOCK_STATIC_DATA, BLOCK_DEVICE_TABS))
    {
        quint8 static_len = 0;
        file->read((char*)&static_len, sizeof(quint8));
        if(static_len)
        {
            m_packet->static_data = new quint8[static_len];
            file->read((char*)m_packet->static_data, static_len);
        }
    }

    //Devices and commands
    devices->setHeader(header);
    if(file->seekToNextBlock(BLOCK_DEVICE_TABS, BLOCK_DATA))
        devices->Load(file, !(load & STORAGE_STRUCTURE));

    //Data
    if(file->seekToNextBlock(BLOCK_DATA, 0))
    {
        quint32 packetCount = 0;
        file->read((char*)&packetCount, sizeof(quint32));

        QByteArray data;
        for(quint32 i = 0; i < packetCount; ++i)
        {
            quint32 len = 0;
            file->read((char*)&len, sizeof(quint32));
            data = file->read(len);

            if(load & STORAGE_DATA)
            {
                analyzer_data *a_data = new analyzer_data(m_packet);
                a_data->setData(data);
                addData(a_data);
            }
        }
    }

    //Widgets
    if(file->seekToNextBlock(BLOCK_WIDGETS, 0))
        area->LoadWidgets(file, !(load & STORAGE_WIDGETS));

    file->close();
    delete file;

    if(!(load & STORAGE_STRUCTURE))
    {
        delete packet->header;
        delete header;
    }

    return m_packet;
}

bool AnalyzerDataStorage::checkMagic(QFile *file)
{
    char *itr = new char[3];
    file->read(itr, 3);
    for(quint8 i = 0; i < 3; ++i)
    {
        if(itr[i] != ANALYZER_DATA_MAGIC[i])
        {
            delete[] itr;
            return false;
        }
    }
    delete[] itr;
    return true;
}
