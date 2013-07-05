/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QFileDialog>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QApplication>

#include "storage.h"
#include "widgetarea.h"
#include "../misc/datafileparser.h"
#include "lorrisanalyzer.h"
#include "filtertabwidget.h"

static const char *ANALYZER_DATA_FORMAT = "v7";
static const char ANALYZER_DATA_MAGIC[] = { (char)0xFF, (char)0x80, 0x68 };

#define MD5(x) QCryptographicHash::hash(x, QCryptographicHash::Md5)

Storage::Storage(LorrisAnalyzer *analyzer)
{
    m_packet = NULL;
    m_analyzer = analyzer;
}

Storage::~Storage()
{
    Clear();
}

void Storage::setPacket(analyzer_packet *packet)
{
    m_packet = packet;

    if(m_data.empty())
        return;
}

void Storage::Clear()
{
    m_data.clear();
}

QByteArray *Storage::addData(const QByteArray& data)
{
    if(!m_packet)
        return NULL;
    return m_data.push_back(data);
}

void Storage::SaveToFile(WidgetArea *area, FilterTabWidget *filters)
{
    if(m_filename.isEmpty())
        return SaveToFile(m_filename, area, filters);

    // Check md5
    QFile file(m_filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        Utils::showErrorBox(QObject::tr("Can't create/open file!"));
        return;
    }

    QByteArray md5 = MD5(file.readAll());
    file.close();

    if(md5 != m_file_md5)
    {
        QMessageBox box(area);
        box.setWindowTitle(tr("File has changed"));
        box.setText(tr("The file has been changed since last save."));
        box.setInformativeText(tr("Ignore and save anyway?"));
        box.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        box.setDefaultButton(QMessageBox::Save);
        box.setIcon(QMessageBox::Question);

        if(box.exec() == QMessageBox::Cancel)
            return;
    }
    SaveToFile(m_filename, area, filters);
}

void Storage::SaveToFile(QString filename, WidgetArea *area, FilterTabWidget *filters)
{
    analyzer_packet *packet = m_packet;
    if(!m_packet)
        packet = new analyzer_packet(new analyzer_header, true);

    if(filename.isEmpty())
    {
        static const QString filters = QObject::tr("Compressed Lorris data file (*.cldta);;Lorris data file (*.ldta)");
        filename = QFileDialog::getSaveFileName(NULL, QObject::tr("Export Data"),
                                                    sConfig.get(CFG_STRING_ANALYZER_FOLDER),
                                                    filters);
        if(filename.isEmpty())
            return;
    }

    m_filename = filename;

    sConfig.set(CFG_STRING_ANALYZER_FOLDER, filename);

    QByteArray data;
    {
        DataFileParser buffer(&data, QIODevice::WriteOnly);

        //Header
        buffer.writeBlockIdentifier("analyzerHeaderV2");
        buffer.write((char*)&packet->header->length, sizeof(analyzer_header));

        //Packet
        buffer.writeBlockIdentifier("analyzerPacket");
        buffer.write((char*)&packet->big_endian, sizeof(bool));

        //collapse status
        buffer.writeBlockIdentifier(BLOCK_COLLAPSE_STATUS);
        buffer.writeVal(m_analyzer->isAreaVisible(AREA_TOP));
        buffer.writeVal(m_analyzer->isAreaVisible(AREA_RIGHT));

        buffer.writeBlockIdentifier(BLOCK_COLLAPSE_STATUS2);
        buffer.writeVal(m_analyzer->isAreaVisible(AREA_LEFT));

        //header static data
        buffer.writeBlockIdentifier(BLOCK_STATIC_DATA);
        buffer.write((char*)&packet->header->static_len, sizeof(packet->header->static_len));
        buffer.write((char*)packet->static_data.data(), packet->header->static_len);

        //Filters
        filters->Save(&buffer);

        //Data
        buffer.writeBlockIdentifier(BLOCK_DATA);

        quint32 packetCount = m_data.size();
        buffer.write((char*)&packetCount, sizeof(quint32));

        for(quint32 i = 0; i < m_data.size(); ++i)
        {
            const QByteArray& d = m_data[i];
            buffer << (quint32)d.size();
            buffer.write(d);
        }

        //Widgets
        buffer.writeBlockIdentifier(BLOCK_WIDGETS);
        area->saveWidgets(&buffer);

        // Area settings
        area->saveSettings(&buffer);

        // Data index
        buffer.writeBlockIdentifier(BLOCK_DATA_INDEX);
        buffer << (quint32)m_analyzer->getCurrentIndex();

        // Packet limits
        buffer.writeBlockIdentifier(BLOCK_PACKET_LIMIT);
        buffer << m_data.getPacketLimit();

        buffer.close();
    }

    try {
        m_file_md5 = DataFileBuilder::writeWithHeader(filename, data, filename.contains(".cldta"), DATAFILE_ANALYZER);
    } catch(const QString& ex) {
        Utils::showErrorBox(ex);
    }

    if(!m_packet)
    {
        delete packet->header;
        delete packet;
    }
}

