/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalMapper>
#include <QMessageBox>
#include <stdio.h>
#include <QLabel>
#include <QDrag>
#include <QMimeData>
#include <QToolBar>
#include <QMenuBar>

#include "tabview.h"
#include "homedialog.h"
#include "chooseconnectiondlg.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../misc/datafileparser.h"
#include "tooltipwarn.h"
#include "settingsdialog.h"
#include "mainwindow.h"
#include "../connection/connectionmgr2.h"

#ifdef Q_OS_WIN
#include "../misc/updater.h"
#endif


#ifndef Q_OS_MAC
#define LAYOUT_MARGIN 4
#else
#include <QtMacExtras>
    void qt_mac_set_dock_menu(QMenu *menu);
#define LAYOUT_MARGIN 0
#endif


TabView::TabView(MainWindow *parent) :
    QWidget(parent), m_active_widget(NULL)
{
    m_windowId = parent->getId();
    m_blockActive = false;

    QHBoxLayout *layout = new QHBoxLayout(this);
    m_layouts.insert(layout);
    layout->setMargin(LAYOUT_MARGIN);

    m_active_widget = newTabWidget(layout);
    QMenu *file_menu = new QMenu(tr("&File"), this);
    QMenu *session_menu = new QMenu(tr("&Sessions"), this);
    QMenu *opt_menu = new QMenu(tr("&Options"), this);
    QAction *connectAll = new QAction(tr("&Open all connections"), this);
    QAction *disconnectAll = new QAction(tr("&Close all connections"), this);
    m_menus.push_back(file_menu->menuAction());
    m_menus.push_back(session_menu->menuAction());
    m_menus.push_back(opt_menu->menuAction());
    m_menus.push_back(connectAll);
    m_menus.push_back(disconnectAll);

    QMenu * menuFileNew = file_menu->addMenu(tr("&New"));
    {
        WorkTabMgr::InfoList const & infos = sWorkTabMgr.GetWorkTabInfos();
        for (int i = 0; i < infos.size(); ++i)
        {
            WorkTabInfo * info = infos[i];
            QAction * action = menuFileNew->addAction(info->GetName());
            m_actionTabInfoMap[action] = info;
            connect(action, SIGNAL(triggered()), this, SLOT(NewSpecificTab()));
            menuFileNew->addAction(action);
        }
    }

    QAction *newW = file_menu->addAction(tr("New window"));
    newW->setShortcut(QKeySequence("Ctrl+N"));

    QAction* actionConnectionManager = file_menu->addAction(tr("Connection &manager..."));
    QAction* actCloseAll = file_menu->addAction(tr("Close &all tabs"));

    sWorkTabMgr.getSessionMgr()->initMenu(session_menu);

    QAction *settingsAct = opt_menu->addAction(tr("&Settings"));
    QAction *updateAct = opt_menu->addAction(tr("Check for update..."));

    connect(actionConnectionManager, SIGNAL(triggered()), SLOT(OpenConnectionManager()));
    connect(settingsAct,             SIGNAL(triggered()), SLOT(showSettings()));
    connect(updateAct,               SIGNAL(triggered()), SLOT(checkForUpdate()));
    connect(newW,                    SIGNAL(triggered()), &sWorkTabMgr, SLOT(newWindow()));
    connect(actCloseAll,             SIGNAL(triggered()), SLOT(closeAllTabs()));
    connect(connectAll,              SIGNAL(triggered()), &sConMgr2, SLOT(connectAll()));
    connect(disconnectAll,           SIGNAL(triggered()), &sConMgr2, SLOT(disconnectAll()));

#ifndef Q_OS_MAC
    QAction* actionQuit = file_menu->addAction(tr("&Close"));
    actionQuit->setShortcut(QKeySequence("Alt+F4"));
    connect(actionQuit,              SIGNAL(triggered()), SIGNAL(closeWindow()));
#else
    m_menuBar = new QMenuBar(0);
    m_menuBar->addMenu(file_menu);
    m_menuBar->addMenu(session_menu);
    m_menuBar->addMenu(opt_menu);

    qt_mac_set_dock_menu(file_menu);
    parent->setMenuBar(m_menuBar);

    file_menu->addSeparator();
    file_menu->addAction(connectAll);
    file_menu->addAction(disconnectAll);
#endif
}

