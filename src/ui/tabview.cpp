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

#include "tabview.h"
#include "homedialog.h"
#include "chooseconnectiondlg.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../misc/datafileparser.h"
#include "tooltipwarn.h"

#ifdef Q_OS_WIN
 #include "../misc/updater.h"
#endif

#define LAYOUT_MARGIN 4

QLocale::Language langs[] = { QLocale::system().language(), QLocale::English, QLocale::Czech };

TabView::TabView(QWidget *parent) :
    QWidget(parent), m_active_widget(NULL), m_session_mgr(this)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    m_layouts.insert(layout);
    layout->setMargin(LAYOUT_MARGIN);

    m_active_widget = newTabWidget(layout);

    QMenu *file_menu = new QMenu(tr("&File"), this);
    QMenu *session_menu = new QMenu(tr("&Sessions"), this);
    QMenu *help_menu = new QMenu(tr("&Help"), this);

    m_menus.push_back(file_menu);
    m_menus.push_back(session_menu);
    m_menus.push_back(help_menu);

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

    QAction* actionConnectionManager = file_menu->addAction(tr("Connection &manager..."));
    QAction* actionQuit = file_menu->addAction(tr("&Quit"));

    m_session_mgr.initMenu(session_menu);

    QMenu* menuLang = help_menu->addMenu(tr("Language"));

    QSignalMapper *langSignals = new QSignalMapper(this);
    connect(langSignals, SIGNAL(mapped(int)), this, SLOT(langChanged(int)));

    for(quint8 i = 0; i < 3; ++i)
    {
        QString langName = QLocale::languageToString(langs[i]);
        if(i == 0)
            langName.prepend(tr("Same as OS - "));

        QAction* actLang = menuLang->addAction(langName);
        actLang->setCheckable(true);
        m_lang_menu.push_back(actLang);

        langSignals->setMapping(actLang, i);
        connect(actLang, SIGNAL(triggered()), langSignals, SLOT(map()));

        if(i == 0)
            menuLang->addSeparator();
    }
    QAction* checkUpdate = help_menu->addAction(tr("Check for update"));
    QAction* actionAbout = help_menu->addAction(tr("About Lorris..."));

    quint32 curLang = sConfig.get(CFG_QUINT32_LANGUAGE);
    if(curLang >= m_lang_menu.size())
        curLang = 0;
    m_lang_menu[curLang]->setChecked(true);

    actionQuit->setShortcut(QKeySequence("Alt+F4"));

    connect(actionQuit,     SIGNAL(triggered()), this, SIGNAL(closeLorris()));
    connect(actionAbout,    SIGNAL(triggered()), this, SLOT(About()));
    connect(actionConnectionManager, SIGNAL(triggered()), this, SLOT(OpenConnectionManager()));
    connect(checkUpdate,    SIGNAL(triggered()), this, SLOT(checkForUpdate()));
}

TabWidget *TabView::newTabWidget(QBoxLayout *l)
{
    quint32 id = sWorkTabMgr.generateNewWidgetId();
    if(m_tab_widgets.empty())
        id = 0;

    TabWidget *tabW = new TabWidget(id, this);
    m_tab_widgets.insert(id, tabW);

    l->addWidget(tabW, 50);

    if(!m_active_widget)
        m_active_widget = tabW;

    connect(tabW, SIGNAL(newTab()),                       SLOT(newTab()));
    connect(tabW, SIGNAL(openHomeTab(quint32)),           SIGNAL(openHomeTab(quint32)));
    connect(tabW, SIGNAL(statusBarMsg(QString,int)),      SIGNAL(statusBarMsg(QString,int)));
    connect(tabW, SIGNAL(split(bool,int)),                SLOT(split(bool,int)));
    connect(tabW, SIGNAL(removeWidget(quint32)),          SLOT(removeWidget(quint32)));
    connect(tabW, SIGNAL(changeActiveWidget(TabWidget*)), SLOT(changeActiveWidget(TabWidget*)));

    updateResizeLines((QBoxLayout*)layout());
    return tabW;
}