analyzer_packet *Storage::loadFromFile(QString *name, quint8 load, WidgetArea *area, FilterTabWidget *filters, quint32 &data_idx)
{
    QString filename;
    if(name)
        filename = *name;
    else
    {
        QString filters = QObject::tr("Lorris data files (*.ldta *.cldta)");
        filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import Data"), "", filters);
    }

    QByteArray data;
    bool legacy = false;

    QScopedPointer<QMessageBox> loading_box;
    {
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly))
        {
            if(filename != "")
                Utils::showErrorBox(QObject::tr("Can't open file!"));
            return NULL;
        }

        loading_box.reset(new QMessageBox());
        loading_box->setText(tr("Loading data file..."));
        loading_box->setStandardButtons(QMessageBox::NoButton);
        loading_box->setWindowModality(Qt::ApplicationModal);
        loading_box->setIcon(QMessageBox::Information);
        loading_box->open();

        // Proccess events to properly draw message box
        // must be done twice, first one draws dialog
        // and second draws content
        QApplication::processEvents();
        QApplication::processEvents();

        try {
            data = DataFileBuilder::readAndCheck(file, DATAFILE_ANALYZER, &legacy);
        }
        catch(const QString& ex)
        {
            delete loading_box.take();
            Utils::showErrorBox(tr("Error while loading data file: %1").arg(ex));
            return NULL;
        }

        file.seek(0);
        m_file_md5 = MD5(file.readAll());

        file.close();
        QFileInfo info(filename);
        m_filename = info.absoluteFilePath();
    }

    DataFileParser buffer(&data, QIODevice::ReadOnly);

    QScopedPointer<analyzer_header> header(new analyzer_header());
    QScopedPointer<analyzer_packet> packet(new analyzer_packet(header.data(), true));

    if(legacy)
    {
        try {
            readLegacyStructure(&buffer, packet.data());
        }
        catch(const QString& ex)
        {
            delete loading_box.take();
            Utils::showErrorBox(ex);
            return NULL;
        }
    }

    Clear();

    if(load & STORAGE_STRUCTURE)
    {
        if(m_packet)
        {
            delete m_packet->header;
            delete m_packet;
        }
        m_packet = packet.data();
    }

    // Header
    if(buffer.seekToNextBlock("analyzerHeaderV2", BLOCK_COLLAPSE_STATUS))
    {
        char *itr = (char*)&header->length;
        buffer.read(itr, sizeof(analyzer_header));
    }

    //Packet
    if(buffer.seekToNextBlock("analyzerPacket", BLOCK_COLLAPSE_STATUS))
    {
        char *itr = (char*)&packet->big_endian;
        buffer.read(itr, sizeof(bool));
    }

    //collapse status
    if(buffer.seekToNextBlock(BLOCK_COLLAPSE_STATUS, BLOCK_STATIC_DATA))
    {
        bool status;

        buffer.read((char*)&status, 1);

        // FIXME: hack, dunno what else to do about this
        if(m_analyzer->isAreaVisible(AREA_TOP) != status)
            area->skipNextMove();

        m_analyzer->setAreaVisibility(AREA_TOP, status);

        buffer.read((char*)&status, 1);
        m_analyzer->setAreaVisibility(AREA_RIGHT, status);
    }
    if(buffer.seekToNextBlock(BLOCK_COLLAPSE_STATUS2, BLOCK_STATIC_DATA))
    {
        bool status;
        buffer.read((char*)&status, 1);

        // FIXME: hack, dunno what else to do about this
        if(m_analyzer->isAreaVisible(AREA_LEFT) != status)
            area->skipNextMove();

        m_analyzer->setAreaVisibility(AREA_LEFT, status);
    }


    //header static data
    if(buffer.seekToNextBlock(BLOCK_STATIC_DATA, BLOCK_DEVICE_TABS))
    {
        quint8 static_len = 0;
        buffer.read((char*)&static_len, sizeof(quint8));

        // FIXME: header data and this lenght must be same,
        // corrupted file?
        m_packet->header->static_len = static_len;

        if(static_len)
        {
            m_packet->static_data.resize(static_len);
            buffer.read((char*)m_packet->static_data.data(), static_len);
        }
    }

    //Devices and commands
    filters->setHeader(header.data());
    filters->Load(&buffer, !(load & STORAGE_STRUCTURE));

    //Data
    if(buffer.seekToNextBlock(BLOCK_DATA, 0))
    {
        quint32 packetCount = 0;
        buffer.read((char*)&packetCount, sizeof(quint32));

        QByteArray data;
        for(quint32 i = 0; i < packetCount; ++i)
        {
            quint32 len = 0;
            buffer.read((char*)&len, sizeof(quint32));
            data = buffer.read(len);

            if(load & STORAGE_DATA)
                addData(data);
        }
    }

    //Widgets
    if(buffer.seekToNextBlock(BLOCK_WIDGETS, 0))
        area->loadWidgets(&buffer, !(load & STORAGE_WIDGETS));

    // Area settings
    area->loadSettings(&buffer);

    // Data index
    if((load & STORAGE_DATA) && buffer.seekToNextBlock(BLOCK_DATA_INDEX, 0))
        buffer.read((char*)&data_idx, sizeof(data_idx));

    // packet limit
    if(buffer.seekToNextBlock(BLOCK_PACKET_LIMIT, 0))
        m_data.setPacketLimit(buffer.readVal<quint32>());

    buffer.close();

    // To process delayed load events
    QApplication::processEvents();

    if(load & STORAGE_STRUCTURE)
    {
        packet.take();
        header.take();
    }

    return m_packet;
}

