/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QMouseEvent>
#include <QInputDialog>
#include <QMenu>
#include <QStyle>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QStylePainter>

#include "../WorkTab/WorkTabMgr.h"
#include "tabwidget.h"

TabWidget::TabWidget(quint32 id, QWidget *parent) :
    QTabWidget(parent)
{
    m_id = id;
    m_menu = new QMenu(this);

    m_tab_bar = new TabBar(m_id, this);
    setTabBar(m_tab_bar);
    m_menuBtn = new QPushButton(tr("Menu"), this);
    m_menuBtn->setMenu(m_menu);
    m_menuBtn->setFlat(true);

    QPushButton* newTabBtn = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "", this);

    setCornerWidget(m_menuBtn, Qt::TopLeftCorner);
    setCornerWidget(newTabBtn, Qt::TopRightCorner);

    connect(this,      SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
    connect(this,      SIGNAL(currentChanged(int)),    SLOT(currentIndexChanged(int)));
    connect(m_tab_bar, SIGNAL(tabMoved(int,int)),      SLOT(tabMoved(int,int)));
    connect(newTabBtn, SIGNAL(clicked()),              SLOT(newTabBtn()));
    connect(m_tab_bar, SIGNAL(plusPressed()),          SLOT(newTabBtn()));
    connect(m_tab_bar, SIGNAL(split(bool,int)),        SIGNAL(split(bool,int)));
    connect(m_tab_bar, SIGNAL(pullTab(int,TabWidget*,int)),
                       SLOT(pullTab(int,TabWidget*,int)), Qt::QueuedConnection);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

int TabWidget::addTab(WorkTab *widget, const QString &name, quint32 tabId)
{
    if(m_id == 0)
        sWorkTabMgr.CloseHomeTab();

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
    disconnect(tab, SIGNAL(statusBarMsg(QString,int)), this, SIGNAL(statusBarMsg(QString,int)));

    sWorkTabMgr.removeTab(tab);
    m_tab_ids.erase(itr);

    changeMenu(currentIndex());
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

int TabWidget::pullTab(int index, TabWidget *origin)
{
    QString name = origin->tabText(index);

    WorkTab *tab = (WorkTab*)origin->unregisterTab(index);
    int idx = addTab(tab, name, tab->getId());
    origin->checkEmpty();
    return idx;
}

void TabWidget::pullTab(int index, TabWidget *origin, int to)
{
    int id = pullTab(index, origin);
    tabBar()->moveTab(id, to);
}

QWidget *TabWidget::unregisterTab(int index)
{
    QWidget *tab = widget(index);

    Q_ASSERT(tab);

    removeTab(index);
    disconnect((WorkTab*)tab, SIGNAL(statusBarMsg(QString,int)), this, SIGNAL(statusBarMsg(QString,int)));

    std::vector<quint32>::iterator itr = m_tab_ids.begin() + index;
    m_tab_ids.erase(itr);

    changeMenu(currentIndex());
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
    if(!tab || tab->getMenu().empty())
        return clearMenu();

    m_menu->clear();
    for(quint32 i = 0; i < tab->getMenu().size(); ++i)
        m_menu->addMenu(tab->getMenu()[i]);
    m_menuBtn->setEnabled(true);
}

void TabWidget::clearMenu()
{
    m_menu->clear();
    m_menuBtn->setEnabled(false);
}

TabBar::TabBar(quint32 id, QWidget *parent) :
    PlusTabBar(parent)
{
    m_id = id;
    m_menu = new QMenu(this);
    m_drag_idx = -1;

    m_newTopBottom = m_menu->addAction(tr("Split view top/bottom"));
    m_newLeftRight = m_menu->addAction(tr("Split view left/right"));
    m_newTopBottom->setEnabled(false);
    m_newLeftRight->setEnabled(false);
    m_newTopBottom->setIcon(QIcon(":/icons/split_top.png"));
    m_newLeftRight->setIcon(QIcon(":/icons/split_left.png"));

    m_menu->addSeparator();

    QAction *rename = m_menu->addAction(tr("Rename..."));

    setAcceptDrops(true);

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

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton) || !tabsClosable())
        return PlusTabBar::mouseMoveEvent(event);

    int idx = tabAt(event->pos());
    if(idx == -1)
        return PlusTabBar::mouseMoveEvent(event);

    event->accept();

    QDrag *drag = new QDrag(this);

    QStyleOptionTabV3 tab;
    initStyleOption(&tab, idx);

    QSize size = tabRect(idx).size();
    QWidget *tabWidget = ((QTabWidget*)parent())->widget(idx);
    size.rwidth() = std::max(tabWidget->width(), size.width());
    size.rheight() += tabWidget->height();

    QPixmap map(size);
    map.fill(Qt::transparent);

    tabWidget->render(&map, QPoint(0, tab.rect.height()-5));

    QStylePainter p(&map, this);
    p.initFrom(this);
    tab.rect.moveTopLeft(tabRect(idx).topLeft());
    p.drawControl(QStyle::CE_TabBarTab, tab);
    p.end();

    QMimeData *mime = new QMimeData();
    mime->setText(QString("%1 %2").arg(m_id).arg(idx));
    mime->setData("data/tabinfo", QByteArray(1, ' '));

    drag->setPixmap(map);
    drag->setMimeData(mime);
    drag->setHotSpot(tabRect(idx).topLeft());
    drag->exec();
}

void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    if(!event->mimeData()->hasFormat("data/tabinfo"))
        return PlusTabBar::dragEnterEvent(event);

    event->acceptProposedAction();
    updateDropMarker(event->pos());
}

void TabBar::dragMoveEvent(QDragMoveEvent *event)
{
    PlusTabBar::dragMoveEvent(event);
    updateDropMarker(event->pos());
}

void TabBar::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    QStringList lst = event->mimeData()->text().split(' ');
    int tabId = lst[1].toInt();

    if(event->source() != this)
    {
        TabWidget *source = sWorkTabMgr.getWi()->getWidget(lst[0].toUInt());
        emit pullTab(tabId, source, m_drag_idx);
    }
    else
        moveTab(tabId, tabId < m_drag_idx ? m_drag_idx -1 : m_drag_idx);

    m_drag_insert.setRect(0, 0, 0, 0);
    update();
}

void TabBar::dragLeaveEvent(QDragLeaveEvent *event)
{
    PlusTabBar::dragLeaveEvent(event);
    m_drag_insert.setRect(0, 0, 0, 0);
    update();
}

void TabBar::updateDropMarker(const QPoint& pos)
{
    int idx = tabAt(pos);
    if(idx == -1)
    {
        m_drag_idx = -1;
        m_drag_insert.setRect(0, 0, 0, 0);
        return;
    }

    QRect rect = tabRect(idx);
    m_drag_insert.setTop(rect.top());
    m_drag_insert.setBottom(rect.bottom()-3);
    if(pos.x() - rect.left() >= rect.width()/2)
    {
        m_drag_idx = idx+1;
        m_drag_insert.setRight(rect.right()+2);
        m_drag_insert.setLeft(rect.right()-3);
    }
    else
    {
        m_drag_idx = idx;
        m_drag_insert.setRight(rect.left()+3);
        m_drag_insert.setLeft(rect.left()-2);
    }
    update();
}

void TabBar::paintEvent(QPaintEvent *event)
{
    PlusTabBar::paintEvent(event);

    if(m_drag_insert.isEmpty())
        return;

    QPainter p(this);
    p.setPen(Qt::red);
    p.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    p.drawRect(m_drag_insert);
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
