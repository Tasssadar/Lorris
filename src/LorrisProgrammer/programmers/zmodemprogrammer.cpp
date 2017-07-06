/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QEventLoop>
#include <QTimer>
#include <QFileInfo>
#include <QDateTime>

#include "zmodemprogrammer.h"
#include "../../shared/defmgr.h"
#include "../../misc/config.h"
#include "../../misc/utils.h"
#include "../../shared/hexfile.h"

#include "zmodemprogrammer-defines.h"

enum recv_state {
    st_idle,
    st_zdle,
    st_hdr_type,
    st_hdr_bin16,
    st_hdr_bin32,
    st_hdr_hex,
};

ZmodemProgrammer::ZmodemProgrammer(const ConnectionPointer<PortConnection> &conn, ProgrammerLogSink *logsink) :
    Programmer(logsink), m_conn(conn)
{
    m_conn = conn;
    m_flash_mode = false;
    m_cancel_requested = false;
    m_bootseq = sConfig.get(CFG_STRING_ZMODEM_BOOTSEQ);
    m_recv_state = st_idle;
    m_escape_ctrl_chars = false;
    m_inside_zdle_seq = false;
    m_drop_newline = false;
    m_recv_32bit_data = false;
    m_wait_pkt = 0;
    m_send_bufsize = 4096;

    connect(m_conn.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(dataRead(QByteArray)));
}

QString ZmodemProgrammer::getBootseq() const
{
    return m_bootseq;
}

void ZmodemProgrammer::setBootseq(const QString& seq)
{
    m_bootseq = seq;
    sConfig.set(CFG_STRING_ZMODEM_BOOTSEQ, seq);
}

int ZmodemProgrammer::getType()
{
    return programmer_zmodem;
}

ProgrammerCapabilities ZmodemProgrammer::capabilities() const
{
    ProgrammerCapabilities ps;
    ps.flash = true;
    return ps;
}

void ZmodemProgrammer::stopAll(bool /*wait*/)
{
}

void ZmodemProgrammer::cancelRequested() {
    m_cancel_requested = true;
}

void ZmodemProgrammer::switchToFlashMode(quint32 /*prog_speed_hz*/)
{
    // FIXME
    if(!m_bootseq.isEmpty())
    {
        QByteArray bootseq = Utils::convertByteStr(m_bootseq);
        if(bootseq.isEmpty())
            throw tr("Invalid bootloader sequence!");
        m_conn->SendData(bootseq);
    }

    m_flash_mode = true;
    m_recv_buff.clear();
    m_hex_nibble_buff.clear();
    m_recv_state = st_idle;
}

void ZmodemProgrammer::switchToRunMode()
{
    // FIXME
    m_flash_mode = false;
}

bool ZmodemProgrammer::isInFlashMode()
{
    return m_flash_mode;
}

chip_definition ZmodemProgrammer::readDeviceId()
{
    return sDefMgr.findChipdef("zmodem:00");
}

QByteArray ZmodemProgrammer::readMemory(const QString&/*mem*/, chip_definition &/*chip*/)
{
    throw tr("Zmodem protocol cannot read program from the device.");
}

void ZmodemProgrammer::readFuses(std::vector<quint8>& data, chip_definition &chip)
{
    throw tr("Zmodem protocol cannot handle fuses.");
}

void ZmodemProgrammer::writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode verifyMode)
{
    throw tr("Zmodem protocol cannot handle fuses.");
}

void ZmodemProgrammer::erase_device(chip_definition& chip)
{
    throw tr("Zmodem protocol cannot erase the device.");
}

