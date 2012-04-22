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

#include <QScrollArea>
#include <QList>
#include <QAction>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>
#include <QFile>

#include "devicetabwidget.h"
#include "labellayout.h"
#include "../common.h"
#include "DataWidgets/datawidget.h"
#include "lorrisanalyzer.h"
#include "analyzerdatafile.h"

DeviceTabWidget::DeviceTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    m_id_enabled = false;

    QAction *new_device_act = new QAction(tr("Add device"), this);
    connect(new_device_act, SIGNAL(triggered()), this, SLOT(newDevice()));
    addAction(new_device_act);

    m_add_all_act = new QAction(tr("Add \"All devices\" tab"), this);
    m_add_all_act->setEnabled(false);
    connect(m_add_all_act, SIGNAL(triggered()), this, SLOT(addAllDevices()));
    addAction(m_add_all_act);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClose(int)));
    m_all_devices = NULL;

    m_header = NULL;
}

DeviceTabWidget::~DeviceTabWidget()
{
    QList<QAction *> a = actions();
    for(QList<QAction *>::iterator itr = a.begin(); itr != a.end(); ++itr)
    {
        removeAction(*itr);
        delete *itr;
    }
    removeAll();
}

void DeviceTabWidget::setHeader(analyzer_header *h)
{
    m_header = h;
    bool enable = h ? m_header->data_mask & DATA_DEVICE_ID : false;
    (*actions().begin())->setEnabled(enable);

    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
        itr->second->setHeader(h);

    if(m_all_devices)
        m_all_devices->setHeader(h);
}

void DeviceTabWidget::removeAll()
{
    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
        delete itr->second;
    m_devices.clear();
    delete m_all_devices;
    m_all_devices = NULL;
    clear();
}

CmdTabWidget *DeviceTabWidget::addDevice(bool all_devices, quint8 id)
{
    if(!all_devices)
    {
       dev_map::iterator itr = m_devices.find(id);
       if(itr != m_devices.end())
           return NULL;
    }
    else if(m_all_devices)
        return NULL;

    CmdTabWidget *cmd_tab = new CmdTabWidget(m_header, this, this);
    cmd_tab->addCommand();
    connect(cmd_tab, SIGNAL(updateData()), this, SIGNAL(updateData()));

    QString name;
    int index;
    if(!all_devices)
    {
        name = tr("Device") + " " + Utils::hexToString(id, true);
        m_devices.insert(std::make_pair<quint8,CmdTabWidget*>(id, cmd_tab));
        index = addTab(cmd_tab, name);
    }
    else
    {
        name = tr("All devices");
        m_all_devices = cmd_tab;
        index = insertTab(0, cmd_tab, name);
    }
    setCurrentIndex(index);
    return cmd_tab;
}

void DeviceTabWidget::handleData(analyzer_data *data, quint32 /*index*/)
{
    quint8 id = 0;
    if( m_all_devices)
        m_all_devices->handleData(data, 0);

    if(!m_id_enabled || (m_id_enabled && !data->getDeviceId(id)))
        return;

    dev_map::iterator itr = m_devices.find(id);
    if(itr == m_devices.end())
        return;
    itr->second->handleData(data, 0);
}

void DeviceTabWidget::newDevice()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New device"),
                                         tr("Device ID (hex or normal number):"), QLineEdit::Normal,
                                         "", &ok);
    int id = 0;
    quint8 res = 0;
    if(ok && !text.isEmpty())
    {
        if(text.contains("0x", Qt::CaseInsensitive))
        {
            text.replace("0x", "", Qt::CaseInsensitive);
            id = text.toInt(&ok, 16);
        }
        else
            id = text.toInt(&ok);
    }
    if(id > 255 || id < -128)
        ok = false;
    else
        res = (quint8)id;

    if(!ok)
    {
        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Wrong format, must be 8bit hex or normal number"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        return;
    }
    m_id_enabled = true;
    addDevice(false, res);
    setTabsClosable(true);
    emit updateData();
}

void DeviceTabWidget::tabClose(int index)
{
    QWidget *w = widget(index);
    removeTab(index);

    if(index == 0 && m_all_devices)
    {
        delete m_all_devices;
        m_all_devices = NULL;
        m_add_all_act->setEnabled(true);
        return;
    }

    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
    {
        if(itr->second == w)
        {
            m_devices.erase(itr);
            break;
        }
    }
    delete w;

    if(count() < 2)
    {
        setTabsClosable(false);
        if(m_all_devices)
            m_id_enabled = false;
    }
}

void DeviceTabWidget::addAllDevices()
{
    m_add_all_act->setEnabled(false);
    addDevice();
    if(count() > 1)
        setTabsClosable(true);
    emit updateData();
}

qint16 DeviceTabWidget::getCurrentDevice()
{
    int index = currentIndex();
    if(index == 0 && m_all_devices)
        return -1;

    QWidget *w = widget(index);
    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
        if(itr->second == w)
            return itr->first;
    return -1;
}

void DeviceTabWidget::Save(AnalyzerDataFile *file)
{
    quint32 count = m_devices.size();
    if(m_all_devices)
        ++count;
    file->write((char*)&count, sizeof(quint32));

    qint16 id;
    if(m_all_devices)
    {
        id = -1;
        file->writeBlockIdentifier(BLOCK_DEVICE_TAB);
        file->write((char*)&id, sizeof(qint16));
        file->writeBlockIdentifier(BLOCK_CMD_TABS);
        m_all_devices->Save(file);
    }

    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
    {
        id = itr->first;
        file->writeBlockIdentifier(BLOCK_DEVICE_TAB);
        file->write((char*)&id, sizeof(qint16));
        file->writeBlockIdentifier(BLOCK_CMD_TABS);
        itr->second->Save(file);
    }
}

void DeviceTabWidget::Load(AnalyzerDataFile *file, bool skip)
{
    removeAll();

    quint32 count = 0;
    file->read((char*)&count, sizeof(quint32));

    qint16 id;
    for(quint32 i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock(BLOCK_DEVICE_TAB, 0))
            break;
        file->read((char*)&id, sizeof(qint16));

        if(!file->seekToNextBlock(BLOCK_CMD_TABS, BLOCK_DEVICE_TAB))
            break;

        if(id == -1)
        {
           addAllDevices();
           m_all_devices->Load(file, skip);
        }
        else
        {
            m_id_enabled = true;
            setTabsClosable(true);
            CmdTabWidget *tab = addDevice(false, id);
            if(tab)
                tab->Load(file, skip);
        }
    }
}

bool DeviceTabWidget::setHighlightPos(const data_widget_info& info, bool highlight)
{
    if(m_all_devices && info.device == -1)
    {
        setCurrentIndex(0);
        return m_all_devices->setHighlightPos(info, highlight);
    }
    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
    {
        if(itr->first == info.device)
        {
            setCurrentIndex(indexOf(itr->second));
            return itr->second->setHighlightPos(info, highlight);
        }
    }
    return false;
}
