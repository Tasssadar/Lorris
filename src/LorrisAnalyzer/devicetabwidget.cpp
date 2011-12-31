#include <QScrollArea>
#include <QList>
#include <QAction>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>

#include "devicetabwidget.h"
#include "labellayout.h"
#include "common.h"

DeviceTabWidget::DeviceTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    m_id_enabled = false;

    QAction *new_device_act = new QAction(tr("Add device"), this);
    connect(new_device_act, SIGNAL(triggered()), this, SLOT(newDevice()));
    addAction(new_device_act);

    m_add_all_act = new QAction(tr("Add \"All devices\" tab"), this);
    m_add_all_act->setEnabled(false);
    connect(m_add_all_act, SIGNAL(triggered()), this, SLOT(addAllDevices()));
    addAction(m_add_all_act);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClose(int)));
    m_all_devices = NULL;
}

DeviceTabWidget::~DeviceTabWidget()
{
    QList<QAction *> a = actions();
    for(QList<QAction *>::iterator itr = a.begin(); itr != a.end(); ++itr)
    {
        removeAction(*itr);
        delete *itr;
    }

    removeAll();
}

void DeviceTabWidget::removeAll()
{
    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
        delete itr->second;
    m_devices.clear();
    delete m_all_devices;
    clear();
}

void DeviceTabWidget::addDevice(bool all_devices, quint8 id)
{
    CmdTabWidget *cmd_tab = new CmdTabWidget(m_header, this);
    cmd_tab->addCommand();

    QString name;
    int index;
    if(!all_devices)
    {
        name = tr("Device") + " " + Utils::hexToString(id, true);
        m_devices.insert(std::make_pair<quint8,CmdTabWidget*>(id, cmd_tab));
        index = addTab(cmd_tab, name);
    }
    else
    {
        name = tr("All devices");
        m_all_devices = cmd_tab;
        index = insertTab(0, cmd_tab, name);
    }
    setCurrentIndex(index);
}

void DeviceTabWidget::handleData(analyzer_data *data)
{
    quint8 id = 0;
    if( m_all_devices)
        m_all_devices->handleData(data);

    if(!m_id_enabled || (m_id_enabled && !data->getDeviceId(id)))
        return;

    dev_map::iterator itr = m_devices.find(id);
    if(itr == m_devices.end())
        return;
    itr->second->handleData(data);
}

void DeviceTabWidget::newDevice()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New device"),
                                         tr("Device ID (hex or normal number):"), QLineEdit::Normal,
                                         "", &ok);
    int id = 0;
    quint8 res = 0;
    if(ok && !text.isEmpty())
    {
        if(text.contains("0x", Qt::CaseInsensitive))
        {
            text.replace("0x", "", Qt::CaseInsensitive);
            id = text.toInt(&ok, 16);
        }
        else
            id = text.toInt(&ok);
    }
    if(id > 255 || id < -128)
        ok = false;
    else
        res = (quint8)id;

    if(!ok)
    {
        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Wrong format, must be 8bit hex or normal number"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        return;
    }
    m_id_enabled = true;
    addDevice(false, res);
    setTabsClosable(true);
}

void DeviceTabWidget::tabClose(int index)
{
    QWidget *w = widget(index);
    removeTab(index);

    if(index == 0 && m_all_devices)
    {
        delete m_all_devices;
        m_all_devices = NULL;
        m_add_all_act->setEnabled(true);
        return;
    }

    for(dev_map::iterator itr = m_devices.begin(); itr != m_devices.end(); ++itr)
    {
        if(itr->second == w)
        {
            m_devices.erase(itr);
            break;
        }
    }
    delete w;

    if(count() < 2)
    {
        setTabsClosable(false);
        if(m_all_devices)
            m_id_enabled = false;
    }
}

void DeviceTabWidget::addAllDevices()
{
    m_add_all_act->setEnabled(false);
    addDevice();
    if(count() > 1)
        setTabsClosable(true);
}
