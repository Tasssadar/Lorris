/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef WORKTAB_H
#define WORKTAB_H

#include <QtGui/QWidget>
#include <vector>
#include <QMenu>

#include "../common.h"
#include "../connection/connection.h"
#include "WorkTabInfo.h"
#include "../misc/datafileparser.h"

class WorkTab : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void statusBarMsg(const QString& message, int timeout = 0);
    void setConnId(const QString& str, bool hadConn);

public:
    virtual ~WorkTab();

    void setId(quint32 id) { m_id = id; }
    quint32 getId() { return m_id; }

    static void DeleteAllMembers(QLayout *layout);

    virtual void onTabShow(const QString& filename);
    virtual bool onTabClose();
    virtual void openFile(const QString& filename);
    virtual std::vector<QMenu*>& getMenu() { return m_menus; }

    WorkTabInfo *getInfo() const { return m_info; }
    void setInfo(WorkTabInfo *info) { m_info = info; }

    virtual void saveData(DataFileParser *file);
    virtual void loadData(DataFileParser *file);
    virtual QString GetIdString() = 0;

protected:
    WorkTab();

    void addTopMenu(QMenu *menu);

    quint32 m_id;

private:
    std::vector<QMenu*> m_menus;
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
    virtual void setConnection(PortConnection *con);

protected:
    PortConnection *m_con;

protected slots:
    virtual void readData(const QByteArray &data);
    virtual void connectedStatus(bool connected);
    virtual void connectionDestroyed();
};

#endif // WORKTAB_H
