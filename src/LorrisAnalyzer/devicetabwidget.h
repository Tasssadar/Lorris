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

#ifndef STRUCTURETABWIDGET_H
#define STRUCTURETABWIDGET_H

#include <QTabWidget>
#include <map>

#include "packet.h"
#include "labellayout.h"
#include "cmdtabwidget.h"

class QAction;
class AnalyzerDataFile;

struct data_widget_info;

class DeviceTabWidget : public QTabWidget
{
    Q_OBJECT

Q_SIGNALS:
    void updateData();

public:
    typedef std::map<quint8,CmdTabWidget*> dev_map;

    explicit DeviceTabWidget(QWidget *parent = 0);
    ~DeviceTabWidget();

    void setHeader(analyzer_header *h);
    CmdTabWidget *addDevice(bool all_devices = true, quint8 id = 0);
    void setEnableIds(bool enable) { m_id_enabled = enable; }
    qint16 getCurrentDevice();
    void removeAll();
    void Save(AnalyzerDataFile *file);
    void Load(AnalyzerDataFile *file, bool skip);

    bool setHighlightPos(const data_widget_info& info, bool highlight);

public slots:
    void handleData(analyzer_data *data);

private slots:
    void newDevice();
    void addAllDevices();
    void tabClose(int index);

private:
    analyzer_header *m_header;
    dev_map m_devices;
    CmdTabWidget* m_all_devices;
    QAction *m_add_all_act;
    bool m_id_enabled;
};

#endif // STRUCTURETABWIDGET_H
