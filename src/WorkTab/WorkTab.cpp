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


WorkTab::WorkTab() : Tab(TABTYPE_WORKTAB, NULL)
{
    m_id = 0;
    m_info = NULL;
}

WorkTab::~WorkTab()
{
}

void WorkTab::onTabShow(const QString &)
{

}

void WorkTab::addTopMenu(QMenu *menu)
{
    m_actions.push_back(menu->menuAction());
}

void WorkTab::addTopAction(QAction *act)
{
    m_actions.push_back(act);
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

#ifdef Q_OS_MAC
QMacToolBarItem *WorkTab::addItemMacToolBar(const QIcon &icon, const QString &text)
{
    QMacToolBarItem *btn = new QMacToolBarItem;
    btn->setIcon(icon);
    btn->setText(text);
    m_macBarItems.push_back(btn);

    return btn;
}
#endif

//----------------------------------------------------------------------------
PortConnWorkTab::PortConnWorkTab()
{
}

PortConnWorkTab::~PortConnWorkTab()
{
    if (m_con)
        m_con->releaseTab();
}

void PortConnWorkTab::setConnection(ConnectionPointer<Connection> const & con)
{
    this->setPortConnection(con.dynamicCast<PortConnection>());
}

void PortConnWorkTab::setPortConnection(ConnectionPointer<PortConnection> const & con)
{
    if (m_con)
    {
        m_con->disconnect(this);
        this->disconnect(m_con.data());
        m_con->releaseTab();
    }

    emit setConnId(con ? con->GetIDString() : QString(), m_con != NULL);

    m_con = con;

    if(m_con)
    {
        connect(m_con.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(readData(QByteArray)));
        connect(m_con.data(), SIGNAL(connected(bool)), this, SLOT(connectedStatus(bool)));
        m_con->addTabRef();
    }
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

    if(m_con && m_con->canSaveToSession())
    {
        file->writeBlockIdentifier("portTabConV2");
        file->writeConn(m_con.data());
    }
}

void PortConnWorkTab::loadData(DataFileParser *file)
{
    WorkTab::loadData(file);

    if(file->seekToNextBlock("portTabConV2", BLOCK_WORKTAB))
    {
        quint8 type = 0;
        QHash<QString, QVariant> cfg;

        if(file->readConn(type, cfg))
        {
            ConnectionPointer<Connection> con = sConMgr2.getConnWithConfig(type, cfg);
            if(con)
            {
                setConnection(con);
                if(!con->isOpen() && sConfig.get(CFG_BOOL_SESSION_CONNECT))
                    con->OpenConcurrent();
            }
        }
    }
}
