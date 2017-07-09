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
#include <QHBoxLayout>
#include <QStyleOption>
#include <QStylePainter>
#include <algorithm>
#include <QShortcut>
#include <QLabel>
#include <QDrag>
#include <QMimeData>

#ifdef Q_OS_MAC
#include <QtMacExtras>
#endif

#include "../WorkTab/WorkTabMgr.h"
#include "tabwidget.h"
#include "../misc/datafileparser.h"
#include "../WorkTab/childtab.h"
#include "HomeTab.h"

#include "ui_tabswitchwidget.h"

#define HOME_TAB QVariant('H')

TabWidget::TabWidget(quint32 id, QWidget *parent) :
    QTabWidget(parent)
{
    m_id = id;
    m_menu = new AltClosableMenu(this);
    m_switchWidget = NULL;
    m_altEventValid = false;


    m_tab_bar = new TabBar(m_id, this);
    setTabBar(m_tab_bar);

    m_menuBtn = new QPushButton(tr("&Menu"), this);
    m_menuBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_menuBtn->setMenu(m_menu);
    m_menuBtn->setFlat(true);
    m_menuBtn->installEventFilter(this);
#ifndef Q_OS_MAC
    setCornerWidget(m_menuBtn, Qt::TopLeftCorner);
#else
    setDocumentMode(true);
    m_menuBtn->hide();
    m_macBar = new QMacToolBar();
#endif

    new QShortcut(QKeySequence("Ctrl+T"), this, SLOT(newTabBtn()), NULL, Qt::WidgetWithChildrenShortcut);

    connect(this,      SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
    connect(this,      SIGNAL(currentChanged(int)),    SLOT(currentIndexChanged(int)));
    connect(m_tab_bar, SIGNAL(tabMoved(int,int)),      SLOT(tabMoved(int,int)));
    connect(m_tab_bar, SIGNAL(plusPressed()),          SLOT(newTabBtn()));
    connect(m_tab_bar, SIGNAL(split(int,int)),         SIGNAL(split(int,int)));
    connect(m_tab_bar, SIGNAL(pullTab(int,TabWidget*,int)),
                       SLOT(pullTab(int,TabWidget*,int)), Qt::QueuedConnection);
    connect(m_tab_bar, SIGNAL(newWindow(int)),         SLOT(newWindow(int)), Qt::QueuedConnection);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    sWorkTabMgr.registerTabWidget(this);
}

int TabWidget::addTab(HomeTab *tab, const QString& name)
{
    int idx = QTabWidget::addTab(tab, name);
    m_tab_bar->setTabData(idx, HOME_TAB);
    return idx;
}

int TabWidget::addTab(Tab *widget, const QString &name, quint32 tabId)
{
    if(!widget->isHometab())
        emit closeHomeTab();

    int idx = QTabWidget::addTab(widget, name);

    installEventFilterToChildren(widget);

    std::vector<quint32>::iterator itr = m_tab_ids.begin() + idx;
    m_tab_ids.insert(itr, tabId);

    setCurrentIndex(idx);
    changeMenu(idx);

    if(count() >= 2)
        m_tab_bar->enableSplit(true);

    if(widget->isWorkTab())
    {
        connect(widget, SIGNAL(statusBarMsg(QString,int)),     SIGNAL(statusBarMsg(QString,int)));
        connect(widget, SIGNAL(setConnId(QString,bool)),       SLOT(setConnString(QString,bool)));
    }
    else if(widget->isChild())
        connect((ChildTab*)widget, SIGNAL(tabText(QString)), SLOT(setTabNameAndTooltip(QString)));
    connect(widget, SIGNAL(activateMe()), SLOT(activateTab()));
    connect(widget, SIGNAL(destroyed()), SLOT(checkEmpty()), Qt::QueuedConnection);

    setTabNameAndTooltip(idx, name);
    setTabsClosable(true);
    return idx;
}

void TabWidget::checkEmpty()
{
    if(count() < 2)
        m_tab_bar->enableSplit(false);

    if(count() != 0)
        return;

    emit removeWidget(m_id);
}

bool TabWidget::closeTab(int index)
{
    if(index < 0 || m_tab_ids.size() <= (uint)index)
    {
        Q_ASSERT(false);
        return false;
    }

    quint32 id = m_tab_ids[index];
    Tab *tab = dynamic_cast<Tab*>(widget(index));
    if(!tab)
    {
        Q_ASSERT(false);
        return false;
    }

    if(!tab->onTabClose())
        return false;

    if(id & IDMASK_CHILD)
    {
        sWorkTabMgr.removeChildTab((ChildTab*)tab);
    }
    else
    {
        disconnect((WorkTab*)tab, SIGNAL(statusBarMsg(QString,int)), this, SIGNAL(statusBarMsg(QString,int)));
        sWorkTabMgr.removeTab((WorkTab*)tab);
    }

    changeMenu(currentIndex());
    checkEmpty();
    return true;
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

    // FIXME: tab content is not properly updated (Qt bug?)
    // Two tabs, moveTab(1, 0); results in currentIndex = 0,
    // but tab 1's content is displayed
    setCurrentIndex(from);
    setCurrentIndex(to);
}

int TabWidget::pullTab(int index, TabWidget *origin)
{
    QString name = origin->tabText(index);

    Tab *tab = (Tab*)origin->unregisterTab(index);
    int idx = addTab(tab, name, tab->isWorkTab() ? ((WorkTab*)tab)->getId() : ((ChildTab*)tab)->getId());
    origin->checkEmpty();
    tab->setWindowId(tabView()->getWindowId());
    return idx;
}

void TabWidget::pullTab(int index, TabWidget *origin, int to)
{
    int id = pullTab(index, origin);
    tabBar()->moveTab(id, to);
}

QWidget *TabWidget::unregisterTab(int index)
{
    Tab *tab = (Tab*)widget(index);

    Q_ASSERT(tab);

    removeTab(index);
    removeEventFilterFromChildren(tab);

    tab->disconnect(this);
    this->disconnect(tab);

    changeMenu(currentIndex());
    return tab;
}

void TabWidget::currentIndexChanged(int idx)
{
    changeMenu(idx);

    if(idx == -1)
        return;

    emit changeWindowTitle(tabToolTip(idx));

    if((size_t)idx < m_tab_ids.size())
    {
        m_tabHistory.removeOne(m_tab_ids[idx]);
        m_tabHistory.push_back(m_tab_ids[idx]);
    }
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
    if(!tab || tab->getActions().empty())
        return clearMenu();

    clearMenu();

    const std::vector<QAction*>& acts = tab->getActions();
    for(quint32 i = 0; i < acts.size(); ++i) {
        m_menu->addAction(acts[i]);

#ifdef Q_OS_MAC
        tabView()->m_menuBar->addAction(acts[i]);

        m_macBar = new QMacToolBar();
        m_macBar->setItems(tab->getMacBarItems());
        m_macBar->attachToWindow(window()->windowHandle());
        m_macBar->addSeparator();
#endif
    }
    m_menuBtn->setEnabled(true);
}

void TabWidget::clearMenu()
{
    m_menu->clear();

#ifdef Q_OS_MAC
    tabView()->m_menuBar->clear();

    QList<QMacToolBarItem *> emptyList = QList<QMacToolBarItem *>();
    m_macBar->setItems(emptyList);
    m_macBar->detachFromWindow();
    m_macBar->addSeparator();
    m_macBar->attachToWindow(window()->windowHandle());
#endif

    const std::vector<QAction*>& menus = tabView()->getMenus();
    for(quint32 i = 0; i < menus.size(); ++i) {
        m_menu->addAction(menus[i]);

#ifdef Q_OS_MAC
        tabView()->m_menuBar->addAction(menus[i]);
#endif
    }

    m_menu->addSeparator();
}

void TabWidget::saveData(DataFileParser *file)
{
    file->writeVal(m_id);

    if(m_tab_ids.empty())
    {
        file->writeVal((quint32)0);
        return;
    }

    file->writeVal(count());
    for(int i = 0; i < count(); ++i)
    {
        Tab *tab = (Tab*)widget(i);
        if(tab->isWorkTab())
        {
            file->writeBlockIdentifier("tabWidgetTab");

            QString name = tabToolTip(i);
            int idx = name.lastIndexOf(" - ");
            if(idx != -1)
                name = name.left(idx);
            file->writeString(name);

            ((WorkTab*)tab)->saveData(file);
        }
    }

    file->writeBlockIdentifier("tabWidgetIdx");
    file->writeVal(currentIndex());
}

void TabWidget::loadData(DataFileParser *file)
{
    int count = 0;
    file->readVal(count);

    const WorkTabMgr::InfoList& info = sWorkTabMgr.GetWorkTabInfos();

    for(int i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock("tabWidgetTab", "tabWidget"))
            break;

        QString name = file->readString();
        QString id = file->readString();

        WorkTab *tab = NULL;
        for(WorkTabMgr::InfoList::const_iterator itr = info.begin(); !tab && itr != info.end(); ++itr)
            if((*itr)->GetIdString() == id)
                tab = sWorkTabMgr.GetNewTab(*itr);

        if(!tab)
            continue;

        tab->setParent(this);
        tab->setWindowId(((TabView*)parent())->getWindowId());

        sWorkTabMgr.registerTab(tab);
        addTab(tab, name, tab->getId());

        tab->loadData(file);
    }

    if(file->seekToNextBlock("tabWidgetIdx", "tabWidget"))
    {
        int idx = 0;
        file->readVal(idx);
        setCurrentIndex(idx);
    }

    checkEmpty();
}