void TabView::changeActiveWidget(TabWidget *widget)
{
    m_active_widget = widget;
}

void TabView::removeWidget(quint32 id)
{
    QHash<quint32, TabWidget*>::iterator wid = m_tab_widgets.find(id);
    if(wid == m_tab_widgets.end())
        return;

    if(m_active_widget == *wid)
        m_active_widget = m_tab_widgets[0];

    if(QBoxLayout *l = getLayoutForWidget(*wid))
        l->removeWidget(*wid);

    (*wid)->deleteLater();
    m_tab_widgets.erase(wid);

    removeEmptyLayouts();
    updateResizeLines((QBoxLayout*)layout());
}

void TabView::split(bool horizontal, int index)
{
    Q_ASSERT(sender());

    TabWidget *widget = (TabWidget*)sender();
    QBoxLayout *l = getLayoutForWidget(widget);
    if(!l)
        return;

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

    TabWidget *second = newTabWidget(l);
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
            m_resize_lines.remove((ResizeLine*)curItem->widget());
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
    ResizeLine *line = new ResizeLine(l->inherits("QHBoxLayout"), this);
    l->insertWidget(idx, line, 1);
    m_resize_lines.insert(line, l);
    line->updateStretch();
}

bool TabView::isResizeLine(QLayoutItem *item)
{
    return (item && item->widget() && item->widget()->inherits("ResizeLine"));
}

