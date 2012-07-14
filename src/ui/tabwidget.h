/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef MAINTABWIDGET_H
#define MAINTABWIDGET_H

#include <QTabWidget>
#include <QTabBar>
#include <QHash>
#include "plustabbar.h"

class QPushButton;
class QHBoxLayout;
class QMenu;
class TabBar;
class WorkTab;
class DataFileParser;

class TabWidget : public QTabWidget
{
    Q_OBJECT

Q_SIGNALS:
    void newTab();
    void openHomeTab(quint32 id);
    void removeWidget(quint32 id);
    void split(bool horizontal, int index);
    void changeActiveWidget(TabWidget *widget);
    void statusBarMsg(const QString& message, int timeout = 0);

public:
    explicit TabWidget(quint32 id, QWidget *parent = 0);

    quint32 getId() const { return m_id; }

    int addTab(QWidget *widget, const QString& name)
    {
        return QTabWidget::addTab(widget, name);
    }

    int addTab(WorkTab *widget, const QString& name, quint32 tabId);
    QWidget* unregisterTab(int index);

    virtual QSize sizeHint() const
    {
        return QSize(100, 100);
    }

    virtual QSize minimumSizeHint() const
    {
        return QSize(0, 0);
    }

    bool isEmpty()
    {
        return m_tab_ids.empty();
    }

    bool containsTab(quint32 id) const;

    void changeMenu(int idx);
    void clearMenu();
    void checkEmpty();

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    void addChildTab(QWidget *widget, const QString &name);
    void removeChildTab(QWidget *widget);

public slots:
    int pullTab(int index, TabWidget *origin);
    void pullTab(int index, TabWidget *origin, int to);
    void closeTab(int index);

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void tabMoved(int from, int to);
    void newTabBtn();
    void currentIndexChanged(int idx);
    void setConnString(const QString& str, bool hadConn);

private:
    bool checkEvent(QMouseEvent *event);
    void setTabNameAndTooltip(int idx, QString name);

    quint32 m_id;
    std::vector<quint32> m_tab_ids;
    TabBar *m_tab_bar;

    QPushButton *m_menuBtn;
    QMenu *m_menu;
};

class TabBar : public PlusTabBar
{
    Q_OBJECT

Q_SIGNALS:
    void split(bool horizontal, int index);
    void pullTab(int idx, TabWidget *origin, int to);

public:
    explicit TabBar(quint32 id, QWidget * parent = 0);

    void enableSplit(bool enable);

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void renameTab();
    void splitTop();
    void splitLeft();

private:
    void updateDropMarker(const QPoint& pos);

    int m_cur_menu_tab;
    QRect m_drag_insert;
    int m_drag_idx;

    quint32 m_id;
    QMenu *m_menu;
    QAction *m_newTopBottom;
    QAction *m_newLeftRight;
    QPoint m_startDragPos;
};

#endif // MAINTABWIDGET_H
