#include <QAction>
#include <QInputDialog>
#include <QMessageBox>

#include "cmdtabwidget.h"
#include "common.h"

CmdTabWidget::CmdTabWidget(analyzer_header *header, QWidget *parent) :
    QTabWidget(parent)
{
    setTabPosition(QTabWidget::South);

    QAction *new_device_act = new QAction(tr("Add command"), this);
    connect(new_device_act, SIGNAL(triggered()), this, SLOT(newCommand()));
    addAction(new_device_act);

    m_add_all_act = new QAction(tr("Add \"All commands\" tab"), this);
    m_add_all_act->setEnabled(false);
    connect(m_add_all_act, SIGNAL(triggered()), this, SLOT(addAllCmds()));
    addAction(m_add_all_act);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClose(int)));

    m_enableCmds = false;
    m_header = header;
    m_all_cmds = NULL;
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
    QWidget *w = new QWidget();
    ScrollDataLayout *layout = new ScrollDataLayout(m_header, false, true, w);
    QScrollArea *area = new QScrollArea(this);
    area->setWidget(w);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

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

void CmdTabWidget::handleData(analyzer_data *data)
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
}

void CmdTabWidget::addAllCmds()
{
    m_add_all_act->setEnabled(false);
    addCommand();
    if(count() > 1)
        setTabsClosable(true);
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
        return;
    }

    for(cmd_map::iterator itr = m_cmds.begin(); itr != m_cmds.end(); ++itr)
    {
        if(itr->second->a == w)
        {
            delete itr->second;
            m_cmds.erase(itr);
            break;
        }
    }

    if(count() < 2)
    {
        setTabsClosable(false);
        if(m_all_cmds)
            m_enableCmds = false;
    }
}