QBoxLayout *TabView::getLayoutForLine(ResizeLine *line)
{
    QHash<ResizeLine*, QBoxLayout*>::iterator itr = m_resize_lines.find(line);
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

    SplitOverlay *overlay = new SplitOverlay(SplitOverlay::POS_RIGHT, this);
    connect(overlay, SIGNAL(split(bool,int)), tab, SIGNAL(split(bool,int)));
    connect(drag,    SIGNAL(destroyed()), overlay, SLOT(deleteLater()));

    QPoint pos;
    pos.rx() = (tab->x() + tab->width()) - overlay->width() - 15;
    pos.ry() = tab->y() + (tab->height() - overlay->height())/2;

    overlay->move(pos);
    overlay->show();

    overlay = new SplitOverlay(SplitOverlay::POS_BOTTOM, this);
    connect(overlay, SIGNAL(split(bool,int)), tab, SIGNAL(split(bool,int)));
    connect(drag,    SIGNAL(destroyed()), overlay, SLOT(deleteLater()));

    pos.rx() = tab->x() + (tab->width() - overlay->width())/2;
    pos.ry() = tab->y() + tab->height() - overlay->height() - 15;

    overlay->move(pos);
    overlay->show();
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

void TabView::langChanged(int idx)
{
    sConfig.set(CFG_QUINT32_LANGUAGE, idx);
    for(quint8 i = 0; i < m_lang_menu.size(); ++i)
        m_lang_menu[i]->setChecked(i == idx);

    QMessageBox box(this);
    box.setWindowTitle(tr("Restart"));
    box.setText(tr("You need to restart Lorris for this change to take effect"));
    box.setIcon(QMessageBox::Information);
    box.exec();
}

void TabView::NewSpecificTab()
{
    WorkTabInfo * info = m_actionTabInfoMap.value(this->sender());
    if (info)
        sWorkTabMgr.AddWorkTab(info);
}


void TabView::OpenConnectionManager()
{
    ChooseConnectionDlg dialog(0, this);
    dialog.exec();
}

void TabView::About()
{
    QString text = tr("Lorris version " VERSION);
    if(text.contains("-dev"))
        text += ", git revision " + QString::number(REVISION);

    QMessageBox box(this);
    box.setWindowTitle(tr("About Lorris"));
    box.setText(text);
    box.setIcon(QMessageBox::Information);
    box.exec();
}

void TabView::newTab()
{
    HomeDialog dialog(this);
    dialog.exec();
}

void TabView::checkForUpdate()
{
#ifdef Q_OS_WIN
    Utils::printToStatusBar(tr("Checking for update..."), 0);
    if(Updater::doUpdate(false))
        emit closeLorris();
    else
        new ToolTipWarn(tr("No update available"), (QWidget*)sender(), this);
#else
    Utils::ThrowException(QObject::tr("Update feature is available on Windows only, you have to rebuild Lorris by yourself.\n"
                                      "<a href='http://tasssadar.github.com/Lorris'>http://tasssadar.github.com/Lorris</a>"));
#endif
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
    file->writeVal(l->count());
    for(int i = 0; i < l->count(); ++i)
    {
        QLayoutItem *item = l->itemAt(i);
        if(item->layout())
            writeLayoutStructure(file, item->layout());
        else if(!isResizeLine(item) && item->widget())
        {
            file->writeVal((quint8)ITEM_WIDGET);
            file->writeVal( ((TabWidget*)item->widget())->getId() );
        }
        else
            file->writeVal((quint8)ITEM_SKIP);
    }
}

void TabView::loadData(DataFileParser *file)
{
    // close all tabs
    QHash<quint32, TabWidget*> tabs = m_tab_widgets;
    for(QHash<quint32, TabWidget*>::iterator itr = tabs.begin(); itr != tabs.end(); ++itr)
    {
        while(!(*itr)->isEmpty())
            (*itr)->closeTab((*itr)->count()-1);
    }

    if(!file->seekToNextBlock("tabViewLayouts", 0))
        return;

    sWorkTabMgr.CloseHomeTab();
    delete m_tab_widgets[0];

    quint8 type = 0;
    file->readVal(type);

    delete layout();

    m_tab_widgets.clear();
    m_layouts.clear();
    m_active_widget = NULL;

    QBoxLayout *l = newLayout(type == ITEM_LAYOUT_V);
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
}

void TabView::loadLayoutStructure(DataFileParser *file, QBoxLayout *parent, QHash<quint32, quint32>& id_pair)
{
    int count = 0;
    file->readVal(count);

    for(int i = 0; i < count; ++i)
    {
        quint8 type = 0;
        file->readVal(type);
        switch(type)
        {
            case ITEM_LAYOUT_H:
            case ITEM_LAYOUT_V:
            {
                QBoxLayout *l = newLayout(type == ITEM_LAYOUT_V);
                parent->addLayout(l, 50);
                loadLayoutStructure(file, l, id_pair);
                break;
            }
            case ITEM_WIDGET:
            {
                quint32 new_id = newTabWidget(parent)->getId();
                quint32 load_id = 0;
                file->readVal(load_id);
                id_pair.insert(load_id, new_id);
                break;
            }
            case ITEM_SKIP:
                break;
        }
    }
}

ResizeLine::ResizeLine(bool vertical, TabView *parent) : QFrame(parent)
{
    m_vertical = vertical;
    m_cur_stretch = 50;
    m_tab_view = parent;
    m_resize_layout = NULL;

    setFrameStyle((vertical ? QFrame::VLine : QFrame::HLine) | QFrame::Plain);

    if(vertical)
        setSizeIncrement(QSizePolicy::Fixed, QSizePolicy::Expanding);
    else
        setSizeIncrement(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setCursor(vertical ? Qt::SizeHorCursor : Qt::SizeVerCursor);
}

void ResizeLine::updateStretch()
{
    QBoxLayout *l= m_tab_view->getLayoutForLine(this);
    Q_ASSERT(l);

    if(l)
    {
        int index = l->indexOf(this);
        if(index < 1)
        {
            Q_ASSERT(false);
            return;
        }
        m_cur_stretch = l->stretch(index-1);
    }
}

void ResizeLine::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return QFrame::mousePressEvent(event);

    event->accept();

    m_resize_layout = m_tab_view->getLayoutForLine(this);
    Q_ASSERT(m_resize_layout);

    m_mouse_pos = event->globalPos();

    if(m_resize_layout)
    {
        int index = m_resize_layout->indexOf(this);
        if(index < 1)
        {
            Q_ASSERT(false);
            m_resize_layout = NULL;
            return;
        }

        m_resize_index = index;

        QLayoutItem *item = m_resize_layout->itemAt(index-1);
        if(item->widget()) m_resize_pos[0] = item->widget()->pos();
        else               m_resize_pos[0] = item->layout()->geometry().topLeft();

        item = m_resize_layout->itemAt(index+1);
        if(!item)
            return;

        if(item->widget())
        {
            m_resize_pos[1] = item->widget()->pos();
            m_resize_pos[1].rx() += item->widget()->width();
            m_resize_pos[1].ry() += item->widget()->height();
        }
        else
            m_resize_pos[1] = item->layout()->geometry().bottomRight();
    }
}

void ResizeLine::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return QFrame::mouseReleaseEvent(event);

    event->accept();

    m_resize_layout = NULL;
}

