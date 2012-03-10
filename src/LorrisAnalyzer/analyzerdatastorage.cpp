/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <QFileDialog>
#include <QMessageBox>

#include "analyzerdatastorage.h"
#include "analyzerdataarea.h"
#include "devicetabwidget.h"
#include "analyzerdatafile.h"
#include "lorrisanalyzer.h"

static const char *ANALYZER_DATA_FORMAT = "v7";
static const char ANALYZER_DATA_MAGIC[] = { 0xFF, 0x80, 0x68 };

AnalyzerDataStorage::AnalyzerDataStorage(LorrisAnalyzer *analyzer)
{
    m_packet = NULL;
    m_size = 0;
    m_analyzer = analyzer;
}

AnalyzerDataStorage::~AnalyzerDataStorage()
{
    Clear();
}

void AnalyzerDataStorage::setPacket(analyzer_packet *packet)
{
    m_packet = packet;

    if(m_data.empty())
        return;

    for(std::vector<analyzer_data*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
        (*itr)->setPacket(packet);
}

void AnalyzerDataStorage::Clear()
{
    m_size = 0;
    for(std::vector<analyzer_data*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
        delete *itr;
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

    QString filters = QObject::tr("Compressed Lorris data file (*.cldta);;Lorris data file (*.ldta)");
    QString filename = QFileDialog::getSaveFileName(NULL, QObject::tr("Export Data"),
                                                    sConfig.get(CFG_STRING_ANALYZER_FOLDER),
                                                    filters);

    if(filename.isEmpty())
        return;

    AnalyzerDataFile *file = new AnalyzerDataFile(filename);
    if(!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        Utils::ThrowException(QObject::tr("Can't create/open file!"));
        delete file;
        return;
    }

    sConfig.set(CFG_STRING_ANALYZER_FOLDER, filename.left(filename.lastIndexOf(QRegExp("[\\/]"))));

    //Magic
    file->write(ANALYZER_DATA_FORMAT);
    file->write(ANALYZER_DATA_MAGIC, 3);

    //Header
    char *itr = (char*)&m_packet->header->length;
    file->write(itr, sizeof(analyzer_header));

    //Packet
    itr = (char*)&m_packet->big_endian;
    file->write(itr, sizeof(bool));

    //collapse status
    file->writeBlockIdentifier(BLOCK_COLLAPSE_STATUS);
    char dta = m_analyzer->isAreaVisible(AREA_TOP);
    file->write(&dta, 1);
    dta = m_analyzer->isAreaVisible(AREA_RIGHT);
    file->write(&dta, 1);

    file->writeBlockIdentifier(BLOCK_COLLAPSE_STATUS2);
    dta = m_analyzer->isAreaVisible(AREA_LEFT);
    file->write(&dta, 1);

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

    // Data index
    file->writeBlockIdentifier(BLOCK_DATA_INDEX);
    quint32 idx = m_analyzer->getCurrentIndex();
    file->write((char*)&idx, sizeof(idx));

    file->close();
    delete file;
}

analyzer_packet *AnalyzerDataStorage::loadFromFile(QString *name, quint8 load, AnalyzerDataArea *area, DeviceTabWidget *devices, quint32 &data_idx)
{
    QString filename;
    if(name)
        filename = *name;
    else
    {
        QString filters = QObject::tr("Lorris data files (*.ldta *.cldta)");
        filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import Data"), "", filters);
    }

    AnalyzerDataFile *file = new AnalyzerDataFile(filename);
    if(!file->open(QIODevice::ReadOnly))
    {
        if(filename != "")
            Utils::ThrowException(QObject::tr("Can't open file!"));
        delete file;
        return NULL;
    }

    //Magic
    char *version = new char[3];
    file->read(version, 2);
    version[2] = 0;

    if(version[0] != ANALYZER_DATA_FORMAT[0] || version[1] != ANALYZER_DATA_FORMAT[1])
    {
        QMessageBox box;
        box.setWindowTitle(QObject::tr("Warning!"));
        box.setText(QObject::tr("You are opening file with old structure format, some things may be messed up!"));
        box.setIcon(QMessageBox::Warning);
        box.exec();
    }

    if(!checkMagic(file))
    {
        Utils::ThrowException(QObject::tr("Data file has wrong magic!"));

        file->close();
        delete file;
        delete[] version;
        return NULL;
    }

    Clear();

    analyzer_header *header = new analyzer_header();
    analyzer_packet *packet = new analyzer_packet(header, true, NULL);

    //Header
    char *itr = NULL;
    if(strcmp(version, "v6") == 0)
    {
        analyzer_header_v1 old_header;
        itr = (char*)&old_header.length;
        file->read(itr, sizeof(analyzer_header_v1));

        old_header.copyToNew(header);
    }
    else
    {
        itr = (char*)&header->length;
        file->read(itr, sizeof(analyzer_header));
    }

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

    //collapse status
    if(file->seekToNextBlock(BLOCK_COLLAPSE_STATUS, BLOCK_STATIC_DATA))
    {
        bool status;

        file->read((char*)&status, 1);

        // FIXME: hack, dunno what else to do about this
        if(m_analyzer->isAreaVisible(AREA_TOP) != status)
            area->skipNextMove();

        m_analyzer->setAreaVisibility(AREA_TOP, status);


        file->read((char*)&status, 1);
        m_analyzer->setAreaVisibility(AREA_RIGHT, status);
    }
    if(file->seekToNextBlock(BLOCK_COLLAPSE_STATUS2, BLOCK_STATIC_DATA))
    {
        bool status;

        file->read((char*)&status, 1);

        // FIXME: hack, dunno what else to do about this
        if(m_analyzer->isAreaVisible(AREA_LEFT) != status)
            area->skipNextMove();

        m_analyzer->setAreaVisibility(AREA_LEFT, status);
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

    // Data index
    if((load & STORAGE_DATA) && file->seekToNextBlock(BLOCK_DATA_INDEX, 0))
        file->read((char*)&data_idx, sizeof(data_idx));

    file->close();
    delete file;

    if(!(load & STORAGE_STRUCTURE))
    {
        delete packet->header;
        delete packet;
    }

    delete[] version;
    return m_packet;
}

bool AnalyzerDataStorage::checkMagic(AnalyzerDataFile *file)
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
