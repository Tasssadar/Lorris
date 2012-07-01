/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOTUNNEL_H
#define SHUPITOTUNNEL_H

#include "connection.h"

class QComboBox;
class Shupito;

class ShupitoTunnel : public PortConnection
{
    Q_OBJECT

public:
    ShupitoTunnel();

    bool Open();
    void OpenConcurrent();
    void Close();
    void SendData(const QByteArray &data);

    void setShupito(Shupito* s);

protected:
    ~ShupitoTunnel();

private:
    Shupito *m_shupito;
    bool dataSigConnected;
};

#endif // SHUPITOTUNNEL_H