bool Storage::checkMagic(DataFileParser *file)
{
    char magic[3];
    file->read(magic, 3);

    for(quint8 i = 0; i < 3; ++i)
        if(magic[i] != ANALYZER_DATA_MAGIC[i])
            return false;

    return true;
}

void Storage::ExportToBin(const QString &filename)
{
    QFile f(filename);
    if(!f.open(QIODevice::Truncate | QIODevice::WriteOnly))
        throw tr("Unable to open file %1 for writing!").arg(filename);

    for(quint32 i = 0; i < m_data.size(); ++i)
        f.write(m_data[i]);

    f.close();
}

void Storage::readLegacyStructure(DataFileParser *file, analyzer_packet *packet)
{
    char version[3] = { 0 };
    file->read(version, 2);

    if(strcmp(version, ANALYZER_DATA_FORMAT) != 0)
    {
        QMessageBox box;
        box.setWindowTitle(QObject::tr("Warning!"));
        box.setText(QObject::tr("You are opening file with old structure format, some things may be messed up!"));
        box.setIcon(QMessageBox::Warning);
        box.exec();
    }

    if(!checkMagic(file))
        throw QObject::tr("Data file has wrong magic!");

    //Header
    char *itr = NULL;
    if(strcmp(version, "v6") == 0)
    {
        analyzer_header_v1 old_header;
        itr = (char*)&old_header.length;
        file->read(itr, sizeof(analyzer_header_v1));

        old_header.copyToNew(packet->header);
    }
    else
    {
        itr = (char*)&packet->header->length;
        file->read(itr, sizeof(analyzer_header));
    }

    //Packet
    itr = (char*)&packet->big_endian;
    file->read(itr, sizeof(bool));
}
