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

#include "tabwidget.h"

class QLayoutItem;
class QBoxLayout;
class ResizeLine;
class SplitOverlay;
class QDrag;
class WorkTabInfo;

extern QLocale::Language langs[];

class TabView : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void openHomeTab(quint32 id);
    void statusBarMsg(const QString& message, int timeout = 0);
    void close();

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

    QMenu *getFileMenu()
    {
        return m_file_menu;
    }

    QMenu *getHelpMenu()
    {
        return m_help_menu;
    }

private slots:
    void split(bool horizontal, int index);
    void removeWidget(quint32 id);
    void changeActiveWidget(TabWidget *widget);
    void langChanged(int idx);
    void NewSpecificTab();
    void OpenConnectionManager();
    void About();
    void newTab();


private:
    TabWidget *newTabWidget(QBoxLayout *l);
    void updateSize(QBoxLayout *l);
    inline bool isResizeLine(QLayoutItem *item);
    void updateResizeLines(QBoxLayout *l);
    inline void newResizeLine(QBoxLayout *l, int idx);

    inline QBoxLayout *getLayoutForWidget(QWidget *widget);
    inline void removeEmptyLayouts();
    inline QBoxLayout *newLayout(bool hor);

    QHash<quint32, TabWidget*> m_tab_widgets;
    QHash<ResizeLine*, QBoxLayout*> m_resize_lines;

    std::set<QBoxLayout*> m_layouts;

    TabWidget *m_active_widget;
    std::vector<QAction*> m_lang_menu;

    QMenu *m_file_menu;
    QMenu *m_help_menu;

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
    bool m_vertical;
    float m_cur_stretch;
    TabView *m_tab_view;
    QBoxLayout *m_resize_layout;
    QPoint m_resize_pos[2];
    QPoint m_mouse_pos;
    int m_resize_index;
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
