#ifndef CMDTABWIDGET_H
#define CMDTABWIDGET_H

#include <QTabWidget>
#include <map>
#include <QScrollArea>

#include "labellayout.h"
#include "packet.h"

class QFile;
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

public:
    typedef std::map<quint8,CmdTabInfo*> cmd_map;

    explicit CmdTabWidget(analyzer_header *header, DeviceTabWidget *device, QWidget *parent = 0);
    ~CmdTabWidget();

    void removeAll();
    void addCommand(bool add_all_cmds = true, quint8 id = 0);
    void handleData(analyzer_data *data);
    void setEnablePackets(bool enable)
    {
        m_enableCmds = enable;
    }
    qint16 getCurrentCmd();
    void Save(QFile *file);
    void Load(QFile *file, bool skip);

    QPoint getBytePos(const data_widget_info& info);

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
    DeviceTabWidget *m_devTab;
};

#endif // CMDTABWIDGET_H
