#ifndef FILECONNECTION_H
#define FILECONNECTION_H

#include "connection.h"

class FileConnection : public Connection
{
public:
    FileConnection();
    ~FileConnection();

    bool Open();
    void OpenConcurrent();
};

#endif // FILECONNECTION_H
