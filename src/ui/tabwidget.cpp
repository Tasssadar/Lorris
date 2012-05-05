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

#include <QMouseEvent>
#include <QInputDialog>
#include <QMenu>
#include <QStyle>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QMenuBar>
#include <QHBoxLayout>

#include "../WorkTab/WorkTabMgr.h"
#include "tabwidget.h"

TabWidget::TabWidget(quint32 id, QWidget *parent) :
    QTabWidget(parent)
{
    m_id = id;

    m_tab_bar = new TabBar(this);
    setTabBar(m_tab_bar);
    setMovable(true);

    QWidget *cornerWidget = new QWidget(this);
    m_cornerLayout = new QHBoxLayout(cornerWidget);

    QPushButton* newTabBtn = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "", this);
    m_cornerLayout->addWidget(newTabBtn);

    setCornerWidget(cornerWidget);

    connect(this,      SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
    connect(this,      SIGNAL(currentChanged(int)),    SLOT(currentIndexChanged(int)));
    connect(m_tab_bar, SIGNAL(tabMoved(int,int)),      SLOT(tabMoved(int,int)));
    connect(newTabBtn, SIGNAL(clicked()),              SIGNAL(newTab()));
    connect(m_tab_bar, SIGNAL(split(bool,int)),        SIGNAL(split(bool,int)));
    connect(m_tab_bar, SIGNAL(plusPressed()),          SIGNAL(newTab()));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

int TabWidget::addTab(WorkTab *widget, const QString &name, quint32 tabId)
{
    int idx = QTabWidget::addTab(widget, name);

    std::vector<quint32>::iterator itr = m_tab_ids.begin() + idx;
    m_tab_ids.insert(itr, tabId);

    setCurrentIndex(idx);
    changeMenu(idx);

    if(count() >= 2)
        m_tab_bar->enableSplit(true);

    connect(widget, SIGNAL(statusBarMsg(QString,int)), SIGNAL(statusBarMsg(QString,int)));

    setTabsClosable(true);
    return idx;
}

void TabWidget::checkEmpty()
{
    if(count() < 2)
        m_tab_bar->enableSplit(false);

    if(count() != 0)
        return;

    if(m_id == 0) // check if we are the first tabWidget
    {
        emit openHomeTab(m_id);
        setTabsClosable(false);
    }
    else
        emit removeWidget(m_id);
}

void TabWidget::closeTab(int index)
{
    if(index < 0 || m_tab_ids.size() <= (uint)index)
    {
        Q_ASSERT(false);
        return;
    }

    std::vector<quint32>::iterator itr = m_tab_ids.begin() + index;

    WorkTab *tab = sWorkTabMgr.getWorkTab(*itr);
    if(!tab->onTabClose())
        return;

    removeTab(index);
    sWorkTabMgr.removeTab(tab);
    m_tab_ids.erase(itr);

    checkEmpty();
}

void TabWidget::tabMoved(int from, int to)
{
    if(from < 0 || m_tab_ids.size() <= (uint)from)
    {
        Q_ASSERT(false);
        return;
    }

    std::vector<quint32>::iterator itr = m_tab_ids.begin() + from;
    quint32 id = *itr;

    m_tab_ids.erase(itr);

    itr = m_tab_ids.begin() + to;
    m_tab_ids.insert(itr, id);
}

void TabWidget::pullTab(int index, TabWidget *origin)
{
    QString name = origin->tabText(index);

    WorkTab *tab = (WorkTab*)origin->unregisterTab(index);
    addTab(tab, name, tab->getId());
}

QWidget *TabWidget::unregisterTab(int index)
{
    QWidget *tab = widget(index);

    Q_ASSERT(tab);

    removeTab(index);

    std::vector<quint32>::iterator itr = m_tab_ids.begin() + index;
    m_tab_ids.erase(itr);

    checkEmpty();

    return tab;
}

void TabWidget::currentIndexChanged(int idx)
{
    changeMenu(idx);
}

void TabWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::MidButton || !checkEvent(event))
        return QTabWidget::mousePressEvent(event);
}

void TabWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || !checkEvent(event))
        return QTabWidget::mouseDoubleClickEvent(event);
}

bool TabWidget::checkEvent(QMouseEvent *event)
{
    if(event->pos().y() > tabBar()->height())
        return false;

    emit changeActiveWidget(this);
    emit newTab();
    return true;
}

void TabWidget::newTabBtn()
{
    emit changeActiveWidget(this);
    emit newTab();
}

void TabWidget::changeMenu(int idx)
{
    if(idx == -1 || (uint)idx >= m_tab_ids.size())
        return clearMenu();

    WorkTab *tab = sWorkTabMgr.getWorkTab(m_tab_ids[idx]);
    if(!tab)
        return clearMenu();

    while(m_menuBtns.size() != tab->getMenu().size())
    {
        if(m_menuBtns.size() < tab->getMenu().size())
        {
            QPushButton *btn = new QPushButton(this);
            btn->setFlat(true);
            btn->setStyleSheet("padding: 0px 5px 0px 15px");

            m_menuBtns.push_back(btn);
            m_cornerLayout->insertWidget(0, btn);
        }
        else
        {
            delete m_menuBtns.back();
            m_menuBtns.pop_back();
        }
    }

    for(quint32 i = 0; i < tab->getMenu().size(); ++i)
    {
        m_menuBtns[i]->setText(tab->getMenu()[i]->title());
        m_menuBtns[i]->setMenu(tab->getMenu()[i]);
    }
}

void TabWidget::clearMenu()
{
    for(std::vector<QPushButton*>::iterator itr = m_menuBtns.begin(); itr != m_menuBtns.end(); ++itr)
        delete *itr;
    m_menuBtns.clear();
}

TabBar::TabBar(QWidget *parent) :
    PlusTabBar(parent)
{
    m_menu = new QMenu(this);

    m_newTopBottom = m_menu->addAction(tr("Split view top/bottom"));
    m_newLeftRight = m_menu->addAction(tr("Split view left/right"));
    m_newTopBottom->setEnabled(false);
    m_newLeftRight->setEnabled(false);
    m_newTopBottom->setIcon(QIcon(":/icons/split_top.png"));
    m_newLeftRight->setIcon(QIcon(":/icons/split_left.png"));

    m_menu->addSeparator();

    QAction *rename = m_menu->addAction(tr("Rename..."));

    connect(rename,         SIGNAL(triggered()), SLOT(renameTab()));
    connect(m_newTopBottom, SIGNAL(triggered()), SLOT(splitTop()));
    connect(m_newLeftRight, SIGNAL(triggered()), SLOT(splitLeft()));
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    switch(event->button())
    {
        case Qt::MidButton:
        {
            int tab = tabAt(event->pos());

            if(tab != -1)
                emit tabCloseRequested(tab);
            break;
        }
        case Qt::RightButton:
        {
            int tab = tabAt(event->pos());
            if(tab == -1 || tabText(tab) == "Home")
                break;

            m_cur_menu_tab = tab;

            m_menu->exec(event->globalPos());
            break;
        }
        default:
            PlusTabBar::mousePressEvent(event);
            break;
    }
}

void TabBar::renameTab()
{
    QString name = QInputDialog::getText(this, tr("Rename tab"), tr("New name:"),
                                         QLineEdit::Normal, tabText(m_cur_menu_tab));
    if(!name.isEmpty())
        setTabText(m_cur_menu_tab, name);
}

void TabBar::splitTop()
{
    emit split(true, m_cur_menu_tab);
}

void TabBar::splitLeft()
{
    emit split(false, m_cur_menu_tab);
}

void TabBar::enableSplit(bool enable)
{
    m_newTopBottom->setEnabled(enable);
    m_newLeftRight->setEnabled(enable);
}
