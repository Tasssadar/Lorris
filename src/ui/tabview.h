/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TABVIEW_H
#define TABVIEW_H

#include <QFrame>
#include <set>
#include <QHash>
#include <QLocale>
#include <QMenuBar>

#include "../misc/sessionmgr.h"
#include "tabwidget.h"
#include "resizeline.h"

#ifdef Q_OS_MAC
#include <QtMacExtras>
#endif

class QLayoutItem;
class QBoxLayout;
class TabViewResLine;
class SplitOverlay;
class QDrag;
class WorkTabInfo;
class DataFileParser;
class QLabel;
class MainWindow;

enum saveLayoutItem
{
    ITEM_WIDGET = 0, // used in old session files
    ITEM_LAYOUT_H,
    ITEM_LAYOUT_V,
    ITEM_SKIP,
    ITEM_WIDGET_WITH_PCT,
    ITEM_LAYOUT_STRETCH
};

class TabView : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void openHomeTab();
    void statusBarMsg(const QString& message, int timeout = 0);
    void closeWindow();
    void closeHomeTab();
    void changeWindowTitle(const QString &title);

public:
    explicit TabView(MainWindow *parent);

    TabWidget *getActiveWidget() const
    {
        Q_ASSERT(m_active_widget);
        return m_active_widget;
    }

    TabWidget *getWidget(quint32 id)
    {
        if(m_tab_widgets.contains(id))
            return m_tab_widgets[id];
        return NULL;
    }

    TabWidget *getWidgetWithTab(quint32 tabId);
    TabWidget *getWidgetWithWidget(QWidget *widget);

    QBoxLayout *getLayoutForLine(TabViewResLine *line);

    void createSplitOverlay(quint32 id, QDrag *drag);

    const std::vector<QAction*>& getMenus() const
    {
        return m_menus;
    }

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    quint32 getWindowId() const { return m_windowId; }

    bool canCloseWindow();
    void forceCloseChilds();

private slots:
    void split(int pos, int index);
    void removeWidget(quint32 id);
    void changeActiveWidget(TabWidget *widget);
    void NewSpecificTab();
    void OpenConnectionManager();
    void newTab();
    void showSettings();
    void checkForUpdate();
    void checkChangeWindowTitle(const QString& title);
    void closeAllTabs();

private:
    TabWidget *newTabWidget(QBoxLayout *l, bool addAsNextToLast = false);
    void updateSize(QBoxLayout *l);
    inline bool isResizeLine(QLayoutItem *item);
    void updateResizeLines(QBoxLayout *l);
    inline void newResizeLine(QBoxLayout *l, int idx);

    inline QBoxLayout *getLayoutForWidget(QWidget *widget);
    inline void removeEmptyLayouts();
    inline QBoxLayout *newLayout(bool ver);

    void writeLayoutStructure(DataFileParser *file, QLayout *l);
    void loadLayoutStructure(DataFileParser *file, QBoxLayout *parent, QHash<quint32, quint32>& id_pair);

    QHash<quint32, TabWidget*> m_tab_widgets;
    QHash<TabViewResLine*, QBoxLayout*> m_resize_lines;

    std::set<QBoxLayout*> m_layouts;

    TabWidget *m_active_widget;
    std::vector<QAction*> m_lang_menu;
    std::vector<QAction*> m_menus;

    quint32 m_windowId;
    bool m_blockActive;

    QHash<QObject *, WorkTabInfo *> m_actionTabInfoMap;

#ifdef Q_OS_MAC
    QMenuBar *m_menuBar;
    QMacToolBar *macBar;
#endif
};

class TabViewResLine : public ResizeLine
{
    Q_OBJECT
public:
    TabViewResLine(bool vertical, TabView *parent);

    void setResizeLayout(QBoxLayout *) { }

protected:
    QBoxLayout *getLayout();

private:
    TabView *m_tab_view;
};

class SplitOverlay : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void split(int pos, int index);
    void newWindow(int index);

public:
    enum position
    {
        POS_RIGHT = 0,
        POS_LEFT,
        POS_BOTTOM,
        POS_TOP,
        POS_CENTER,
        POS_SPLIT_MAX = POS_CENTER,

        POS_MAX
    };

    SplitOverlay(position pos, TabWidget *tab, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    void showAt(TabWidget *tab);

    position m_pos;
    bool m_hover;
};

#endif // TABVIEW_H
