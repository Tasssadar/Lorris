#ifndef STRUCTURETABWIDGET_H
#define STRUCTURETABWIDGET_H

#include <QTabWidget>
#include <map>

#include "packet.h"
#include "labellayout.h"
#include "cmdtabwidget.h"

class QAction;

class DeviceTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    typedef std::map<quint8,CmdTabWidget*> dev_map;

    explicit DeviceTabWidget(QWidget *parent = 0);
    ~DeviceTabWidget();

    void setHeader(analyzer_header *h);
    void addDevice(bool all_devices = true, quint8 id = 0);
    void setEnableIds(bool enable) { m_id_enabled = enable; }
    void handleData(analyzer_data *data);

    void removeAll();

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