TabWidget *TabView::newTabWidget(QBoxLayout *l, bool addAsNextToLast)
{
    quint32 id = sWorkTabMgr.generateNewWidgetId();

    TabWidget *tabW = new TabWidget(id, this);
    m_tab_widgets.insert(id, tabW);

    if(!addAsNextToLast)
        l->addWidget(tabW, 50);
    else
    {
        int index = std::max(0, l->count()-1);
        l->insertWidget(index, tabW, 50);
    }

    if(!m_active_widget)
        m_active_widget = tabW;

    connect(tabW, SIGNAL(newTab()),                       SLOT(newTab()));
    connect(tabW, SIGNAL(statusBarMsg(QString,int)),      SIGNAL(statusBarMsg(QString,int)));
    connect(tabW, SIGNAL(closeHomeTab()),                 SIGNAL(closeHomeTab()));
    connect(tabW, SIGNAL(split(int,int)),                 SLOT(split(int,int)));
    connect(tabW, SIGNAL(removeWidget(quint32)),          SLOT(removeWidget(quint32)));
    connect(tabW, SIGNAL(changeActiveWidget(TabWidget*)), SLOT(changeActiveWidget(TabWidget*)));
    connect(tabW, SIGNAL(changeWindowTitle(QString)),     SLOT(checkChangeWindowTitle(QString)));

    updateResizeLines((QBoxLayout*)layout());
    return tabW;
}

void TabView::changeActiveWidget(TabWidget *widget)
{
    if(m_blockActive)
        return;

    m_active_widget = widget;
    if(widget && !widget->isEmpty())
        emit changeWindowTitle(widget->tabToolTip(widget->currentIndex()));
}

void TabView::removeWidget(quint32 id)
{
    QHash<quint32, TabWidget*>::iterator wid = m_tab_widgets.find(id);
    if(wid == m_tab_widgets.end())
        return;

    if(m_tab_widgets.size() == 1)
    {
        if(sWorkTabMgr.canCloseWindow())
        {
            emit closeWindow();
            sWorkTabMgr.removeWindow(m_windowId);
        }
        else
        {
            emit openHomeTab();
            (*wid)->setTabsClosable(false);
        }
        return;
    }

    for(QHash<quint32, TabWidget*>::iterator i = m_tab_widgets.begin(); m_active_widget == *wid && i != m_tab_widgets.end(); ++i)
        m_active_widget = *i;

    if(QBoxLayout *l = getLayoutForWidget(*wid))
        l->removeWidget(*wid);

    (*wid)->deleteLater();
    m_tab_widgets.erase(wid);

    removeEmptyLayouts();
    updateResizeLines((QBoxLayout*)layout());
}

void TabView::split(int pos, int index)
{
    Q_ASSERT(sender());

    TabWidget *widget = (TabWidget*)sender();
    QBoxLayout *l = getLayoutForWidget(widget);
    if(!l)
        return;

    bool horizontal = (pos == SplitOverlay::POS_BOTTOM || pos == SplitOverlay::POS_TOP);
    bool switched = (pos == SplitOverlay::POS_LEFT || pos == SplitOverlay::POS_TOP);

    if(horizontal ^ l->inherits("QVBoxLayout"))
    {
        if(l->count() == 1)
        {
            bool setAsMain = (layout() == l);
            QBoxLayout *parentL = (QBoxLayout*)l->parent();

            int idx = -1;
            for(int i = 0; !setAsMain && idx == -1 && i < parentL->count(); ++i)
                if(parentL->itemAt(i)->layout() == l)
                    idx = i;

            m_layouts.erase(l);
            delete l;

            l = newLayout(horizontal);
            if(setAsMain) setLayout(l);
            else          parentL->insertLayout(idx, l, 50);

            l->setMargin(setAsMain ? LAYOUT_MARGIN : 0);
            l->addWidget(widget, 50);
        }
        else
        {
            int pos = l->indexOf(widget);
            l->removeWidget(widget);

            QBoxLayout *newL = newLayout(horizontal);
            newL->setMargin(0);
            newL->addWidget(widget, 50);
            l->insertLayout(pos, newL, 50);

            l = newL;
        }
    }

    TabWidget *second = newTabWidget(l, switched);
    second->pullTab(index, widget);

    updateResizeLines((QBoxLayout*)layout());
}