void TabWidget::setConnString(const QString &str, bool hadConn)
{
    Q_ASSERT(sender());
    if(!sender())
        return;

    int idx = indexOf((QWidget*)sender());
    if(idx == -1)
        return;

    QString text = tabToolTip(idx);
    if(!hadConn)
        text += " - " + str;
    else
    {
        text = text.left(text.lastIndexOf(" - "));
        if(!str.isEmpty())
            text += " - " + str;
    }

    setTabNameAndTooltip(idx, text);
}

void TabWidget::setTabNameAndTooltip(int idx, QString name)
{
    setTabToolTip(idx, name);

    if(idx == currentIndex())
        emit changeWindowTitle(name);

    if(name.size() > 25)
    {
        name.resize(28);
        name.replace(25, 3, "...");
    }

    setTabText(idx, name);
}

void TabWidget::setTabNameAndTooltip(QString name)
{
    Q_ASSERT(sender());
    if(!sender())
        return;

    int idx = indexOf((QWidget*)sender());
    if(idx == -1)
        return;

    setTabNameAndTooltip(idx, name);
}

void TabWidget::addChildTab(ChildTab *widget, const QString& name)
{
    quint32 id = sWorkTabMgr.generateNewChildId();
    widget->setId(id);
    addTab(widget, name, id);
}

