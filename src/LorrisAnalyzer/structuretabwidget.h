#ifndef STRUCTURETABWIDGET_H
#define STRUCTURETABWIDGET_H

#include <QTabWidget>
#include <map>

#include "packet.h"
#include "labellayout.h"

class StructureTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    typedef std::map<quint8,ScrollDataLayout*> dev_map;

    explicit StructureTabWidget(QWidget *parent = 0);
    ~StructureTabWidget();

    void setHeader(analyzer_header *h) { m_header = h; }
    void addDevice(quint8 id = 0);
    void setEnableIds(bool enable) { m_id_enabled = enable; }
    void handleData(analyzer_data *data);

    void removeAll();

signals:

public slots:

private:
    analyzer_header *m_header;
    dev_map m_devices;
    bool m_id_enabled;
};

#endif // STRUCTURETABWIDGET_H
