/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QLayout>
#include <QMessageBox>
#include <typeinfo>

#include "WorkTab.h"
#include "../connection/connectionmgr2.h"
#include "../connection/serialport.h"
#include "../connection/tcpsocket.h"

WorkTab::WorkTab() : QWidget(NULL)
{
    m_id = 0;
    m_info = NULL;
}

WorkTab::~WorkTab()
{
}

void WorkTab::DeleteAllMembers(QLayout *layout)
{
    while(layout->count())
    {
        QLayoutItem *item = layout->itemAt(0);
        layout->removeItem(item);
        if(item->layout())
        {
            WorkTab::DeleteAllMembers(item->layout());
            delete item->layout();
            continue;
        }
        else if(item->widget())
            delete item->widget();
        delete item;
    }
}

void WorkTab::onTabShow(const QString &)
{

}

bool WorkTab::onTabClose()
{
    return true;
}

void WorkTab::addTopMenu(QMenu *menu)
{
    m_menus.push_back(menu);
}

void WorkTab::openFile(const QString &/*filename*/)
{

}

void WorkTab::saveData(DataFileParser *file)
{
    file->writeString(GetIdString());
}

void WorkTab::loadData(DataFileParser *)
{

}

void WorkTab::childClosed(QWidget *child)
{
    delete child;
}

//----------------------------------------------------------------------------
PortConnWorkTab::PortConnWorkTab()
    : m_con(0)
{
}

PortConnWorkTab::~PortConnWorkTab()
{
    if (m_con)
        m_con->releaseTab();
}

void PortConnWorkTab::setConnection(PortConnection *con)
{
    if (m_con)
    {
        disconnect(m_con, 0, this, 0);
        m_con->releaseTab();
    }

    emit setConnId(con ? con->GetIDString() : QString(), m_con != NULL);

    m_con = con;

    if(!con)
        return;

    connect(m_con, SIGNAL(dataRead(QByteArray)), this, SLOT(readData(QByteArray)));
    connect(m_con, SIGNAL(connected(bool)), this, SLOT(connectedStatus(bool)));
    connect(m_con, SIGNAL(destroyed()), this, SLOT(connectionDestroyed()));
    m_con->addTabRef();
}

void PortConnWorkTab::connectionDestroyed()
{
    m_con = 0;
}

void PortConnWorkTab::readData(const QByteArray& /*data*/)
{

}

void PortConnWorkTab::connectedStatus(bool /*connected*/)
{

}

void PortConnWorkTab::saveData(DataFileParser *file)
{
    WorkTab::saveData(file);

    if(m_con && (m_con->getType() == CONNECTION_SERIAL_PORT || m_con->getType() == CONNECTION_TCP_SOCKET))
    {
        file->writeBlockIdentifier("portTabCon");
        file->writeVal(m_con->getType());

        QHash<QString, QVariant> cfg = m_con->config();
        file->writeVal(cfg.count());
        for(QHash<QString, QVariant>::iterator itr = cfg.begin(); itr != cfg.end(); ++itr)
        {
            file->writeString(itr.key());
            file->writeVal((int)(*itr).type());

            switch((*itr).type())
            {
                case QVariant::String:
                    file->writeString((*itr).toString());
                    break;
                case QVariant::Int:
                case QVariant::UInt:
                    file->writeVal((*itr).value<int>());
                    break;
                default:
                    break;
            }
        }
    }
}

void PortConnWorkTab::loadData(DataFileParser *file)
{
    WorkTab::loadData(file);

    if(file->seekToNextBlock("portTabCon", BLOCK_WORKTAB))
    {
        quint8 type = 0;
        file->readVal(type);

        int count = 0;
        file->readVal(count);
        QHash<QString, QVariant> cfg;
        for(int i = 0; i < count; ++i)
        {
            QString key = file->readString();
            int type = 0;
            file->readVal(type);
            QVariant val;
            switch(type)
            {
                case QVariant::String:
                    val = file->readString();
                    break;
                case QVariant::Int:
                case QVariant::UInt:
                {
                    int dta = 0;
                    file->readVal(dta);
                    val = dta;
                    val.convert((QVariant::Type)type);
                    break;
                }
                default:
                    break;
            }
            cfg.insert(key, val);
        }

        PortConnection *con = sConMgr2.getConnWithConfig(type, cfg);
        if(con)
        {
            setConnection(con);
            if(!con->isOpen() && sConfig.get(CFG_BOOL_SESSION_CONNECT))
                con->OpenConcurrent();
        }
    }
}