void TabWidget::removeChildTab(ChildTab *widget)
{
    int idx = indexOf(widget);
    if(idx == -1)
        return;

    removeTab(idx);

    changeMenu(currentIndex());
    checkEmpty();
}

bool TabWidget::containsTab(quint32 id) const
{
    return std::find(m_tab_ids.begin(), m_tab_ids.end(), id) != m_tab_ids.end();
}

void TabWidget::activateTab()
{
    Q_ASSERT(sender());
    Q_ASSERT(sender()->inherits("QWidget"));
    if(!sender() || !sender()->inherits("QWidget"))
        return;

    int idx = indexOf((QWidget*)sender());
    if(idx == -1)
        return;
    setCurrentIndex(idx);
}

void TabWidget::newWindow(int idx)
{
    MainWindow *window = sWorkTabMgr.newWindow();
    window->getTabView()->getActiveWidget()->pullTab(idx, this);
}

void TabWidget::keyPressEvent(QKeyEvent *event)
{
    if(!(event->modifiers() & Qt::ControlModifier) || event->key() != Qt::Key_Tab)
        return QTabWidget::keyPressEvent(event);

    if(!m_switchWidget)
    {
        m_switchWidget = new TabSwitchWidget(this);
        m_switchWidget->move(width()/2 - m_switchWidget->width()/2,
                             height()/2 - m_switchWidget->height()/2);
        m_switchWidget->show();
        connect(m_switchWidget, SIGNAL(setIndex(int)), SLOT(setCurrentIndex(int)));
    }
    m_switchWidget->next();
}

