#ifndef CMDTABWIDGET_H
#define CMDTABWIDGET_H

#include <QTabWidget>
#include <map>
#include <QScrollArea>

#include "labellayout.h"
#include "packet.h"


struct CmdTabInfo
{
    CmdTabInfo(QScrollArea *area, ScrollDataLayout *layout)
    {
        a = area;
        l = layout;
    }
    ~CmdTabInfo()
    {
        delete l;
        delete a;
    }

    QScrollArea *a;
    ScrollDataLayout *l;
};

class CmdTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    typedef std::map<quint8,CmdTabInfo*> cmd_map;

    explicit CmdTabWidget(analyzer_header *header, QWidget *parent = 0);
    ~CmdTabWidget();

    void removeAll();
    void addCommand(bool add_all_cmds = true, quint8 id = 0);
    void handleData(analyzer_data *data);
    void setEnablePackets(bool enable)
    {
        m_enableCmds = enable;
    }

private slots:
    void newCommand();
    void addAllCmds();
    void tabClose(int index);

private:
    cmd_map m_cmds;
    bool m_enableCmds;
    analyzer_header *m_header;
    QAction *m_add_all_act;
    CmdTabInfo *m_all_cmds;
};

#endif // CMDTABWIDGET_H
