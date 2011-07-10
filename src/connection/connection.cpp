#include "connection.h"
#include "connectionmgr.h"

Connection::Connection()
{
    opened = false;
}

Connection::~Connection()
{
    sConMgr.RemoveCon(m_type, this);
}

bool Connection::Open()
{
    return false;
}

void Connection::SendData(QByteArray /*data*/)
{

}

void Connection::OpenConcurrent()
{

}
