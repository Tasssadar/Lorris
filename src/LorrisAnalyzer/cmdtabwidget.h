/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CMDTABWIDGET_H
#define CMDTABWIDGET_H

#include <QTabWidget>
#include <map>
#include <QScrollArea>

#include "labellayout.h"
#include "packet.h"

class DataFileParser;
struct data_widget_info;

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

Q_SIGNALS:
    void updateData();
    void disableCmdAdd(bool disable);

public:
    typedef std::map<quint8,CmdTabInfo*> cmd_map;

    explicit CmdTabWidget(analyzer_header *header, DeviceTabWidget *device, QWidget *parent = 0);
    ~CmdTabWidget();

    void removeAll();
    void addCommand(bool add_all_cmds = true, quint8 id = 0);
    void handleData(analyzer_data *data, quint32 index);
    void setEnablePackets(bool enable)
    {
        m_enableCmds = enable;
    }
    qint16 getCurrentCmd();
    void Save(DataFileParser *file);
    void Load(DataFileParser *file, bool skip);

    bool setHighlightPos(const data_widget_info& info, bool highlight);

    void setHeader(analyzer_header *header);

private slots:
    void newCommand();
    void addAllCmds();
    void tabClose(int index);

private:
    cmd_map m_cmds;
    bool m_enableCmds;
    analyzer_header *m_header;
    QAction *m_add_all_act;
    QAction *new_cmd_act;
    CmdTabInfo *m_all_cmds;
    DeviceTabWidget *m_devTab;
};

#endif // CMDTABWIDGET_H
