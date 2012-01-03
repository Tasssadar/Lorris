#include <QFileDialog>
#include <QMessageBox>
#include "analyzerdatastorage.h"

static const char *ANALYZER_DATA_FORMAT = "v1";
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
    for(quint32 i = 0; i < m_data.size(); ++i)
        delete m_data[i];
    m_data.clear();
}

analyzer_data *AnalyzerDataStorage::addData(QByteArray data)
{
    if(!m_packet)
        return NULL;
    analyzer_data *a_data = new analyzer_data(m_packet);
    a_data->setData(data);
    m_data.push_back(a_data);
    ++m_size;
    return a_data;
}

void AnalyzerDataStorage::SaveToFile()
{
    if(!m_packet)
        return;

    QString filters = QObject::tr("Lorris data file (*.ldta)");
    QString filename = QFileDialog::getSaveFileName(NULL, QObject::tr("Export Data"), "", filters);

    QFile *file = new QFile(filename);
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

    quint32 packetCount = m_data.size();
    file->write((char*)&packetCount, sizeof(quint32));

    //Data
    for(quint32 i = 0; i < m_data.size(); ++i)
    {
        quint32 len = m_data[i]->getData().length();
        file->write((char*)&len, sizeof(len));
        file->write(m_data[i]->getData());
    }

    file->close();
    delete file;
}

analyzer_packet *AnalyzerDataStorage::loadFromFile()
{
    QString filters = QObject::tr("Lorris data file (*.ldta)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import Data"), "", filters);

    QFile *file = new QFile(filename);
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
        box->setWindowTitle(QObject::tr("Error!"));
        box->setText(QObject::tr("Data file has different version of structure!"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        file->close();
        delete file;
        return NULL;
    }

    file->read(itr, 3);
    for(quint8 i = 0; i < 3; ++i)
    {
        if(itr[i] != ANALYZER_DATA_MAGIC[i])
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
    }
    delete[] itr;

    if(m_packet)
    {
        delete m_packet->header;
        delete m_packet;
    }

    Clear();

    analyzer_header *header = new analyzer_header();
    m_packet = new analyzer_packet(header, true);

    //Header
    itr = (char*)&header->length;
    file->read(itr, sizeof(analyzer_header));

    //Packet
    itr = (char*)&m_packet->big_endian;
    file->read(itr, sizeof(bool));

    //Data
    quint32 packetCount = 0;
    file->read((char*)&packetCount, sizeof(quint32));

    for(quint32 i = 0; i < packetCount; ++i)
    {
        quint32 len = 0;
        file->read((char*)&len, sizeof(quint32));
        addData(file->read(len));
    }

    file->close();
    delete file;

    return m_packet;
}