void TabWidget::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        delete m_switchWidget;
        m_switchWidget = NULL;
    }
}

void TabWidget::tabRemoved(int index)
{
    if(m_tab_ids.size() != (size_t)count())
    {
        m_tabHistory.removeOne(m_tab_ids[index]);
        m_tab_ids.erase(m_tab_ids.begin() + index);
    }
}

bool TabWidget::eventFilter(QObject *obj, QEvent *ev)
{
    switch(ev->type())
    {
        case QEvent::ChildAdded:
            installEventFilterToChildren(((QChildEvent*)ev)->child());
            return false;
        case QEvent::ChildRemoved:
            removeEventFilterFromChildren(((QChildEvent*)ev)->child());
            return false;
        case QEvent::KeyPress:
        {
            QKeyEvent *keyEv = (QKeyEvent*)ev;

            m_altEventValid = (keyEv->key() == Qt::Key_Alt && keyEv->modifiers() == Qt::AltModifier);

            if(!(keyEv->modifiers() & Qt::ControlModifier) || keyEv->key() != Qt::Key_Tab)
                return false;

            keyPressEvent(keyEv);
            return true;
        }
        case QEvent::KeyRelease:
        {
            QKeyEvent *keyEv = (QKeyEvent*)ev;

            if(m_altEventValid && keyEv->key() == Qt::Key_Alt && keyEv->modifiers() == Qt::NoModifier)
            {
                m_menuBtn->animateClick();
                m_altEventValid = false;
                return false;
            }

            if(keyEv->key() == Qt::Key_Control)
                keyReleaseEvent(keyEv);
            return false;
        }
        case QEvent::FocusIn:
        {
            emit changeActiveWidget(this);
            return false;
        }
        default:break;
    }
    return QTabWidget::eventFilter(obj, ev);
}

void TabWidget::installEventFilterToChildren(QObject *object)
{
    object->installEventFilter(this);
    const QObjectList &children = object->children();
    for(int i = 0; i < children.size(); ++i)
        installEventFilterToChildren(children[i]);
}

void TabWidget::removeEventFilterFromChildren(QObject *object)
{
    object->removeEventFilter(this);
    const QObjectList &children = object->children();
    for(int i = 0; i < children.size(); ++i)
        removeEventFilterFromChildren(children[i]);
}

bool TabWidget::canCloseTabs()
{
    if(m_tab_ids.empty())
        return true;

    for(int i = 0; i < count(); ++i)
    {
        Tab *tab = dynamic_cast<Tab*>(widget(i));
        if(!tab)
            continue;

        if(tab->isChild())
        {
            ChildTab* child = (ChildTab*)tab;
            WorkTab* parent = sWorkTabMgr.getWorkTab(child->getParentId());
            if(child->getWindowId() == parent->getWindowId())
                continue;
        }

        if(!tab->onTabClose())
            return false;
    }
    return true;
}

void TabWidget::forceCloseChilds()
{
    if(m_tab_ids.empty())
        return;

    for(int i = 0; i < count(); ++i)
    {
        ChildTab *tab = dynamic_cast<ChildTab*>(widget(i));
        if(!tab)
            continue;
        sWorkTabMgr.removeChildTab(tab);
        i = 0;
    }
}

TabBar::TabBar(quint32 id, QWidget *parent) :
    PlusTabBar(parent)
{
    m_id = id;
    m_menu = new QMenu(this);
    m_drag_idx = -1;
    m_drag_insert = NULL;

    m_newTopBottom = m_menu->addAction(tr("Split view top/bottom"));
    m_newLeftRight = m_menu->addAction(tr("Split view left/right"));
    m_newTopBottom->setEnabled(false);
    m_newLeftRight->setEnabled(false);
    m_newTopBottom->setIcon(QIcon(":/icons/split_top.png"));
    m_newLeftRight->setIcon(QIcon(":/icons/split_left.png"));

    QAction *newWindow = m_menu->addAction(tr("To new window"));

    m_menu->addSeparator();

    QAction *rename = m_menu->addAction(tr("Rename..."));

    setAcceptDrops(true);

    connect(rename,         SIGNAL(triggered()), SLOT(renameTab()));
    connect(m_newTopBottom, SIGNAL(triggered()), SLOT(splitTop()));
    connect(m_newLeftRight, SIGNAL(triggered()), SLOT(splitLeft()));
    connect(newWindow,      SIGNAL(triggered()), SLOT(toNewWindow()));
}