void TabView::updateResizeLines(QBoxLayout *l)
{
    QLayoutItem *prevItem = NULL;
    QLayoutItem *curItem = NULL;
    int count = l->count();
    for(int i = 0; i < count; ++i)
    {
        curItem = l->itemAt(i);

        if(curItem->layout())
            updateResizeLines((QBoxLayout*)curItem->layout());

        // Remove ResizeLine if there are two in a row or if it is the first or last item
        if(isResizeLine(curItem) && (!prevItem || i+1 >= count || isResizeLine(prevItem)))
        {
            m_resize_lines.remove((TabViewResLine*)curItem->widget());
            delete curItem->widget();

            if(!l->count())
            {
                m_layouts.erase(l);
                delete l;
                return;
            }

            goto restart_loop;
        }

        if(prevItem && !isResizeLine(curItem) && !isResizeLine(prevItem))
        {
            newResizeLine(l, i);
            goto restart_loop;
        }

        prevItem = curItem;
        continue;

restart_loop:
        i = -1;
        prevItem = curItem = NULL;
        count = l->count();
    }
}

void TabView::newResizeLine(QBoxLayout *l, int idx)
{
    TabViewResLine *line = new TabViewResLine(l->inherits("QHBoxLayout"), this);
    l->insertWidget(idx, line, 1);
    m_resize_lines.insert(line, l);
    line->updateStretch();
}

bool TabView::isResizeLine(QLayoutItem *item)
{
    return (item && item->widget() && item->widget()->inherits("TabViewResLine"));
}

QBoxLayout *TabView::getLayoutForLine(TabViewResLine *line)
{
    QHash<TabViewResLine*, QBoxLayout*>::iterator itr = m_resize_lines.find(line);
    if(itr != m_resize_lines.end())
        return *itr;
    return NULL;
}

void TabView::createSplitOverlay(quint32 id, QDrag *drag)
{
    QHash<quint32, TabWidget*>::iterator itr = m_tab_widgets.find(id);
    if(itr == m_tab_widgets.end())
        return;

    TabWidget *tab = *itr;
    SplitOverlay *overlay;

    // Center - new window
    overlay = new SplitOverlay(SplitOverlay::POS_CENTER, tab, this);
    connect(overlay, SIGNAL(newWindow(int)), tab,     SLOT(newWindow(int)), Qt::QueuedConnection);
    connect(drag,    SIGNAL(destroyed()),    overlay, SLOT(deleteLater()));

    if(tab->count() < 2)
        return;

    // splits
    for(int i = 0; i < SplitOverlay::POS_SPLIT_MAX; ++i)
    {
        overlay = new SplitOverlay((SplitOverlay::position)i, tab, this);
        connect(overlay, SIGNAL(split(int,int)),  tab,     SIGNAL(split(int,int)));
        connect(drag,    SIGNAL(destroyed()),     overlay, SLOT(deleteLater()));
    }
}

QBoxLayout *TabView::getLayoutForWidget(QWidget *widget)
{
    for(std::set<QBoxLayout*>::iterator i = m_layouts.begin(); i != m_layouts.end(); ++i)
        if((*i)->indexOf(widget) != -1)
            return *i;
    return NULL;
}

void TabView::removeEmptyLayouts()
{
    for(std::set<QBoxLayout*>::iterator i = m_layouts.begin(); i != m_layouts.end();)
    {
        // DAMNYOUQT: aparently, when you have layout
        // with non-QWidget item inside, it is empty.
        if(!(*i)->count())
        {
            delete *i;
            m_layouts.erase(i);
            i = m_layouts.begin();
        }
        else
            ++i;
    }
}

QBoxLayout *TabView::newLayout(bool ver)
{
    QBoxLayout *l = ver ? (QBoxLayout*)new QVBoxLayout : (QBoxLayout*)new QHBoxLayout;
    m_layouts.insert(l);
    return l;
}

void TabView::NewSpecificTab()
{
    WorkTabInfo * info = m_actionTabInfoMap.value(this->sender());
    if (info)
        sWorkTabMgr.AddWorkTab(info, m_windowId);
}

void TabView::OpenConnectionManager()
{
    ChooseConnectionDlg dialog(this);
    dialog.exec();
}

void TabView::newTab()
{
    m_blockActive = true;
    HomeDialog dialog(getWindowId(), this);
    dialog.exec();
    m_blockActive = false;
}

