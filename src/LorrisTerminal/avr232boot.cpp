/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QEventLoop>
#include <QTimer>

#include "avr232boot.h"
#include "../misc/utils.h"
#include "../connection/connection.h"
#include "../shared/defmgr.h"

#include "ui_lorristerminal.h"

avr232boot::avr232boot(QObject *parent) :
    QObject(parent)
{
    m_state = 0;
    m_con = NULL;
}

bool avr232boot::dataRead(const QByteArray &data)
{
    if((m_state & STATE_WAIT_ACK) && data.indexOf(char(20)) != -1)
    {
        emit received();
        return true;
    }
    else if(m_state & STATE_WAITING_ID)
    {
        m_dev_id.append(data);
        if(m_dev_id.size() >= 4)
        {
            m_dev_id.resize(4);
            emit received();
        }
        return true;
    }
    else if(m_state & STATE_EEPROM_READ)
    {
        m_eeprom.AddData(data);
        if(m_eeprom.size()%128 == 0)
            emit received();
        return true;
    }

    return false;
}

bool avr232boot::waitForRec()
{
    QEventLoop ev(this);
    QTimer t(this);

    connect(&t,   SIGNAL(timeout()), &ev, SLOT(quit()));
    connect(this, SIGNAL(received()),  &ev, SLOT(quit()));

    t.setSingleShot(true);
    t.start(1000);

    ev.exec();

    return t.isActive();
}

bool avr232boot::waitForAck()
{
    m_state |= STATE_WAIT_ACK;
    bool res = waitForRec();
    m_state &= ~(STATE_WAIT_ACK);
    return res;
}

bool avr232boot::startStop()
{
    if(!m_con || !m_con->isOpen())
        return false;

    if(m_state & STATE_STOPPED)
    {
        m_con->SendData(QByteArray(1, 0x11));
    }
    else
    {
        if(!stopSequence())
            return false;
    }
    m_state ^= STATE_STOPPED;
    return true;
}

bool avr232boot::stopSequence()
{
    static const char stopCmd[4] = { 0x74, 0x7E, 0x7A, 0x33 };

    m_con->SendData(QByteArray::fromRawData(stopCmd, 4));
    // FIXME: first sequence restarts chip to bootloader,
    // but I won't get the ack byte. Correct?
    //if(!waitForAck())
    //    return false;
    QApplication::processEvents(QEventLoop::WaitForMoreEvents, 100);

    m_con->SendData(QByteArray::fromRawData(stopCmd, 4));
    if(!waitForAck())
        return false;
    return true;
}

bool avr232boot::getChipId()
{
    m_dev_id.clear();

    m_state |= STATE_WAITING_ID;
    m_con->SendData(QByteArray(1, 0x12));

    waitForRec();

    m_state &= ~(STATE_WAITING_ID);

    if(m_dev_id.size() != 4)
    {
        Utils::ThrowException(tr("Can't read chip ID!"));
        return false;
    }

    return true;
}

bool avr232boot::flash(Ui::LorrisTerminal *ui)
{
    QString deviceId(m_dev_id);

    chip_definition cd = sDefMgr.findChipdef("avr232boot:" + deviceId);
    if(cd.getName().isEmpty())
    {
        Utils::ThrowException(tr("Unsupported chip: ") + deviceId);
        ui->flashText->setText(tr("Chip: %1").arg(tr("<unknown>")));
        return false;
    }

    ui->flashText->setText(tr("Chip: %1").arg(cd.getName()));

    std::vector<page> pages;
    try {
        m_hex.makePages(pages, MEM_FLASH, cd, NULL);
    } catch(QString ex) {
        Utils::ThrowException(tr("Error making pages: ") + ex);
        return false;
    }

    ui->progressBar->show();
    ui->progressBar->setMaximum(pages.size());

    quint32 cur_page = 0;
    do
    {
        if(cur_page >= pages.size())
            goto exit;

        page& p = pages[cur_page++];
        m_con->SendData(QByteArray(1, 0x10));

        QByteArray data;
        data[0] = quint8(p.address >> 8);
        data[1] = quint8(p.address);
        m_con->SendData(data);

        for(quint16 i = 0; i < p.data.size(); ++i)
            data[i] = p.data[i];
        m_con->SendData(data);

        ui->progressBar->setValue(cur_page);
    } while(waitForAck());

    Utils::ThrowException(tr("Timeout during flashing!"));

exit:
    ui->progressBar->setValue(0);
    ui->progressBar->hide();

    return cur_page >= pages.size();
}

void avr232boot::readEEPROM(Ui::LorrisTerminal *ui)
{
    QString id(m_dev_id);

    chip_definition cd = sDefMgr.findChipdef("avr232boot:" + id);
    if(cd.getName().isEmpty() || !cd.getMemDef(MEM_EEPROM))
    {
        Utils::ThrowException(tr("Unsupported chip: ") + id);
        return;
    }

    m_eeprom.reset(cd);
    ui->progressBar->show();
    ui->progressBar->setMaximum(m_eeprom.GetEEPROMSize());

    m_state |= STATE_EEPROM_READ;

    quint16 itr = 0;
    do
    {
        if(m_eeprom.size() >= m_eeprom.GetEEPROMSize())
        {
            m_eeprom.Export();
            goto exit;
        }
        QByteArray data(1, 0x13);
        m_con->SendData(data);

        data[0] = (itr >> 8);
        data[1] = quint8(itr);
        data[2] = 128;
        m_con->SendData(data);

        itr += 128;
        ui->progressBar->setValue(itr);
    }while(waitForRec());

    Utils::ThrowException(tr("Timeout during EEPROM read."));

exit:
    ui->progressBar->setValue(0);
    ui->progressBar->hide();

    m_state &= ~(STATE_EEPROM_READ);
}

void avr232boot::writeEEPROM(Ui::LorrisTerminal *ui)
{
    QString id(m_dev_id);

    chip_definition cd = sDefMgr.findChipdef("avr232boot:" + id);
    if(cd.getName().isEmpty() || !cd.getMemDef(MEM_EEPROM))
    {
        Utils::ThrowException(tr("Unsupported chip: ") + id);
        return;
    }
    m_eeprom.reset(cd);
    if(!m_eeprom.Import())
        return;

    ui->progressBar->show();
    ui->progressBar->setMaximum(m_eeprom.GetEEPROMSize());

    do
    {
        page *p = m_eeprom.getNextPage();

        if(!p)
            goto exit;

        QByteArray data(1, 0x14);
        m_con->SendData(data);

        data[0] = (p->address >> 8);
        data[1] = quint8(p->address);
        data[2] = 2;
        m_con->SendData(data);

        data.clear();
        for(quint8 i = 0; i < p->data.size(); ++i)
            data[i] = p->data[i];
        m_con->SendData(data);

        ui->progressBar->setValue(p->address);
    }while(waitForAck());

    Utils::ThrowException(tr("Timout during EEPROM write!"));

exit:
    ui->progressBar->setValue(0);
    ui->progressBar->hide();
}

void avr232boot::setStopStatus(bool stopped)
{
    if(stopped)
        m_state |= STATE_STOPPED;
    else
        m_state &= ~(STATE_STOPPED);
}