TabBar::~TabBar()
{
    delete m_drag_insert;
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    switch(event->button())
    {
        case Qt::LeftButton:
            m_startDragPos = event->pos();
            PlusTabBar::mousePressEvent(event);
            break;
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
            if(tab == -1 || tabData(tab) == HOME_TAB)
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

void TabBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return PlusTabBar::mouseDoubleClickEvent(event);

    int tab = tabAt(event->pos());
    if(tab == -1 || tabData(tab) == HOME_TAB)
        return PlusTabBar::mouseDoubleClickEvent(event);

    m_cur_menu_tab = tab;
    renameTab();
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton) || !tabsClosable())
        return PlusTabBar::mouseMoveEvent(event);

    if((event->pos() - m_startDragPos).manhattanLength() < QApplication::startDragDistance())
        return PlusTabBar::mouseMoveEvent(event);

    int idx = tabAt(event->pos());
    if(idx == -1)
        return PlusTabBar::mouseMoveEvent(event);

    event->accept();

    QDrag *drag = new QDrag(this);

    tabView()->createSplitOverlay(m_id, drag);

    QStyleOptionTabV3 tab;
    initStyleOption(&tab, idx);

    QWidget *tabWidget = ((QTabWidget*)parent())->widget(idx);
    QPixmap wMap(tabWidget->size());
    tabWidget->render(&wMap);

    if(wMap.width() > 400 || wMap.height() > 400)
        wMap = wMap.scaled(400, 400, Qt::KeepAspectRatio,
                           sConfig.get(CFG_BOOL_SMOOTH_SCALING) ? Qt::SmoothTransformation : Qt::FastTransformation);

    QSize size = tabRect(idx).size();
    size.rwidth() = std::max(wMap.width(), size.width());
    size.rheight() += wMap.height();

    QPixmap map(size);
    map.fill(Qt::transparent);

    QStylePainter p(&map, this);
    p.initFrom(this);
    p.drawItemPixmap(QRect(0, tab.rect.height()-5, wMap.width(), wMap.height()), 0, wMap);

    tab.rect.moveTopLeft(QPoint(0, 0));
    p.drawControl(QStyle::CE_TabBarTab, tab);
    p.end();

    QMimeData *mime = new QMimeData();
    mime->setData("data/tabinfo", QString("%1 %2 %3").arg(m_id).arg(idx).arg(tabView()->getWindowId()).toLatin1());

    drag->setPixmap(map);
    drag->setMimeData(mime);
    drag->exec();
    delete drag;
}

void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    if(!event->source() || !event->mimeData()->hasFormat("data/tabinfo"))
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

    QStringList lst = QString::fromLatin1(event->mimeData()->data("data/tabinfo")).split(' ');
    int tabId = lst[1].toInt();

    if(event->source() != this)
    {
        MainWindow *window = sWorkTabMgr.getWindow(lst[2].toUInt());
        if(!window)
            return;
        TabWidget *source = window->getTabView()->getWidget(lst[0].toUInt());
        emit pullTab(tabId, source, m_drag_idx);
    }
    else
        moveTab(tabId, tabId < m_drag_idx ? m_drag_idx -1 : m_drag_idx);

    if(m_drag_insert)
        m_drag_insert->hide();
}

void TabBar::dragLeaveEvent(QDragLeaveEvent *event)
{
    PlusTabBar::dragLeaveEvent(event);
    if(m_drag_insert)
        m_drag_insert->hide();
}