void TabView::saveData(DataFileParser *file)
{
    file->writeBlockIdentifier("tabViewLayouts");
    writeLayoutStructure(file, layout());

    file->writeBlockIdentifier("tabViewWidgets");
    file->writeVal(m_tab_widgets.count());

    for(QHash<quint32, TabWidget*>::iterator itr = m_tab_widgets.begin(); itr != m_tab_widgets.end(); ++itr)
    {
        file->writeBlockIdentifier("tabWidget");
        (*itr)->saveData(file);
    }
}

void TabView::writeLayoutStructure(DataFileParser *file, QLayout *l)
{
    file->writeVal(quint8(l->inherits("QHBoxLayout") ? ITEM_LAYOUT_H : ITEM_LAYOUT_V));

    int count = l->count();
    quint64 countPos = file->pos();
    file->writeVal(count); // placeholder

    for(int i = 0; i < l->count(); ++i)
    {
        QLayoutItem *item = l->itemAt(i);
        if(item->layout())
        {
            file->writeVal((quint8)ITEM_LAYOUT_STRETCH);
            file->writeVal(((QBoxLayout*)l)->stretch(i));
            ++count; // additional item for layout

            writeLayoutStructure(file, item->layout());
        }
        else if(!isResizeLine(item) && item->widget())
        {
            file->writeVal((quint8)ITEM_WIDGET_WITH_PCT);
            file->writeVal( ((TabWidget*)item->widget())->getId() );
            file->writeVal( ((QBoxLayout*)l)->stretch(i) );
        }
        else
            file->writeVal((quint8)ITEM_SKIP);
    }

    // real count
    file->writeVal(count, countPos);
}

void TabView::loadData(DataFileParser *file)
{
    // close all tabs
    QHash<quint32, TabWidget*> tabs = m_tab_widgets;
    for(QHash<quint32, TabWidget*>::iterator itr = tabs.begin(); itr != tabs.end(); ++itr)
    {
        while((*itr)->count() && (*itr)->tabsClosable())
            (*itr)->closeTab((*itr)->count()-1);
    }

    if(!file->seekToNextBlock("tabViewLayouts", 0))
        return;

    sWorkTabMgr.getWindow(m_windowId)->closeHomeTab();
    for(QList<quint32> keys = m_tab_widgets.keys(); !keys.isEmpty();)
        delete m_tab_widgets.take(keys.takeLast());

    quint8 type = 0;
    file->readVal(type);

    delete layout();

    m_tab_widgets.clear();
    m_layouts.clear();
    m_active_widget = NULL;

    QBoxLayout *l = newLayout(type == ITEM_LAYOUT_V);
    l->setMargin(LAYOUT_MARGIN);
    setLayout(l);

    QHash<quint32, quint32> id_pair;

    loadLayoutStructure(file, l, id_pair);

    if(!file->seekToNextBlock("tabViewWidgets", 0))
        return;

    int count = 0;
    file->readVal(count);

    for(int i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock("tabWidget", 0))
            break;

        quint32 id = 0;
        file->readVal(id);

        if(!id_pair.contains(id))
            continue;

        m_tab_widgets[id_pair[id]]->loadData(file);
    }

    if(m_tab_widgets.empty())
    {
        if(!layout())
        {
            QHBoxLayout *layout = new QHBoxLayout(this);
            m_layouts.insert(layout);
            layout->setMargin(LAYOUT_MARGIN);
        }
        m_active_widget = newTabWidget((QBoxLayout*)layout());
        removeEmptyLayouts();
        emit openHomeTab();
    }
}

void TabView::loadLayoutStructure(DataFileParser *file, QBoxLayout *parent, QHash<quint32, quint32>& id_pair)
{
    quint8 type;
    int stretch = 50;

    int count = file->readVal<int>();

    for(int i = 0; i < count; ++i)
    {
        type = file->readVal<quint8>();
        switch(type)
        {
            case ITEM_LAYOUT_STRETCH:
            {
                stretch = file->readVal<int>();
                break;
            }
            case ITEM_LAYOUT_H:
            case ITEM_LAYOUT_V:
            {
                QBoxLayout *l = newLayout(type == ITEM_LAYOUT_V);
                l->setMargin(0);
                parent->addLayout(l, stretch);
                loadLayoutStructure(file, l, id_pair);

                stretch = 50;
                break;
            }
            case ITEM_WIDGET:
            case ITEM_WIDGET_WITH_PCT:
            {
                quint32 new_id = newTabWidget(parent)->getId();
                quint32 load_id = 0;
                file->readVal(load_id);
                id_pair.insert(load_id, new_id);

                if(type == ITEM_WIDGET)
                    break;

                parent->setStretch(parent->count()-1, file->readVal<int>());
                break;
            }
            case ITEM_SKIP:
                break;
        }
    }
}

