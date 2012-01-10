/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef CMDTABWIDGET_H
#define CMDTABWIDGET_H

#include <QTabWidget>
#include <map>
#include <QScrollArea>

#include "labellayout.h"
#include "packet.h"

class AnalyzerDataFile;
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
    void Save(AnalyzerDataFile *file);
    void Load(AnalyzerDataFile *file, bool skip);

    bool setHighlightPos(const data_widget_info& info, bool highlight);

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
