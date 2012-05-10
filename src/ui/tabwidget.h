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
    void pullTab(int index, TabWidget *origin);
    QWidget* unregisterTab(int index);

    virtual QSize sizeHint() const
    {
        return QSize(100, 100);
    }

    virtual QSize minimumSizeHint() const
    {
        return QSize(0, 0);
    }

    void changeMenu(int idx);
    void clearMenu();

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void closeTab(int index);
    void tabMoved(int from, int to);
    void newTabBtn();
    void currentIndexChanged(int idx);

private:
    bool checkEvent(QMouseEvent *event);
    void checkEmpty();

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

public:
    explicit TabBar(QWidget * parent = 0);

    void enableSplit(bool enable);

protected:
    void mousePressEvent(QMouseEvent * event);

private slots:
    void renameTab();
    void splitTop();
    void splitLeft();

private:
    int m_cur_menu_tab;

    QMenu *m_menu;
    QAction *m_newTopBottom;
    QAction *m_newLeftRight;
};

#endif // MAINTABWIDGET_H