void TabView::showSettings()
{
    SettingsDialog d(this);
    connect(&d, SIGNAL(closeLorris()), qApp, SLOT(closeAllWindows()));
    d.exec();
}

void TabView::checkForUpdate()
{
#ifdef Q_OS_WIN
    Updater::checkForUpdate(false);
#else
    Utils::showErrorBox(tr("Update feature is available on Windows only, you have to rebuild Lorris by yourself.\n"
                             "<a href='http://tasssadar.github.com/Lorris'>http://tasssadar.github.com/Lorris</a>"));
#endif
}

TabWidget *TabView::getWidgetWithTab(quint32 tabId)
{
    for(QHash<quint32, TabWidget*>::iterator itr = m_tab_widgets.begin(); itr != m_tab_widgets.end(); ++itr)
        if((*itr)->containsTab(tabId))
            return *itr;

    return NULL;
}

// very clear name
TabWidget *TabView::getWidgetWithWidget(QWidget *widget)
{
    for(QHash<quint32, TabWidget*>::iterator itr = m_tab_widgets.begin(); itr != m_tab_widgets.end(); ++itr)
        if((*itr)->indexOf(widget) != -1)
            return *itr;
    return NULL;
}

void TabView::checkChangeWindowTitle(const QString &title)
{
    Q_ASSERT(sender());
    if(!sender())
        return;

    if((TabWidget*)sender() != m_active_widget)
        return;

    emit changeWindowTitle(title);
}

void TabView::closeAllTabs()
{
    int size = m_tab_widgets.size();
    for(QHash<quint32, TabWidget*>::iterator itr = m_tab_widgets.begin(); itr != m_tab_widgets.end(); )
    {
        while(size == m_tab_widgets.size() && (*itr)->count() && (*itr)->tabsClosable())
            if(!(*itr)->closeTab(0))
                return;

        if(size != m_tab_widgets.size())
        {
            itr = m_tab_widgets.begin();
            size = m_tab_widgets.size();
        }
        else
            ++itr;
    }
}

bool TabView::canCloseWindow()
{
    for(QHash<quint32, TabWidget*>::iterator itr = m_tab_widgets.begin(); itr != m_tab_widgets.end(); ++itr)
    {
        if(!(*itr)->canCloseTabs())
            return false;
    }
    return true;
}

void TabView::forceCloseChilds()
{
    for(QHash<quint32, TabWidget*>::iterator itr = m_tab_widgets.begin(); itr != m_tab_widgets.end(); ++itr)
        (*itr)->forceCloseChilds();
}

TabViewResLine::TabViewResLine(bool vertical, TabView *parent) : ResizeLine(vertical, parent)
{
    m_tab_view = parent;
}

QBoxLayout *TabViewResLine::getLayout()
{
    return m_tab_view->getLayoutForLine(this);
}

#define OVERLAY_1 40
#define OVERLAY_2 150

SplitOverlay::SplitOverlay(position pos, TabWidget *tab, QWidget *parent) : QWidget(parent)
{
    m_pos = pos;
    m_hover = false;

    setPalette(Qt::transparent);
    setAcceptDrops(true);

    QFont fnt = font();
    fnt.setPointSize(10);
    fnt.setBold(true);
    setFont(fnt);

    switch(pos)
    {
        case POS_RIGHT:
        case POS_LEFT:
            resize(OVERLAY_1, OVERLAY_2);
            break;
        case POS_BOTTOM:
        case POS_TOP:
        case POS_CENTER:
            resize(OVERLAY_2, OVERLAY_1);
            break;
        default: Q_ASSERT(false); break;
    }

    showAt(tab);
}