void ZmodemProgrammer::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode)
{
    sendHexHeader(ZRQINIT);
    if(!waitForPkt(ZRINIT, 2000))
        throw tr("Timeout while waiting for ZRINIT");

    const QByteArray data = file.getDataArray(0);

    sendBin32Header(ZFILE, 0, 0, ZF1_ZMCLOB, ZF0_ZCBIN);

    QString fname = QFileInfo(file.getFilePath()).fileName();
    if(fname.isEmpty()) {
        fname = "lorris" + QDateTime::currentDateTime().toString(Qt::ISODate);
    }

    QByteArray buf;
    buf.append(fname);
    buf.append('\0');
    buf.append(QString("%1 0 0644 0 1 %1").arg(data.size()));
    buf.append('\0');
    sendBin32Data(ZCRCW, buf);

    if(!waitForPkt(ZRPOS, 2000))
        qDebug() << tr("Timeout while waiting for ZRPOS.");

    m_cancel_requested = false;

    int sent = 0;
    while(sent < data.size()) {
        int chunk = (std::min)(m_send_bufsize, data.size() - sent);
        buf = data.mid(sent, chunk);

        sendBin32Header(ZDATA, sent, sent >> 8, sent >> 16, sent >> 24);

        sent += chunk;

        quint8 type = ZCRCW;
        if(sent >= data.size())
            type = ZCRCE;

        sendBin32Data(type, buf);

        if(type == ZCRCW && !waitForPkt(ZACK, 2000))
            throw tr("Timeout while waiting for ZACK.");

        if(m_cancel_requested)
        {
            qDebug() << "cancel";
            emit updateProgressDialog(-1);
            m_cancel_requested = false;
            break;
        }

        emit updateProgressDialog(double(sent)/data.size()*100);
    }

    sendHexHeader(ZEOF, sent, sent >> 8, sent >> 16, sent >> 24);
    if(!waitForPkt(ZRINIT, 2000))
        throw tr("Timeout while waiting for ZRINIT.");

    sendHexHeader(ZFIN);
    if(!waitForPkt(ZFIN, 2000))
        throw tr("Timeout while waiting for ZFIN.");
    m_conn->SendData(QByteArray("OO"));
}

bool ZmodemProgrammer::waitForPkt(int waitPkt, int timeout)
{
    if(m_wait_pkt != 0)
    {
        Q_ASSERT(false);
        return false;
    }

    m_wait_pkt = waitPkt;

    QEventLoop ev;
    QTimer t;

    connect(&t,   SIGNAL(timeout()), &ev, SLOT(quit()));
    connect(this, SIGNAL(waitPktDone()),  &ev, SLOT(quit()));

    t.setSingleShot(true);
    t.start(timeout);

    ev.exec();

    m_wait_pkt = 0;

    return t.isActive();
}

void ZmodemProgrammer::appendHex(QByteArray &dest, quint8 val)
{
    const char* xdigit="0123456789abcdef";
    dest.append(xdigit[val >> 4]);
    dest.append(xdigit[val & 0xF]);
}

void ZmodemProgrammer::appendEscapedByte(QByteArray &dest, quint8 c)
{
    switch(c) {
    case DLE:
    case DLE|0x80:
    case XON:
    case XON|0x80:
    case XOFF:
    case XOFF|0x80:
    case ZDLE:
        dest.append(ZDLE);
        dest.append(c ^ 0x40);
        return;
    case '\r':
    case '\r'|0x80:
        if(m_escape_ctrl_chars && dest.size() != 0 && dest[dest.size()-1] == '@') {
            dest.append(ZDLE);
            dest.append(c ^ 0x40);
            return;
        }
        break;
   /* case TELNET_IAC:
        if(m_escape_telnet_iac) {
            dest.append(ZDLE);
            dest.append(ZRUB1);
            return;
        }
        break;*/
    default:
        if(m_escape_ctrl_chars && (c & 0x60) == 0) {
            dest.append(ZDLE);
            dest.append(c ^ 0x40);
            return;
        }
        break;
    }
    dest.append(c);
}

void ZmodemProgrammer::sendHexHeader(quint8 type, quint8 a1, quint8 a2, quint8 a3, quint8 a4)
{
    quint16 crc = 0;
    QByteArray data;
    data.append(ZPAD);
    data.append(ZPAD);
    data.append(ZDLE);
    data.append(ZHEX);

    appendHex(data, type);
    crc = ucrc16(type, crc);
    appendHex(data, a1);
    crc = ucrc16(a1, crc);
    appendHex(data, a2);
    crc = ucrc16(a2, crc);
    appendHex(data, a3);
    crc = ucrc16(a3, crc);
    appendHex(data, a4);
    crc = ucrc16(a4, crc);

    appendHex(data, quint8(crc >> 8));
    appendHex(data, quint8(crc & 0xFF));
    data.append("\r\n");

    if(type!=ZACK && type!=ZFIN)
        data.append(XON);

    m_conn->SendData(data);
}

