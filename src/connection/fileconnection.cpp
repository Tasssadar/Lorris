/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "fileconnection.h"
#include "connectionmgr.h"
#include "WorkTab/WorkTabInfo.h"

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

void FileConnectionBuilder::CreateConnection(WorkTabInfo *info)
{
    FileConnection *con = (FileConnection*)sConMgr.FindConnection(CONNECTION_FILE, "");
    if(!con)
    {
        con = new FileConnection();
        sConMgr.AddCon(CONNECTION_FILE, con);
    }

    emit connectionSucces(con, info->GetName(), info);
}