void SplitOverlay::paintEvent(QPaintEvent *)
{
    static const QPoint poly[POS_MAX][5] =
    {

        // POS_RIGHT
        {
            QPoint(0, 0), QPoint(OVERLAY_1/2, 0), QPoint(OVERLAY_1, OVERLAY_2/2),
            QPoint(OVERLAY_1/2, OVERLAY_2), QPoint(0, OVERLAY_2)
        },
        // POS_LEFT
        {
            QPoint(OVERLAY_1/2, 0), QPoint(OVERLAY_1, 0), QPoint(OVERLAY_1, OVERLAY_2),
            QPoint(OVERLAY_1/2, OVERLAY_2), QPoint(0, OVERLAY_2/2),
        },
        // POS_BOTTOM
        {
            QPoint(0, 0), QPoint(OVERLAY_2, 0), QPoint(OVERLAY_2, OVERLAY_1/2),
            QPoint(OVERLAY_2/2, OVERLAY_1), QPoint(0, OVERLAY_1/2)
        },
        // POS_TOP
        {
            QPoint(0, OVERLAY_1/2), QPoint(OVERLAY_2/2, 0), QPoint(OVERLAY_2, OVERLAY_1/2),
            QPoint(OVERLAY_2, OVERLAY_1), QPoint(0, OVERLAY_1)
        },
        // POS_CENTER
        {
            QPoint(0, 0), QPoint(OVERLAY_2, 0), QPoint(OVERLAY_2, OVERLAY_1),
            QPoint(OVERLAY_2, OVERLAY_1), QPoint(0, OVERLAY_1)
        }
    };

    QPainter p(this);
    p.setPen(m_hover ? Qt::yellow : Qt::red);
    p.setBrush(QBrush(m_hover ? Qt::yellow : Qt::red, Qt::SolidPattern));
    p.drawPolygon(poly[m_pos], 5);

    p.setPen(Qt::black);
    switch(m_pos)
    {
        case POS_RIGHT:
            p.rotate(-90);
            p.translate(-height(), 0);
            p.drawText(0, 0, height(), width(), Qt::AlignCenter, tr("Split"));
            break;
        case POS_LEFT:
            p.rotate(-90);
            p.translate(-height(), 5);
            p.drawText(0, 0, height(), width(), Qt::AlignCenter, tr("Split"));
            break;
        case POS_BOTTOM:
        case POS_TOP:
            p.drawText(0, 0, width(), height(), Qt::AlignCenter, tr("Split"));
            break;
        case POS_CENTER:
            p.drawText(0, 0, width(), height(), Qt::AlignCenter, tr("New window"));
            break;
        default: break;
    }
}

void SplitOverlay::dragEnterEvent(QDragEnterEvent *event)
{
    if(!event->source() || !event->mimeData()->hasFormat("data/tabinfo"))
        return QWidget::dragEnterEvent(event);

    event->acceptProposedAction();
    m_hover = true;
    update();
}

void SplitOverlay::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();

    m_hover = false;
    update();
}

void SplitOverlay::dropEvent(QDropEvent *event)
{
    event->accept();

    QStringList lst = QString::fromLatin1(event->mimeData()->data("data/tabinfo")).split(' ');
    switch(m_pos)
    {
        case POS_RIGHT:
        case POS_BOTTOM:
        case POS_LEFT:
        case POS_TOP:
            return emit split(m_pos, lst[1].toInt());
        case POS_CENTER:
            return emit newWindow(lst[1].toInt());
        default: break;
    }
}

void SplitOverlay::showAt(TabWidget *tab)
{
    int x,y;
    switch(m_pos)
    {
        case POS_RIGHT:
            x = (tab->x() + tab->width()) - width() - 15;
            y = tab->y() + (tab->height() - height())/2;
            break;
        case POS_LEFT:
            x = tab->x() + 15;
            y = tab->y() + (tab->height() - height())/2;
            break;
        case POS_BOTTOM:
            x = tab->x() + (tab->width() - width())/2;
            y = tab->y() + tab->height() - height() - 15;
            break;
        case POS_TOP:
            x = tab->x() + (tab->width() - width())/2;
            y = tab->y() + 15;
            break;
        case POS_CENTER:
            x = tab->x() + (tab->width() - width())/2;
            y = tab->y() + (tab->height() - height())/2;
            break;
        default: Q_ASSERT(false); return;
    }
    move(x, y);
    show();
}
