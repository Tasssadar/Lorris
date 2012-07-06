/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>

#include "cmdtabwidget.h"
#include "../common.h"
#include "DataWidgets/datawidget.h"
#include "../misc/datafileparser.h"
#include "../ui/plustabbar.h"

CmdTabWidget::CmdTabWidget(analyzer_header *header, DeviceTabWidget *device, QWidget *parent) :
    QTabWidget(parent)
{
    PlusTabBar *bar = new PlusTabBar(this);
    setTabBar(bar);
    connect(bar,  SIGNAL(plusPressed()),            SLOT(newCommand()));
    connect(this, SIGNAL(disableCmdAdd(bool)), bar, SLOT(setDisablePlus(bool)));

    setTabPosition(QTabWidget::South);

    new_cmd_act = new QAction(tr("Add command"), this);
    if(!header || !(header->data_mask & DATA_OPCODE))
    {
        new_cmd_act->setEnabled(false);
        emit disableCmdAdd(true);
    }
    connect(new_cmd_act, SIGNAL(triggered()), this, SLOT(newCommand()));
    addAction(new_cmd_act);

    m_add_all_act = new QAction(tr("Add \"All commands\" tab"), this);
    m_add_all_act->setEnabled(false);
    connect(m_add_all_act, SIGNAL(triggered()), this, SLOT(addAllCmds()));
    addAction(m_add_all_act);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClose(int)));

    m_enableCmds = false;
    m_header = header;
    m_all_cmds = NULL;
    m_devTab = device;
}

CmdTabWidget::~CmdTabWidget()
{
    QList<QAction *> a = actions();
    for(QList<QAction *>::iterator itr = a.begin(); itr != a.end(); ++itr)
    {
        removeAction(*itr);
        delete *itr;
    }

    removeAll();
}

void CmdTabWidget::removeAll()
{
    for(cmd_map::iterator itr = m_cmds.begin(); itr != m_cmds.end(); ++itr)
        delete itr->second;
    m_cmds.clear();

    delete m_all_cmds;
    m_all_cmds = NULL;
    clear();
}

void CmdTabWidget::addCommand(bool add_all_cmds, quint8 id)
{
    if(!add_all_cmds)
    {
       cmd_map::iterator itr = m_cmds.find(id);
       if(itr != m_cmds.end())
           return;
    }
    else if(m_all_cmds)
        return;

    QScrollArea *area = new QScrollArea(this);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ScrollDataLayout *layout = NULL;
    QWidget *w = new QWidget();

    QPalette p = area->palette();
    p.setColor(QPalette::Window, QColor("#F5F5F5"));
    area->setPalette(p);
    area->setAutoFillBackground(true);

    if(m_header)
    {
        layout = new ScrollDataLayout(m_header, false, true, this, m_devTab, w);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    area->setWidget(w);

    QString name;
    int index;
    if(!add_all_cmds)
    {
        name = tr("Command") + " " + Utils::hexToString(id, true);
        m_cmds.insert(std::make_pair<quint8,CmdTabInfo*>(id, new CmdTabInfo(area, layout)));
        index = addTab(area, name);
    }
    else
    {
        name = tr("All commands");
        m_all_cmds = new CmdTabInfo(area, layout);
        index = insertTab(0, area, name);
    }
    setCurrentIndex(index);
}

void CmdTabWidget::handleData(analyzer_data *data, quint32 /*index*/)
{
    if(m_all_cmds)
        m_all_cmds->l->SetData(data->getData());

    quint8 cmd = 0;
    if(!m_enableCmds || (m_enableCmds && !data->getCmd(cmd)))
        return;

    cmd_map::iterator itr = m_cmds.find(cmd);
    if(itr == m_cmds.end())
        return;
    itr->second->l->SetData(data->getData());
}

void CmdTabWidget::newCommand()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New command"),
                                         tr("Command (hex or normal number):"), QLineEdit::Normal,
                                         "", &ok);
    if(!ok)
        return;

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
    addCommand(false, res);
    m_enableCmds = true;
    setTabsClosable(true);
    emit updateData();
}

void CmdTabWidget::addAllCmds()
{
    m_add_all_act->setEnabled(false);
    addCommand();
    if(count() > 1)
        setTabsClosable(true);
    emit updateData();
}

void CmdTabWidget::tabClose(int index)
{
    QWidget *w = widget(index);
    removeTab(index);

    if(index == 0 && m_all_cmds)
    {
        delete m_all_cmds;
        m_all_cmds = NULL;
        m_add_all_act->setEnabled(true);
    }
    else
    {
        for(cmd_map::iterator itr = m_cmds.begin(); itr != m_cmds.end(); ++itr)
        {
            if(itr->second->a == w)
            {
                delete itr->second;
                m_cmds.erase(itr);
                break;
            }
        }
    }
    if(count() < 2)
    {
        setTabsClosable(false);
        if(m_all_cmds)
            m_enableCmds = false;
    }
}

qint16 CmdTabWidget::getCurrentCmd()
{
    int index = currentIndex();
    if(index == 0 && m_all_cmds)
        return -1;

    QWidget *w = widget(index);
    for(cmd_map::iterator itr = m_cmds.begin(); itr != m_cmds.end(); ++itr)
        if(itr->second->a == w)
            return itr->first;
    return -1;
}

void CmdTabWidget::Save(DataFileParser *file)
{
    quint32 size = m_cmds.size();
    if(m_all_cmds)
        ++size;
    file->write((char*)&size, sizeof(quint32));

    qint16 id;
    if(m_all_cmds)
    {
        id = -1;
        file->writeBlockIdentifier(BLOCK_CMD_TAB);
        file->write((char*)&id, sizeof(qint16));
    }

    for(cmd_map::iterator itr = m_cmds.begin(); itr != m_cmds.end(); ++itr)
    {
        id = itr->first;
        file->writeBlockIdentifier(BLOCK_CMD_TAB);
        file->write((char*)&id, sizeof(qint16));
    }
}

void CmdTabWidget::Load(DataFileParser *file, bool /*skip*/)
{
    quint32 count = 0;
    file->read((char*)&count, sizeof(quint32));

    qint16 id;
    for(quint32 i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock(BLOCK_CMD_TAB, BLOCK_CMD_TABS))
            break;
        file->read((char*)&id, sizeof(qint16));
        if(id == -1)
            addAllCmds();
        else
        {
            addCommand(false, id);
            m_enableCmds = true;
            setTabsClosable(true);
        }
    }
}

bool CmdTabWidget::setHighlightPos(const data_widget_info& info, bool highlight)
{
    if(m_all_cmds && info.command == -1)
    {
        setCurrentIndex(0);
        return m_all_cmds->l->setHightlightLabel(info.pos, highlight);
    }

    for(cmd_map::iterator itr = m_cmds.begin(); itr != m_cmds.end(); ++itr)
    {
        if(itr->first == info.command)
        {
            setCurrentIndex(indexOf(itr->second->a));
            return itr->second->l->setHightlightLabel(info.pos, highlight);
        }
    }
    return false;
}

void CmdTabWidget::setHeader(analyzer_header *header)
{
    m_header = header;
    new_cmd_act->setEnabled(header->data_mask & DATA_OPCODE);
    emit disableCmdAdd(!(header->data_mask & DATA_OPCODE));

    if(m_all_cmds && m_all_cmds->l)
        m_all_cmds->l->setHeader(header);

    for(cmd_map::iterator itr = m_cmds.begin(); itr != m_cmds.end(); ++itr)
        itr->second->l->setHeader(header);
}
