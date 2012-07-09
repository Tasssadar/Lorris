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

#include "../misc/sessionmgr.h"
#include "tabwidget.h"

class QLayoutItem;
class QBoxLayout;
class ResizeLine;
class SplitOverlay;
class QDrag;
class WorkTabInfo;
class DataFileParser;
class QLabel;

enum saveLayoutItem
{
    ITEM_WIDGET = 0,
    ITEM_LAYOUT_H,
    ITEM_LAYOUT_V,
    ITEM_SKIP
};

class TabView : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void openHomeTab(quint32 id);
    void statusBarMsg(const QString& message, int timeout = 0);
    void closeLorris();

public:
    explicit TabView(QWidget *parent = 0);

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

    QBoxLayout *getLayoutForLine(ResizeLine *line);
    void createSplitOverlay(quint32 id, QDrag *drag);

    const std::vector<QAction*>& getMenus() const
    {
        return m_menus;
    }

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    SessionMgr *getSessionMgr()
    {
        return &m_session_mgr;
    }

private slots:
    void split(bool horizontal, int index);
    void removeWidget(quint32 id);
    void changeActiveWidget(TabWidget *widget);
    void NewSpecificTab();
    void OpenConnectionManager();
    void newTab();
    void showSettings();
    void checkForUpdate();

private:
    TabWidget *newTabWidget(QBoxLayout *l);
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
    QHash<ResizeLine*, QBoxLayout*> m_resize_lines;

    std::set<QBoxLayout*> m_layouts;

    TabWidget *m_active_widget;
    std::vector<QAction*> m_lang_menu;

    std::vector<QAction*> m_menus;

    SessionMgr m_session_mgr;

    QHash<QObject *, WorkTabInfo *> m_actionTabInfoMap;
};

class ResizeLine : public QFrame
{
    Q_OBJECT
public:
    ResizeLine(bool vertical, TabView *parent);

    void updateStretch();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    void setPctLabel(const QPoint& p, int l, int r);

    bool m_vertical;
    float m_cur_stretch;
    TabView *m_tab_view;
    QBoxLayout *m_resize_layout;
    QPoint m_resize_pos[2];
    QPoint m_mouse_pos;
    int m_resize_index;
    QLabel *m_pct_label;
};

class SplitOverlay : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void split(bool horizontal, int index);

public:
    enum position
    {
        POS_RIGHT = 0,
        POS_BOTTOM,

        POS_MAX
    };

    SplitOverlay(position pos, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    position m_pos;
    bool m_hover;
};

#endif // TABVIEW_H