void ZmodemProgrammer::sendBin32Header(quint8 type, quint8 a1, quint8 a2, quint8 a3, quint8 a4)
{
    quint32 crc = 0xffffffffL;
    QByteArray data;
    data.append(ZPAD);
    data.append(ZDLE);
    data.append(ZBIN32);

    appendEscapedByte(data, type);
    crc = ucrc32(type, crc);
    appendEscapedByte(data, a1);
    crc = ucrc32(a1, crc);
    appendEscapedByte(data, a2);
    crc = ucrc32(a2, crc);
    appendEscapedByte(data, a3);
    crc = ucrc32(a3, crc);
    appendEscapedByte(data, a4);
    crc = ucrc32(a4, crc);

    crc = ~crc;

    appendEscapedByte(data, quint8(crc & 0xFF));
    appendEscapedByte(data, quint8(crc >> 8));
    appendEscapedByte(data, quint8(crc >> 16));
    appendEscapedByte(data, quint8(crc >> 24));

    m_conn->SendData(data);
}

void ZmodemProgrammer::sendBin32Data(quint8 type, const QByteArray& data)
{
    QByteArray buf;
    quint32 crc = 0xffffffffL;

    for(int i = 0; i < data.size(); ++i) {
        crc = ucrc32((quint8)data[i], crc);
        appendEscapedByte(buf, data[i]);
    }

    crc = ucrc32(type, crc);
    buf.append(ZDLE);
    buf.append(type);

    crc = ~crc;

    appendEscapedByte(buf, quint8(crc & 0xFF));
    appendEscapedByte(buf, quint8(crc >> 8));
    appendEscapedByte(buf, quint8(crc >> 16));
    appendEscapedByte(buf, quint8(crc >> 24));

    if(type == ZCRCW)
        buf.append(XON);

    m_conn->SendData(buf);
}


bool ZmodemProgrammer::rx_byte(int &c)
{
    if(!m_inside_zdle_seq) {
        switch(c) {
        case ZDLE:
            m_inside_zdle_seq = true;
            break;
        case XON:
        case XON|0x80:
        case XOFF:
        case XOFF|0x80:
            return false;
        default:
            if(m_escape_ctrl_chars && (c >= 0) && (c & 0x60) == 0)
                return false;
            return true;
        }
    } else {
        switch(c) {
        case XON:
        case XON|0x80:
        case XOFF:
        case XOFF|0x80:
        case ZDLE:
            return false;
        case ZCRCE:
        case ZCRCG:
        case ZCRCQ:
        case ZCRCW:
            m_inside_zdle_seq = false;
            c = (c | ZDLEESC);
            return true;
        case ZRUB0:
            m_inside_zdle_seq = false;
            c = 0x7f;
            return true;
        case ZRUB1:
            m_inside_zdle_seq = false;
            c = 0xFF;
            return true;
        default:
            if(c < 0) {
                m_inside_zdle_seq = false;
                return true;
            }

            if(m_escape_ctrl_chars && (c & 0x60) == 0) {
                return false;
            }

            if((c & 0x60) == 0x40) {
                m_inside_zdle_seq = false;
                c = c ^ 0x40;
                return true;
            }
            return false;
        }
    }
    Q_UNREACHABLE();
}

bool ZmodemProgrammer::rx_hex_byte(int &c) {
    if(!rx_byte(c))
        return false;

    if(c < 0)
        return true;

    if(m_hex_nibble_buff.size() > 1)
        m_hex_nibble_buff.clear();

    if(c > '9') {
        if(c < 'a' || c > 'f') {
            qDebug() << "zmodem: illegal hex character " << c;
            return true;
        }
        c -= 'a' - 10;
    } else {
        if(c < '0') {
            qDebug() << "zmodem: illegal hex character " << c;
            return true;
        }
        c -= '0';
    }

    m_hex_nibble_buff.append(c);
    if(m_hex_nibble_buff.size() == 1) {
        return false;
    } else {
        c = (m_hex_nibble_buff[0] << 4) | m_hex_nibble_buff[1];
        m_hex_nibble_buff.clear();
        return true;
    }
}