void ResizeLine::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_resize_layout)
        return QFrame::mouseMoveEvent(event);

    event->accept();

    QPoint mouse = event->globalPos();

    float dist, move;
    if(m_vertical)
    {
        move = (mouse - m_mouse_pos).x();
        dist = (m_resize_pos[1] - m_resize_pos[0]).x();
    }
    else
    {
        move = (mouse - m_mouse_pos).y();
        dist = (m_resize_pos[1] - m_resize_pos[0]).y();
    }

    m_cur_stretch += move / (dist / 100.f);

    if(m_cur_stretch > 100)    m_cur_stretch = 100;
    else if(m_cur_stretch < 0) m_cur_stretch = 0;

    m_resize_layout->setStretch(m_resize_index-1, (int)m_cur_stretch);
    m_resize_layout->setStretch(m_resize_index+1, 100 - (int)m_cur_stretch);

    m_mouse_pos = event->globalPos();
}

#define OVERLAY_1 40
#define OVERLAY_2 150

SplitOverlay::SplitOverlay(position pos, QWidget *parent) : QWidget(parent)
{
    m_pos = pos;
    m_hover = false;

    setPalette(Qt::transparent);
    setAcceptDrops(true);

    QFont fnt = font();
    fnt.setPointSize(10);
    fnt.setBold(true);
    setFont(fnt);

    if(pos == POS_RIGHT)
        resize(OVERLAY_1, OVERLAY_2);
    else
        resize(OVERLAY_2, OVERLAY_1);
}

void SplitOverlay::paintEvent(QPaintEvent *)
{
    static const QPoint poly[POS_MAX][5] =
    {
        {
            QPoint(0, 0), QPoint(OVERLAY_1/2, 0), QPoint(OVERLAY_1, OVERLAY_2/2),
            QPoint(OVERLAY_1/2, OVERLAY_2), QPoint(0, OVERLAY_2)
        },
        {
            QPoint(0, 0), QPoint(OVERLAY_2, 0), QPoint(OVERLAY_2, OVERLAY_1/2),
            QPoint(OVERLAY_2/2, OVERLAY_1), QPoint(0, OVERLAY_1/2)
        }
    };

    QPainter p(this);
    p.setPen(m_hover ? Qt::yellow : Qt::red);
    p.setBrush(QBrush(m_hover ? Qt::yellow : Qt::red, Qt::SolidPattern));
    p.drawPolygon(poly[m_pos], 5);

    p.setPen(Qt::black);
    if(m_pos == POS_RIGHT)
    {
        p.rotate(-90);
        p.translate(-height(), 0);
        p.drawText(0, 0, height(), width(), Qt::AlignCenter, tr("Split"));
    }
    else
        p.drawText(0, 0, width(), height(), Qt::AlignCenter, tr("Split"));
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

    QStringList lst = event->mimeData()->text().split(' ');
    emit split(m_pos == POS_BOTTOM, lst[1].toInt());
}
