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
#include <QFrame>

#include "plustabbar.h"

class QPushButton;
class QHBoxLayout;
class QMenu;
class TabBar;
class WorkTab;
class DataFileParser;
class Tab;
class ChildTab;
class TabView;
class TabSwitchWidget;

class TabWidget : public QTabWidget
{
    Q_OBJECT

Q_SIGNALS:
    void newTab();
    void removeWidget(quint32 id);
    void split(bool horizontal, int index);
    void changeActiveWidget(TabWidget *widget);
    void statusBarMsg(const QString& message, int timeout = 0);
    void closeHomeTab();

public:
    explicit TabWidget(quint32 id, QWidget *parent = 0);

    quint32 getId() const { return m_id; }

    int addTab(QWidget *widget, const QString& name)
    {
        return QTabWidget::addTab(widget, name);
    }

    int addTab(Tab *widget, const QString& name, quint32 tabId);
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


    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    void addChildTab(ChildTab *widget, const QString &name);
    void removeChildTab(ChildTab *widget);

    const QList<quint32>& getHistory() const { return m_tabHistory; }
    const std::vector<quint32>& getTabIds() const { return m_tab_ids; }

public slots:
    int pullTab(int index, TabWidget *origin);
    void pullTab(int index, TabWidget *origin, int to);
    void closeTab(int index);
    void checkEmpty();

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void tabMoved(int from, int to);
    void newTabBtn();
    void currentIndexChanged(int idx);
    void setConnString(const QString& str, bool hadConn);
    void activateTab();
    void setTabNameAndTooltip(QString name);
    void newWindow(int idx);
    void tabRemoved(int index);

private:
    inline TabView* tabView() const { return (TabView*)parent(); }

    bool checkEvent(QMouseEvent *event);
    void setTabNameAndTooltip(int idx, QString name);

    quint32 m_id;
    std::vector<quint32> m_tab_ids;
    TabBar *m_tab_bar;
    QList<quint32> m_tabHistory;

    QPushButton *m_menuBtn;
    QMenu *m_menu;

    TabSwitchWidget *m_switchWidget;
};

class TabBar : public PlusTabBar
{
    Q_OBJECT

Q_SIGNALS:
    void split(bool horizontal, int index);
    void pullTab(int idx, TabWidget *origin, int to);
    void newWindow(int idx);

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
    void toNewWindow();

private:
    void updateDropMarker(const QPoint& pos);
    inline TabView* tabView() const { return (TabView*)(parent()->parent()); }

    int m_cur_menu_tab;
    QRect m_drag_insert;
    int m_drag_idx;

    quint32 m_id;
    QMenu *m_menu;
    QAction *m_newTopBottom;
    QAction *m_newLeftRight;
    QPoint m_startDragPos;
};

namespace Ui {
    class TabSwitchWidget;
}

class TabSwitchWidget : public QFrame
{
    Q_OBJECT
Q_SIGNALS:
    void setIndex(int idx);

public:
    TabSwitchWidget(QWidget *parent);
    ~TabSwitchWidget();

    void next();

private:
    TabWidget *tabWidget() const { return (TabWidget*)parent(); }
    void createButton(int idx);
    void generatePreview(int idx);

    Ui::TabSwitchWidget *ui;
    QHash<quint32, int> m_id_pair;
    std::vector<QPushButton*> m_buttons;
    int m_active;
};

#endif // MAINTABWIDGET_H