void ZmodemProgrammer::dataRead(const QByteArray& data)
{
    const int sz = data.size();
    for(int i = 0; i < sz; ++i) {
        int c = quint8(data[i]);

        if(m_drop_newline) {
            m_drop_newline = false;
            continue;
        }

        switch(m_recv_state) {
        case st_idle:
            if(c != ZPAD)
                continue;
            m_inside_zdle_seq = false;
            m_recv_state = st_zdle;
            break;
        case st_zdle:
            if(c == ZPAD)
                continue;

            if(c != ZDLE) {
                m_recv_state = st_idle;
                qDebug() << "zmodem: expected  ZDLE, received " << c;
            } else {
                m_recv_state = st_hdr_type;
            }
            break;
        case st_hdr_type:
            if(!rx_byte(c))
                continue;

            m_recv_buff.clear();

            switch(c) {
            case ZBIN:
                m_recv_state = st_hdr_bin16;
                m_recv_32bit_data = false;
                break;
            case ZBIN32:
                m_recv_state = st_hdr_bin32;
                m_recv_32bit_data = true;
                break;
            case ZHEX:
                m_recv_state = st_hdr_hex;
                m_recv_32bit_data = false;
                break;
            default:
                qDebug() << "zmodem: unrecognized header style " << c;
                m_recv_state = st_idle;
                break;
            }
            break;
        case st_hdr_bin16:
        case st_hdr_bin32: {
            if(!rx_byte(c))
                continue;

            m_recv_buff.append((char)c);
            const int crcsize = m_recv_state == st_hdr_bin16 ? 2 : 4;
            if(m_recv_buff.size() != HDRLEN + crcsize)
                continue;

            if(m_recv_state == st_hdr_bin16) {
                quint16 crc = 0;
                for(int i = 0; i < HDRLEN; ++i)
                    crc = ucrc16(m_recv_buff[i], crc);

                quint16 expected_crc = (quint8(m_recv_buff[HDRLEN]) << 8) | quint8(m_recv_buff[HDRLEN+1]);
                if(crc != expected_crc) {
                    qDebug() << "zmodem: invalid crc16 in hdr_bin16";
                }
            } else {
                quint32 crc = 0xffffffffL;
                for(int i = 0; i < HDRLEN; ++i)
                    crc = ucrc32(quint8(m_recv_buff[i]), crc);

                quint32 expected_crc = quint8(m_recv_buff[HDRLEN]) |
                        (quint8(m_recv_buff[HDRLEN+1]) << 8) |
                        (quint8(m_recv_buff[HDRLEN+2]) << 16) |
                        (quint8(m_recv_buff[HDRLEN+3]) << 24);
                if(crc != expected_crc) {
                    qDebug() << "zmodem: invalid crc32 in hdr_bin32";
                }
            }

            m_recv_buff.resize(HDRLEN);
            m_recv_state = st_idle;
            processHeader(m_recv_buff);
            break;
        }
        case st_hdr_hex: {
            if(m_recv_buff.size() == HDRLEN + 2) {
                if(!rx_byte(c))
                    continue;
                m_recv_buff.append((char)c);
            } else {
                if(rx_hex_byte(c))
                    m_recv_buff.append((char)c);
                continue;
            }

            quint16 crc = 0;
            for(int i = 0; i < HDRLEN; ++i)
                crc = ucrc16(m_recv_buff[i], crc);

            quint16 expected_crc = (quint8(m_recv_buff[HDRLEN]) << 8) | quint8(m_recv_buff[HDRLEN+1]);
            if(crc != expected_crc) {
                qDebug() << "zmodem: invalid crc16 in hdr_hex";
            }

            m_drop_newline = m_recv_buff[HDRLEN + 2] == '\r';

            m_recv_buff.resize(HDRLEN);
            m_recv_state = st_idle;
            processHeader(m_recv_buff);
            break;
        }
        }
    }
}

void ZmodemProgrammer::processHeader(const QByteArray& hdr) {
    Q_ASSERT(hdr.size() == HDRLEN);

    int frame_type = hdr[0];
    switch(frame_type) {
    case ZRINIT:
        // Ignoring most flags
        m_escape_ctrl_chars = (hdr[ZF0] & ZF0_ESCCTL) != 0;

        m_send_bufsize = hdr[ZP0] | (hdr[ZP1] << 8);
        if(m_send_bufsize <= 0 || m_send_bufsize > 32*1024*1024)
            m_send_bufsize = 4096;
        break;
    case ZRPOS:
    case ZACK:
    case ZFIN:
        break;
    default:
        qDebug() << "zmodem: got unhandled header " << frame_type;
    }

    if(m_wait_pkt == frame_type) {
        m_wait_hdr = hdr;
        emit waitPktDone();
    }
}
