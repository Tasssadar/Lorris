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

#define	ZPAD		0x2a		/* pad character; begins frames */
#define	ZDLE		0x18		/* ctrl-x zmodem escape */
#define	ZDLEE       0x58        /* escaped ZDLE */
#define	ZBIN		0x41		/* binary frame indicator (CRC16) */
#define	ZHEX		0x42		/* hex frame indicator */
#define	ZBIN32      0x43        /* binary frame indicator (CRC32) */

#define	ZRQINIT		0x00		/* request receive init (s->r) */
#define	ZRINIT		0x01		/* receive init (r->s) */
#define	ZSINIT		0x02		/* send init sequence (optional) (s->r) */
#define	ZACK		0x03		/* ack to ZRQINIT ZRINIT or ZSINIT (s<->r) */
#define	ZFILE		0x04		/* file name (s->r) */
#define	ZSKIP		0x05		/* skip this file (r->s) */
#define	ZNAK		0x06		/* last packet was corrupted (?) */
#define	ZABORT		0x07		/* abort batch transfers (?) */
#define	ZFIN		0x08		/* finish session (s<->r) */
#define	ZRPOS		0x09		/* resume data transmission here (r->s) */
#define	ZDATA		0x0a		/* data packet(s) follow (s->r) */
#define	ZEOF		0x0b		/* end of file reached (s->r) */
#define	ZFERR		0x0c		/* fatal read or write error detected (?) */
#define	ZCRC		0x0d		/* request for file CRC and response (?) */
#define	ZCHALLENGE	0x0e		/* security challenge (r->s) */
#define	ZCOMPL		0x0f		/* request is complete (?) */
#define	ZCAN		0x10		/* pseudo frame;
                                   other end cancelled session with 5* CAN */
#define	ZFREECNT	0x11		/* request free bytes on file system (s->r) */
#define	ZCOMMAND	0x12		/* issue command (s->r) */
#define	ZSTDERR	0x13 /* output data to stderr (??) */

#define	XON			0x11
#define	XOFF	0x13

#define HDRLEN 5            /* size of a zmodem header */
#define ZBLOCKLEN	1024	/* "true" Zmodem max subpacket length */

ZmodemProgrammer::ZmodemProgrammer(const ConnectionPointer<PortConnection> &conn, ProgrammerLogSink *logsink) :
    Programmer(logsink), m_conn(conn)
{
    m_conn = conn;
    m_flash_mode = false;
    //m_wait_act = WAIT_NONE;
    m_cancel_requested = false;
    m_bootseq = sConfig.get(CFG_STRING_ZMODEM_BOOTSEQ);

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

void ZmodemProgrammer::switchToFlashMode(quint32 /*prog_speed_hz*/)
{
    // FIXME
    if(!m_bootseq.isEmpty())
    {
        QByteArray bootseq = Utils::convertByteStr(m_bootseq);
        if(!bootseq.isEmpty())
            throw tr("Invalid bootloader sequence!");

        m_conn->SendData(bootseq);
    }

    m_flash_mode = true;
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
}

void ZmodemProgrammer::dataRead(const QByteArray& data)
{
}

QByteArray ZmodemProgrammer::getPaddedZDLE()
{
    static const char data[] = { ZPAD, ZPAD, ZDLE };
    return QByteArray::fromRawData(data, 3);
}

void ZmodemProgrammer::appendHex(QByteArray &dest, quint8 val)
{
    const char* xdigit="0123456789abcdef";
    dest.append(xdigit[val >> 4]);
    dest.append(xdigit[val & 0xF]);
}

static const quint16 crc16tbl[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

#define ucrc16(ch,crc) (crc16tbl[((crc>>8)&0xff)^(unsigned char)ch]^(crc << 8))

void ZmodemProgrammer::sendHexHeader(quint8 type, quint8 a1, quint8 a2, quint8 a3, quint8 a4)
{
    quint16 crc = 0;
    QByteArray data = getPaddedZDLE();
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

    data.append(quint8(crc >> 8));
    data.append(quint8(crc & 0xFF));

    data.append("\r\n");

    if(type!=ZACK && type!=ZFIN)
        data.append(XON);

    m_conn->SendData(data);
}


