#include "fileconnection.h"
#include "connectionmgr.h"

FileConnection::FileConnection()
{
    m_type = CONNECTION_FILE;
}

FileConnection::~FileConnection()
{

}

bool FileConnection::Open()
{
    return false;
}

void FileConnection::OpenConcurrent()
{
    emit connectResult(this, false);
}
