/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef WORKTAB_H
#define WORKTAB_H

#include <QWidget>
#include <vector>
#include <QMenu>

#ifdef Q_OS_MAC
#include <QtMacExtras>
#endif

#include "../common.h"
#include "../connection/connection.h"
#include "WorkTabInfo.h"
#include "../misc/datafileparser.h"
#include "tab.h"

class WorkTab : public Tab
{
    Q_OBJECT

Q_SIGNALS:
    void statusBarMsg(const QString& message, int timeout = 0);
    void setConnId(const QString& str, bool hadConn);
    void setWindowProgress(int progress);

public:
    virtual ~WorkTab();

    void setId(quint32 id) { m_id = id; }
    quint32 getId() { return m_id; }

    virtual void onTabShow(const QString& filename);
    virtual void openFile(const QString& filename);
    virtual std::vector<QAction*>& getActions() { return m_actions; }

#ifdef Q_OS_MAC
    virtual QList<QMacToolBarItem*>& getMacBarItems() { return m_macBarItems; }
#endif

    WorkTabInfo *getInfo() const { return m_info; }
    void setInfo(WorkTabInfo *info) { m_info = info; }

    virtual void saveData(DataFileParser *file);
    virtual void loadData(DataFileParser *file);
    virtual QString GetIdString() = 0;

    virtual void childClosed(QWidget *child);

protected:
    WorkTab();

    void addTopMenu(QMenu *menu);
    void addTopAction(QAction *act);

#ifdef Q_OS_MAC
    QList<QMacToolBarItem*> m_macBarItems;
    QMacToolBarItem *addItemMacToolBar(const QIcon &icon, const QString &text);
#endif

    quint32 m_id;

private:
    std::vector<QAction*> m_actions;
    WorkTabInfo *m_info;
};

class PortConnWorkTab : public WorkTab
{
    Q_OBJECT

public:
    PortConnWorkTab();
    ~PortConnWorkTab();

    virtual void saveData(DataFileParser *file);
    virtual void loadData(DataFileParser *file);

public slots:
    virtual void setConnection(ConnectionPointer<Connection> const & con);
    virtual void setPortConnection(ConnectionPointer<PortConnection> const & con);

protected:
    ConnectionPointer<PortConnection> m_con;

protected slots:
    virtual void readData(const QByteArray &data);
    virtual void connectedStatus(bool connected);
};

#endif // WORKTAB_H
