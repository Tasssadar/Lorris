/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "zmodemprogrammer.h"
#include "../../shared/defmgr.h"
#include "../../misc/config.h"
#include "../../misc/utils.h"
#include "../../shared/hexfile.h"

ZmodemProgrammer::ZmodemProgrammer(const ConnectionPointer<PortConnection> &conn, ProgrammerLogSink *logsink) :
    Programmer(logsink), m_conn(conn)
{
    m_conn = conn;
    m_flash_mode = false;
    m_wait_act = WAIT_NONE;
    m_cancel_requested = false;
    m_bootseq = sConfig.get(CFG_STRING_ZMODEM_BOOTSEQ);

    connect(m_conn.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(dataRead(QByteArray)));
}
