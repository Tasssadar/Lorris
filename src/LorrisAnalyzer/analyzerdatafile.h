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

#ifndef ANALYZERDATAFILE_H
#define ANALYZERDATAFILE_H

#include <QFile>
#include <QBuffer>

enum DataBlocks
{
    BLOCK_STATIC_DATA,
    BLOCK_COLLAPSE_STATUS,
    BLOCK_COLLAPSE_STATUS2,
    BLOCK_DEVICE_TABS,
    BLOCK_DEVICE_TAB,
    BLOCK_CMD_TABS,
    BLOCK_CMD_TAB,
    BLOCK_DATA,
    BLOCK_WIDGETS,
    BLOCK_WIDGET
};

class AnalyzerDataFile : public QBuffer
{
    Q_OBJECT
public:
    explicit AnalyzerDataFile(const QString& filename, QObject *parent = 0);
    ~AnalyzerDataFile();

    bool seekToNextBlock(DataBlocks block, qint32 maxDist);
    bool seekToNextBlock(const char *block, qint32 maxDist);
    bool seekToNextBlock(DataBlocks block, DataBlocks toMax);
    bool seekToNextBlock(const char *block, const char* toMax);
    bool seekToNextBlock(const char *block, DataBlocks toMax);
    bool seekToNextBlock(DataBlocks block, const char* toMax);

    void writeBlockIdentifier(DataBlocks block);
    void writeBlockIdentifier(const char* block);
    
    char* getBlockName(DataBlocks block);

    bool open ( OpenMode mode );
    
private:
    char *getBlockWithFormat(const char *block, quint8& lenght);
    int m_last_block;
    QFile *m_file;
    QString m_filename;
};

#endif // ANALYZERDATAFILE_H