void TabBar::updateDropMarker(const QPoint& pos)
{
    int idx = tabAt(pos);
    if(idx == -1)
    {
        m_drag_idx = -1;
        if(m_drag_insert)
            m_drag_insert->hide();
        return;
    }

    const int ARROW_SIZE = 32;
    if(!m_drag_insert)
    {
        m_drag_insert = new QLabel(parentWidget());
        m_drag_insert->resize(ARROW_SIZE, ARROW_SIZE);
        m_drag_insert->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_drag_insert->setPixmap(QIcon(":/icons/arrow-down").pixmap(ARROW_SIZE, ARROW_SIZE));
    }

    QPoint p;
    QRect rect = tabRect(idx);
    if(pos.x() - rect.left() >= rect.width()/2)
    {
        m_drag_idx = idx+1;
        p = mapToParent(rect.topRight());
    }
    else
    {
        m_drag_idx = idx;
        p = mapToParent(rect.topLeft());
    }
    m_drag_insert->move(p - QPoint(ARROW_SIZE/2, ARROW_SIZE/2));
    m_drag_insert->show();
}

void TabBar::renameTab()
{
    QString name = tabToolTip(m_cur_menu_tab);
    QString conStr;
    if(name.contains(" - "))
    {
        conStr = name.mid(name.lastIndexOf(" - "));
        name = name.left(name.lastIndexOf(" - "));
    }

    QString newName = QInputDialog::getText(this, tr("Rename tab"), tr("New name:"),
                                         QLineEdit::Normal, name);
    if(!newName.isEmpty())
    {
        newName.replace('-', '_');
        newName += conStr;

        setTabToolTip(m_cur_menu_tab, newName);

        if(newName.size() > 25)
        {
            newName.resize(25);
            newName += "...";
        }
        setTabText(m_cur_menu_tab, newName);
    }
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

void TabBar::toNewWindow()
{
    emit newWindow(m_cur_menu_tab);
}

TabSwitchWidget::TabSwitchWidget(QWidget *parent) : QFrame(parent), ui(new Ui::TabSwitchWidget)
{
    ui->setupUi(this);
    m_active = 0;

    setStyleSheet("background-color: white");

    const std::vector<quint32>& tab_ids = tabWidget()->getTabIds();
    for(size_t i = 0; i < tab_ids.size(); ++i)
        m_id_pair[tab_ids[i]] = i;

    m_buttons.reserve(tab_ids.size());

    const QList<quint32>& history = tabWidget()->getHistory();
    for(int i = history.size()-1; i >= 0; --i)
        createButton(m_id_pair[history[i]]);

    for(size_t i = 0; i < tab_ids.size(); ++i)
        if(!history.contains(tab_ids[i]))
            createButton(i);
}

TabSwitchWidget::~TabSwitchWidget()
{
    if(!m_buttons.empty())
    {
        int idx = m_buttons[m_active]->property("tabIndex").toInt();
        emit setIndex(idx);
    }
    delete ui;
}

void TabSwitchWidget::next()
{
    if(m_buttons.empty())
        return;

    m_buttons[m_active++]->setChecked(false);

    if((size_t)m_active >= m_buttons.size())
        m_active = 0;

    m_buttons[m_active]->setChecked(true);
    ui->scrollArea->ensureWidgetVisible(m_buttons[m_active]);

    int idx = m_buttons[m_active]->property("tabIndex").toInt();
    generatePreview(idx);
}

void TabSwitchWidget::createButton(int idx)
{
    QPushButton *btn = new QPushButton(tabWidget()->tabText(idx), this);
    btn->setFlat(true);
    btn->setCheckable(true);
    btn->setEnabled(false);
    btn->setProperty("tabIndex", idx);
    ui->scrollLayout->insertWidget(ui->scrollLayout->count()-1, btn);

    m_buttons.push_back(btn);
}

void TabSwitchWidget::generatePreview(int idx)
{
    QWidget *w = tabWidget()->widget(idx);
    QPixmap pixmap(w->size());
    w->render(&pixmap);
    pixmap = pixmap.scaled(ui->prevLabel->size(), Qt::KeepAspectRatio,
                           sConfig.get(CFG_BOOL_SMOOTH_SCALING) ? Qt::SmoothTransformation : Qt::FastTransformation);
    ui->prevLabel->setPixmap(pixmap);
}

AltClosableMenu::AltClosableMenu(QWidget *parent) : QMenu(parent)
{

}

void AltClosableMenu::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Alt)
    {
        close();
        event->accept();
        return;
    }
    QMenu::keyPressEvent(event);
}
