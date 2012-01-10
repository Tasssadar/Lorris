#ifndef ANALYZERDATAFILE_H
#define ANALYZERDATAFILE_H

#include <QFile>

enum DataBlocks
{
    BLOCK_STATIC_DATA,
    BLOCK_DEVICE_TABS,
    BLOCK_DEVICE_TAB,
    BLOCK_CMD_TABS,
    BLOCK_CMD_TAB,
    BLOCK_DATA,
    BLOCK_WIDGETS,
    BLOCK_WIDGET
};

class AnalyzerDataFile : public QFile
{
    Q_OBJECT
public:
    explicit AnalyzerDataFile(const QString& filename, QObject *parent = 0);

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
    QByteArray m_data;
    int m_last_block;
};

#endif // ANALYZERDATAFILE_H
