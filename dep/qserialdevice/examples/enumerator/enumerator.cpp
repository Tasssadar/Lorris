#include <QtCore/QDebug>
#include <QtCore/QStringList>

#include "enumerator.h"
#include "serialdeviceenumerator.h"

Enumerator::Enumerator(QObject *parent)
        : QObject(parent)
{
    this->m_sde = SerialDeviceEnumerator::instance();
    connect(this->m_sde, SIGNAL(hasChanged(QStringList)),
            this, SLOT(slotPrintAllDevices(QStringList)));
    this->slotPrintAllDevices(this->m_sde->devicesAvailable());
}

void Enumerator::slotPrintAllDevices(const QStringList &list)
{
    qDebug() << "\n ===> All devices: " << list;

    foreach (QString s, list) {
        this->m_sde->setDeviceName(s);
        qDebug() << "\n <<< info about: " << this->m_sde->name() << " >>>";
        qDebug() << "-> description  : " << this->m_sde->description();
        qDebug() << "-> driver       : " << this->m_sde->driver();
        qDebug() << "-> friendlyName : " << this->m_sde->friendlyName();
        qDebug() << "-> hardwareID   : " << this->m_sde->hardwareID();
        qDebug() << "-> locationInfo : " << this->m_sde->locationInfo();
        qDebug() << "-> manufacturer : " << this->m_sde->manufacturer();
        qDebug() << "-> productID    : " << this->m_sde->productID();
        qDebug() << "-> service      : " << this->m_sde->service();
        qDebug() << "-> shortName    : " << this->m_sde->shortName();
        qDebug() << "-> subSystem    : " << this->m_sde->subSystem();
        qDebug() << "-> systemPath   : " << this->m_sde->systemPath();
        qDebug() << "-> vendorID     : " << this->m_sde->vendorID();

        qDebug() << "-> revision     : " << this->m_sde->revision();
        qDebug() << "-> bus          : " << this->m_sde->bus();
        //
        qDebug() << "-> is exists    : " << this->m_sde->isExists();
        qDebug() << "-> is busy      : " << this->m_sde->isBusy();
    }
}
